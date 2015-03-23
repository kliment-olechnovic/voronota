args=commandArgs(TRUE);

names=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
values=read.table(args[2], header=FALSE, stringsAsFactors=FALSE);
refs=read.table(args[3], header=FALSE, stringsAsFactors=FALSE);

sel=which(names$V3!="target");

length(names[[1]]);
length(values[[1]]);
length(refs[[1]]);
length(sel);

names=names[sel,];
values=values[sel,];

qscores=values;
cadscores=refs$V2;

plot(cadscores, qscores, xlim=c(0, 1), ylim=c(0, 1));

targets=sort(union(names$V2, names$V2));

cors=c();

for(target in targets)
{
	sel=which(names$V2==target);
	if(length(sel)>1)
	{
		plot(cadscores[sel], qscores[sel], xlim=c(0, 1), ylim=c(0, 1), main=target);
		cors=c(cors, cor(cadscores[sel], qscores[sel]));
	}
}

cor(cadscores, qscores);

quantile(cors);

mean(cors);
