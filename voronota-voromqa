#!/bin/bash

set +e

BINDIR=$(dirname $0)

INFILE=""
OUTFILE_ATOM_SCORES=""
OUTFILE_RESIDUE_SCORES=""
SMOOTHING_WINDOW="5"
HELP_MODE=false

while getopts "i:a:r:s:h" OPTION
do
	case $OPTION in
	i)
		INFILE=$OPTARG
		;;
	a)
		OUTFILE_ATOM_SCORES=$OPTARG
		;;
	r)
		OUTFILE_RESIDUE_SCORES=$OPTARG
		;;
	s)
		SMOOTHING_WINDOW=$OPTARG
		;;
	h)
		HELP_MODE=true
		;;
	esac
done

if [ -z "$INFILE" ] || $HELP_MODE
then
cat >&2 << EOF
Script parameters:
    -i input_file.pdb
    [-a output_atom_scores_file.pdb]
    [-r output_residue_scores_file.pdb]
    [-s residue_scores_smoothing_window_size]
EOF
exit 1
fi

if [ ! -s "$BINDIR/voronota" ] || [ ! -s "$BINDIR/resources/radii" ] || [ ! -s "$BINDIR/resources/energy_potential" ] || [ ! -s "$BINDIR/resources/energy_means_and_sds" ]
then
	echo >&2 "Error: script directory does not contain Voronota files"
	exit 1
fi

if [ ! -s "$INFILE" ]
then
	echo >&2 "Error: input file does not exist or is empty"
	exit 1
fi

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

INPRINTER="cat"
file $INFILE > $TMPDIR/input_file_type
if grep "gzip compressed data" $TMPDIR/input_file_type > /dev/null
then
	INPRINTER="zcat"
fi

$INPRINTER $INFILE \
| $BINDIR/voronota get-balls-from-atoms-file \
  --annotated \
  --radii-file $BINDIR/resources/radii \
| $BINDIR/voronota query-balls \
  --drop-adjuncts \
  --drop-altloc-indicators \
| tee $TMPDIR/balls \
| $BINDIR/voronota calculate-contacts \
  --annotated \
  --tag-centrality \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 1 \
| $BINDIR/voronota query-contacts \
  --match-first 'A<C>' \
  --match-second 'A<N>' \
  --match-max-seq-sep 1 \
  --match-max-dist 1.6 \
  --invert \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 1 \
  --match-max-seq-sep 1 \
  --set-tags 'sep1' \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 2 \
  --no-solvent \
  --set-tags 'sep2' \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| tr ';' '_' \
| tee $TMPDIR/contacts \
| $BINDIR/voronota score-contacts-energy \
  --potential-file $BINDIR/resources/energy_potential \
  --atom-scores-file $TMPDIR/atom_energies \
> /dev/null

cat $TMPDIR/contacts \
| $BINDIR/voronota query-contacts-depth-values \
> $TMPDIR/depth_values

cat $TMPDIR/atom_energies \
| $BINDIR/voronota score-contacts-quality \
  --default-mean -0.34 \
  --default-sd 0.19 \
  --means-and-sds-file $BINDIR/resources/energy_means_and_sds \
  --external-weights-file $TMPDIR/depth_values \
  --smoothing-window $SMOOTHING_WINDOW \
  --atom-scores-file $TMPDIR/atom_quality_scores \
  --residue-scores-file $TMPDIR/residue_quality_scores \
> $TMPDIR/global_quality_score

if [ -n "$OUTFILE_ATOM_SCORES" ]
then
	mkdir -p $(dirname $OUTFILE_ATOM_SCORES)
	cat $TMPDIR/balls \
	| $BINDIR/voronota query-balls \
	  --set-external-adjuncts $TMPDIR/atom_quality_scores \
	  --set-external-adjuncts-name aqscore \
	| $BINDIR/voronota write-balls-to-atoms-file \
	  --pdb-output $OUTFILE_ATOM_SCORES \
	  --pdb-output-b-factor aqscore \
	> /dev/null
fi

if [ -n "$OUTFILE_RESIDUE_SCORES" ]
then
	mkdir -p $(dirname $OUTFILE_RESIDUE_SCORES)
	cat $TMPDIR/balls \
	| $BINDIR/voronota query-balls \
	  --set-external-adjuncts $TMPDIR/residue_quality_scores \
	  --set-external-adjuncts-name rqscore \
	| $BINDIR/voronota write-balls-to-atoms-file \
	  --pdb-output $OUTFILE_RESIDUE_SCORES \
	  --pdb-output-b-factor rqscore \
	> /dev/null
fi

cat $TMPDIR/global_quality_score \
| sed 's/^/VoroMQA global score = /'
