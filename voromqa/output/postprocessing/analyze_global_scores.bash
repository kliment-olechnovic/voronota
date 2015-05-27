#!/bin/bash

SCRIPTDIR=$(dirname $0)
INFILE=$1
OUTDIR=$2

SUBDIR=$OUTDIR/qscore_vs_cadscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name qscore_atom \
  V-refscore-name cadscore_residue \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log

SUBDIR=$OUTDIR/qscore_vs_tmscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name qscore_atom \
  V-refscore-name tmscore \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log

SUBDIR=$OUTDIR/goap_vs_cadscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name goap F-invert-testscore F-normalize-testscore \
  V-refscore-name cadscore_residue \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log

SUBDIR=$OUTDIR/goap_vs_tmscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name goap F-invert-testscore F-normalize-testscore \
  V-refscore-name tmscore \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log

SUBDIR=$OUTDIR/dfire_vs_cadscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name dfire F-invert-testscore F-normalize-testscore \
  V-refscore-name cadscore_residue \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log

SUBDIR=$OUTDIR/dfire_vs_tmscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name dfire F-invert-testscore F-normalize-testscore \
  V-refscore-name tmscore \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log

SUBDIR=$OUTDIR/qenergy_vs_cadscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name qenergy F-invert-testscore F-normalize-testscore-by-area \
  V-refscore-name cadscore_residue \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log

SUBDIR=$OUTDIR/qenergy_vs_tmscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name qenergy F-invert-testscore F-normalize-testscore-by-area \
  V-refscore-name tmscore \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log

SUBDIR=$OUTDIR/goap_vs_qscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name goap F-invert-testscore F-normalize-testscore \
  V-refscore-name qscore_atom \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log

SUBDIR=$OUTDIR/dfire_vs_qscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name dfire F-invert-testscore F-normalize-testscore \
  V-refscore-name qscore_atom \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log

SUBDIR=$OUTDIR/qenergy_vs_qscore
mkdir -p $SUBDIR
R --vanilla --args \
  V-input $INFILE \
  V-testscore-name qenergy F-invert-testscore F-normalize-testscore-by-area \
  V-refscore-name qscore_atom \
  V-pdf-output $SUBDIR/plots.pdf \
  F-plot-per-target \
< $SCRIPTDIR/analyze_global_scores.R \
| grep 'Results statistics output' -A 9999 \
> $SUBDIR/log
