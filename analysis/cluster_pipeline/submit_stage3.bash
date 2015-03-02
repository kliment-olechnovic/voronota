#!/bin/bash

find /scratch/lustre/kliment/voromqa/output/complexes/contacts/ -type f -name summary -not -empty \
> /scratch/lustre/kliment/voromqa/output/complexes/contacts_summaries_list

sbatch /scratch/lustre/kliment/voromqa/bin/calc_potential.bash \
-b /scratch/lustre/kliment/voromqa/bin \
-i /scratch/lustre/kliment/voromqa/output/complexes/contacts_summaries_list \
-o /scratch/lustre/kliment/voromqa/output/complexes/potential/full

for i in {1..1000}
do
	sbatch /scratch/lustre/kliment/voromqa/bin/calc_potential.bash \
	-b /scratch/lustre/kliment/voromqa/bin \
	-i /scratch/lustre/kliment/voromqa/output/complexes/contacts_summaries_list \
	-o /scratch/lustre/kliment/voromqa/output/complexes/potential/half \
	-r -s 7000
done
