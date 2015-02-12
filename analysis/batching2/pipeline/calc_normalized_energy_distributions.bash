#!/bin/bash

cat `cat /scratch/lustre/home/kliment/voromqa/output_derivatives/sizes | awk '{if($2>500) print "/scratch/lustre/home/kliment/voromqa/output/" $1 "/normalized_atom_energies"}'` \
> /scratch/lustre/home/kliment/voromqa/output_derivatives/normalized_atom_energies

R --vanilla --args \
/scratch/lustre/home/kliment/voromqa/output_derivatives/normalized_atom_energies \
/scratch/lustre/home/kliment/voromqa/output_derivatives/normalized_atom_energies_distributions_plots.pdf \
/scratch/lustre/home/kliment/voromqa/output_derivatives/normalized_atom_energies_distributions_parameters \
< /scratch/lustre/home/kliment/voromqa/bin/estimate_distributions.R

cat `cat /scratch/lustre/home/kliment/voromqa/output_derivatives/sizes | awk '{if($2>500) print "/scratch/lustre/home/kliment/voromqa/output/" $1 "/normalized_atom_energies_whb"}'` \
> /scratch/lustre/home/kliment/voromqa/output_derivatives/normalized_atom_energies_whb

R --vanilla --args \
/scratch/lustre/home/kliment/voromqa/output_derivatives/normalized_atom_energies_whb \
/scratch/lustre/home/kliment/voromqa/output_derivatives/normalized_atom_energies_whb_distributions_plots.pdf \
/scratch/lustre/home/kliment/voromqa/output_derivatives/normalized_atom_energies_whb_distributions_parameters \
< /scratch/lustre/home/kliment/voromqa/bin/estimate_distributions.R

