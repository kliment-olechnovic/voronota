#!/bin/bash

find /scratch/lustre/kliment/voromqa/output/casp/contacts/ -type f -name hbplus_output -not -empty | while read FNAME
do
	echo $(dirname $FNAME)
done > /scratch/lustre/kliment/voromqa/output/casp/contacts_list

for i in {0..5}
do
	cat /scratch/lustre/kliment/voromqa/output/casp/contacts_list \
	| xargs -L 200 -P 1 \
	sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
	"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y $i -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_with_tags -t -n energy_with_tags_iss${i} -q /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_with_tags_iss${i}_atoms_means_and_sds -d"
done
