#!/bin/bash

set +e

WORKDIR=""

while getopts "d:" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	esac
done

mkdir -p $WORKDIR/vectorized_environments_by_residue

cat $WORKDIR/raw_contacts \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 2 \
| $BINDIR/voronota vectorize-contact-environments \
  --names-file $BINDIR/environment_names \
  --inter-residue \
  --normalize \
  --output-files-prefix $WORKDIR/vectorized_environments_by_residue/ \
> /dev/null
