#!/bin/bash

find /scratch/lustre/kliment/voromqa/output/decoys/contacts/ -type f -name summary -not -empty \
> /scratch/lustre/kliment/voromqa/output/decoys/contacts_summaries_list

sbatch /scratch/lustre/kliment/voromqa/bin/calc_potential.bash \
-b /scratch/lustre/kliment/voromqa/bin \
-i /scratch/lustre/kliment/voromqa/output/decoys/contacts_summaries_list \
-o /scratch/lustre/kliment/voromqa/output/decoys/potential/full
