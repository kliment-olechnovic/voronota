#!/bin/bash

OUTMODE="$1"
INFILE="$2"
shift
shift
FIRSTSCORE="$1"

if [ "$OUTMODE" != "full" ] && [ "$OUTMODE" != "short" ]
then
	echo >&2 "Error: output mode is neither 'full' nor 'short'"
	exit 1
fi

if [ -z "$INFILE" ] && [ ! -t 0 ]
then
	INFILE="-"
fi

if [ -z "$INFILE" ]
then
	echo >&2 "Error: no input"
	exit 1
fi

if [ -z "$FIRSTSCORE" ]
then
	echo >&2 "Error: no score names specified"
	exit 1
fi

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

{
	if [ "$INFILE" == "-" ]
	then
		cat
	else
		cat "$INFILE"
	fi
} \
> "$TMPLDIR/infile"

if [ ! -s "$TMPLDIR/infile" ]
then
	echo >&2 "Error: no input data"
	exit 1
fi

R --vanilla --args "$TMPLDIR/outfile" "$TMPLDIR/infile" "$@" \
> /dev/null \
<< 'EOF'

args=commandArgs(TRUE);

outfile=args[1];
infile=args[2];
scorenames=args[3:length(args)];

t=read.table(infile, header=TRUE, stringsAsFactors=FALSE);

M=length(scorenames);
N=nrow(t);

indices=rep(1, M);
for(m in 1:M)
{
	index=which(colnames(t)==scorenames[m]);
	if(length(index)!=1)
	{
		stop(paste("Invalid score name '", scorenames[m], "'.", sep=""));
	}
	indices[m]=index;
}

signs=rep(0, M);
duel_wins=rep(0, N);
duel_draws=rep(0, N);

i=1;
while(i<=N)
{
	j=i+1;
	while(j<=N)
	{
		for(m in 1:M)
		{
			signs[m]=sign(t[i, indices[m]]-t[j, indices[m]]);
		}
		
		if(length(which(signs>0))==M)
		{
			duel_wins[i]=duel_wins[i]+1;
		} else if(length(which(signs<0))==M)
		{
			duel_wins[j]=duel_wins[j]+1;
		} else
		{
			duel_draws[i]=duel_draws[i]+1;
			duel_draws[j]=duel_draws[j]+1;
		}
		
		j=j+1;
	}
	i=i+1;
}


tour_score=(duel_wins*N+duel_draws);

t$tour_wins=duel_wins;
t$tour_draws=duel_draws;
t$tour_rank=rank(0-tour_score)

t=t[order(t$tour_rank),];

write.table(t, outfile, quote=FALSE, row.names=FALSE);

EOF

if [ ! -s "$TMPLDIR/outfile" ]
then
	echo >&2 "Error: no output"
	exit 1
fi

cat "$TMPLDIR/outfile" \
| \
{
	if [ "$OUTMODE" == "short" ]
	then
		awk '{print $1 " " $(NF-2) " " $(NF-1) " " $NF}'
	else
		cat
	fi
}

