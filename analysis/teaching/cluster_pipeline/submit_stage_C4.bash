#!/bin/bash

for i in {0..5}
do
	sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
	-l /scratch/lustre/kliment/voromqa/output/decoys/contacts_list \
	-e energy_with_tags_iss${i}_global \
	-o /scratch/lustre/kliment/voromqa/output/decoys/energy/energy_with_tags_iss${i}_global
	
	sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
	-l /scratch/lustre/kliment/voromqa/output/decoys/contacts_list \
	-e energy_with_tags_iss${i}_global_quality \
	-o /scratch/lustre/kliment/voromqa/output/decoys/energy/energy_with_tags_iss${i}_global_quality
done
