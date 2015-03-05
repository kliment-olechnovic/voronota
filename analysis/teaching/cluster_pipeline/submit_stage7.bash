#!/bin/bash

for i in {1..6}
do
	sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
	-l /scratch/lustre/kliment/voromqa/output/complexes/contacts_list \
	-e energy_without_tags_iss${i}_global \
	-o /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_without_tags_iss${i}_global
	
	sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
	-l /scratch/lustre/kliment/voromqa/output/complexes/contacts_list \
	-e energy_with_tags_iss${i}_global \
	-o /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_with_tags_iss${i}_global
	
	sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
	-l /scratch/lustre/kliment/voromqa/output/complexes/contacts_list \
	-e energy_without_tags_iss${i}_atoms \
	-o /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_without_tags_iss${i}_atoms
	
	sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
	-l /scratch/lustre/kliment/voromqa/output/complexes/contacts_list \
	-e energy_with_tags_iss${i}_atoms \
	-o /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_with_tags_iss${i}_atoms
done
