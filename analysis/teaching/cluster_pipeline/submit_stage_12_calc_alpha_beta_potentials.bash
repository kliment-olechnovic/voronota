#!/bin/bash

cat /scratch/lustre/kliment/voromqa/output/complexes/contacts_summaries_list \
| grep -f /scratch/lustre/kliment/voromqa/input/pdb_ids_with_less_than_11_percent_beta \
> /scratch/lustre/kliment/voromqa/output/complexes/contacts_summaries_list_mostly_alpha

cat /scratch/lustre/kliment/voromqa/output/complexes/contacts_summaries_list \
| grep -f /scratch/lustre/kliment/voromqa/input/pdb_ids_with_less_than_20_percent_alpha \
> /scratch/lustre/kliment/voromqa/output/complexes/contacts_summaries_list_mostly_beta

sbatch /scratch/lustre/kliment/voromqa/bin/calc_potential.bash \
-b /scratch/lustre/kliment/voromqa/bin \
-i /scratch/lustre/kliment/voromqa/output/complexes/contacts_summaries_list_mostly_alpha \
-o /scratch/lustre/kliment/voromqa/output/complexes/potential/mostly_alpha/

sbatch /scratch/lustre/kliment/voromqa/bin/calc_potential.bash \
-b /scratch/lustre/kliment/voromqa/bin \
-i /scratch/lustre/kliment/voromqa/output/complexes/contacts_summaries_list_mostly_beta \
-o /scratch/lustre/kliment/voromqa/output/complexes/potential/mostly_beta/
