#!/bin/bash

set +e

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

BINDIR=""
WORKDIR=""

while getopts "b:d:" OPTION
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

TARGET_WORKDIR=$(dirname $(dirname $WORKDIR))/target

if [ ! -s "$TARGET_WORKDIR/contacts" ]
then
	echo "Invalid target working directory." 1>&2
	exit 1
fi

cat $TARGET_WORKDIR/contacts \
| $BINDIR/voronota query-contacts --match-min-seq-sep 1 --no-solvent \
| awk '{print $1 " " $2 " " $3}' \
> $TMPDIR/target

cat $WORKDIR/contacts \
| $BINDIR/voronota query-contacts --match-min-seq-sep 1 --no-solvent \
| awk '{print $1 " " $2 " " $3}' \
| $BINDIR/voronota compare-contacts --target-contacts-file $TMPDIR/target --detailed-output \
> $WORKDIR/cadscores
