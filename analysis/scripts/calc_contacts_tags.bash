#!/bin/bash

set +e

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

BINDIR=""
WORKDIR=""
TAG_HBONDS=false
TAG_SSBONDS=false
TAG_SEQSEP=false

while getopts "b:d:lps" OPTION
do
	case $OPTION in
	h)
		echo "No help available, read sript file." 1>&2
		exit 0
		;;
	b)
		BINDIR=$OPTARG
		;;
	d)
		WORKDIR=$OPTARG
		;;
	l)
		TAG_SEQSEP=true
		;;
	p)
		TAG_HBONDS=true
		;;
	s)
		TAG_SSBONDS=true
		;;
	?)
		echo "Unrecognized option." 1>&2
		exit 1
		;;
	esac
done

if [ -z "$BINDIR" ]
then
	echo "Missing binaries directory." 1>&2
	exit 1
fi

if [ -z "$WORKDIR" ]
then
	echo "Missing working directory." 1>&2
	exit 1
fi

if [ ! -s "$BINDIR/voronota" ]
then
	echo "Invalid binaries directory." 1>&2
	exit 1
fi

if [ ! -s "$WORKDIR/contacts" ]
then
	echo "Invalid working directory." 1>&2
	exit 1
fi

cat $WORKDIR/contacts \
| $BINDIR/voronota query-contacts --drop-tags \
> $WORKDIR/contacts_nt

mv $WORKDIR/contacts_nt $WORKDIR/contacts

if $TAG_SEQSEP
then
	cat $WORKDIR/contacts \
	| $BINDIR/voronota query-contacts --match-min-seq-sep 1 --match-max-seq-sep 1 --no-solvent --set-tags 'sep1' \
	| $BINDIR/voronota query-contacts --match-min-seq-sep 2 --match-max-seq-sep 2 --no-solvent --set-tags 'sep2' \
	| $BINDIR/voronota query-contacts --match-min-seq-sep 3 --match-max-seq-sep 3 --no-solvent --set-tags 'sep3' \
	| $BINDIR/voronota query-contacts --match-min-seq-sep 4 --match-max-seq-sep 4 --no-solvent --set-tags 'sep4' \
	| $BINDIR/voronota query-contacts --no-same-chain --drop-tags \
	> $WORKDIR/contacts_wss
	
	mv $WORKDIR/contacts_wss $WORKDIR/contacts
fi

if $TAG_HBONDS
then
	cat $WORKDIR/balls \
	| $BINDIR/voronota query-balls --pdb-output $TMPDIR/refined.pdb \
	> /dev/null

	cp $BINDIR/hbplus $TMPDIR/hbplus
	cd $TMPDIR
	./hbplus ./refined.pdb > $TMPDIR/hbplus_log
	cd - &> /dev/null
	
	mv $TMPDIR/hbplus_log $WORKDIR/hbplus_log
	
	if [ -s "$TMPDIR/refined.hb2" ] && [ "$(cat $TMPDIR/refined.hb2 | wc -l)" -gt "8" ]
	then
		cp $TMPDIR/refined.hb2 $WORKDIR/hbplus_output
	
		cat $WORKDIR/contacts \
		| $BINDIR/voronota query-contacts --set-hbplus-tags $WORKDIR/hbplus_output \
		> $WORKDIR/contacts_whb
	
		mv $WORKDIR/contacts_whb $WORKDIR/contacts
	fi
fi

if $TAG_SSBONDS
then
	cat $WORKDIR/contacts \
	| $BINDIR/voronota query-contacts --set-tags 'ds' --match-first 'R<CYS>&A<SG>' --match-second 'R<CYS>&A<SG>' --match-max-dist 3.0 \
	> $WORKDIR/contacts_wds
	
	mv $WORKDIR/contacts_wds $WORKDIR/contacts
fi
