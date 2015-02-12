#!/bin/bash

cat `cat /scratch/lustre/home/kliment/voromqa/output_derivatives/sizes | awk '{if($2>500) print "/scratch/lustre/home/kliment/voromqa/output/" $1 "/atom_qscores"}'` \
| sed 's/.*\(R<.*>A<.*>\)/\1/' \
> /scratch/lustre/home/kliment/voromqa/output_derivatives/atom_qscores

R --vanilla --args \
/scratch/lustre/home/kliment/voromqa/output_derivatives/atom_qscores \
/scratch/lustre/home/kliment/voromqa/output_derivatives/atom_qscores_distributions_plots.pdf \
/scratch/lustre/home/kliment/voromqa/output_derivatives/atom_qscores_distributions_parameters \
< /scratch/lustre/home/kliment/voromqa/bin/estimate_distributions.R

cat `cat /scratch/lustre/home/kliment/voromqa/output_derivatives/sizes | awk '{if($2>500) print "/scratch/lustre/home/kliment/voromqa/output/" $1 "/atom_qscores_whb"}'` \
| sed 's/.*\(R<.*>A<.*>\)/\1/' \
> /scratch/lustre/home/kliment/voromqa/output_derivatives/atom_qscores_whb

R --vanilla --args \
/scratch/lustre/home/kliment/voromqa/output_derivatives/atom_qscores_whb \
/scratch/lustre/home/kliment/voromqa/output_derivatives/atom_qscores_whb_distributions_plots.pdf \
/scratch/lustre/home/kliment/voromqa/output_derivatives/atom_qscores_whb_distributions_parameters \
< /scratch/lustre/home/kliment/voromqa/bin/estimate_distributions.R

