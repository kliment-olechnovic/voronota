#!/bin/bash

sbatch /scratch/lustre/kliment/voromqa/bin/calc_potential.bash \
-b /scratch/lustre/kliment/voromqa/bin \
-i /scratch/lustre/kliment/voromqa/output/casp/contacts_summaries_list_cadscore_below_04 \
-o /scratch/lustre/kliment/voromqa/output/casp/potential/cadscore_below_04

sbatch /scratch/lustre/kliment/voromqa/bin/calc_potential.bash \
-b /scratch/lustre/kliment/voromqa/bin \
-i /scratch/lustre/kliment/voromqa/output/casp/contacts_summaries_list_cadscore_above_04 \
-o /scratch/lustre/kliment/voromqa/output/casp/potential/cadscore_above_04
