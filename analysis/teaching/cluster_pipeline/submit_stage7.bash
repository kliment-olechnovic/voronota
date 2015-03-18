#!/bin/bash

for i in {0..5}
do
	cat /scratch/lustre/kliment/voromqa/output/complexes/contacts_list \
	| xargs -L 200 -P 1 \
	sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
	"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y $i -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_without_tags -n energy_without_tags_iss$i -d"
	
	cat /scratch/lustre/kliment/voromqa/output/complexes/contacts_list \
	| xargs -L 200 -P 1 \
	sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
	"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y $i -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_with_tags -t -n energy_with_tags_iss$i -d"
done
