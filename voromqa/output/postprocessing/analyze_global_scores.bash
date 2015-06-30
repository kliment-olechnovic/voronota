#!/bin/bash

SCRIPTDIR=$(dirname $0)
INFILE=$1
OUTDIR=$2
MAINSCORE=$3

if [ -z "$MAINSCORE" ]
then
	MAINSCORE="qscore_atom"
fi

function analyze
{
	local TESTSCORE=$1
	local REFSCORE=$2
	local FLAGS=$3
	local SUBDIR=$OUTDIR/${TESTSCORE}__vs__${REFSCORE}
	mkdir -p $SUBDIR
	R --vanilla --args \
	  V-input $INFILE $FLAGS \
	  V-testscore-name $TESTSCORE \
	  V-refscore-name $REFSCORE \
	  V-output-prefix $SUBDIR/ \
	  F-plot-per-target \
	< $SCRIPTDIR/analyze_global_scores.R \
	> $SUBDIR/log
	cat $SUBDIR/results_table | column -t | sponge $SUBDIR/results_table
	cat $SUBDIR/results_summary | column -t | sponge $SUBDIR/results_summary
}

analyze "$MAINSCORE" "cadscore_residue"
analyze "$MAINSCORE" "cadscore_atom"
analyze "$MAINSCORE" "tmscore"
analyze "$MAINSCORE" "goap" "F-invert-refscore F-normalize-refscore"
analyze "$MAINSCORE" "dfire" "F-invert-refscore F-normalize-refscore"
analyze "$MAINSCORE" "goap_ag" "F-invert-refscore F-normalize-refscore"
analyze "$MAINSCORE" "rwplus" "F-invert-refscore F-normalize-refscore"
analyze "$MAINSCORE" "doop" "F-invert-refscore F-normalize-refscore"

analyze "qenergy" "cadscore_residue" "F-invert-testscore F-normalize-testscore-by-area"
analyze "qenergy" "cadscore_atom" "F-invert-testscore F-normalize-testscore-by-area"
analyze "qenergy" "tmscore" "F-invert-testscore F-normalize-testscore-by-area"

analyze "qarea" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "qarea" "cadscore_atom" "F-invert-testscore F-normalize-testscore"
analyze "qarea" "tmscore" "F-invert-testscore F-normalize-testscore"

analyze "qsas" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "qsas" "cadscore_atom" "F-invert-testscore F-normalize-testscore"
analyze "qsas" "tmscore" "F-invert-testscore F-normalize-testscore"

analyze "goap" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "goap" "cadscore_atom" "F-invert-testscore F-normalize-testscore"
analyze "goap" "tmscore" "F-invert-testscore F-normalize-testscore"

analyze "dfire" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "dfire" "cadscore_atom" "F-invert-testscore F-normalize-testscore"
analyze "dfire" "tmscore" "F-invert-testscore F-normalize-testscore"

analyze "goap_ag" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "goap_ag" "cadscore_atom" "F-invert-testscore F-normalize-testscore"
analyze "goap_ag" "tmscore" "F-invert-testscore F-normalize-testscore"

analyze "rwplus" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "rwplus" "cadscore_atom" "F-invert-testscore F-normalize-testscore"
analyze "rwplus" "tmscore" "F-invert-testscore F-normalize-testscore"

analyze "doop" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "doop" "cadscore_atom" "F-invert-testscore F-normalize-testscore"
analyze "doop" "tmscore" "F-invert-testscore F-normalize-testscore"

{
	find $OUTDIR -type f -name results_summary | head -1 | xargs -L 1 head -1
	find $OUTDIR -type f -name results_summary | xargs -L 1 tail -1 | grep cadscore_residue | sort
} | column -t > $OUTDIR/results_summaries_for_cadscore_residue

{
	find $OUTDIR -type f -name results_summary | head -1 | xargs -L 1 head -1
	find $OUTDIR -type f -name results_summary | xargs -L 1 tail -1 | grep cadscore_atom | sort
} | column -t > $OUTDIR/results_summaries_for_cadscore_atom

{
	find $OUTDIR -type f -name results_summary | head -1 | xargs -L 1 head -1
	find $OUTDIR -type f -name results_summary | xargs -L 1 tail -1 | grep tmscore | sort
} | column -t > $OUTDIR/results_summaries_for_tmscore
