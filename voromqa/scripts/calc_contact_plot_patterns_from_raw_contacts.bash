#!/bin/bash

WORKDIR=""

while getopts "d:" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	esac
done

cat $WORKDIR/raw_contacts \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 6 \
  --no-solvent \
  --match-first-not 'A<C,CA,N,O>' \
  --match-second-not 'A<C,CA,N,O>' \
| $BINDIR/voronota query-contacts \
  --inter-residue \
| $BINDIR/voronota plot-contacts \
  --no-contraction \
  --patterns-output $WORKDIR/contact_plot_patterns

( cat $WORKDIR/contact_plot_patterns | awk '{print $3}' | tr '\n' ' ' ; echo ) > $WORKDIR/contact_plot_patterns_in_line
mv $WORKDIR/contact_plot_patterns_in_line $WORKDIR/contact_plot_patterns
