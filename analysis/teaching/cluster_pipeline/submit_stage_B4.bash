#!/bin/bash

cat /scratch/lustre/kliment/voromqa/output/casp/contacts_list | grep models > /scratch/lustre/kliment/voromqa/output/casp/contacts_list_models
cat /scratch/lustre/kliment/voromqa/output/casp/contacts_list | grep target > /scratch/lustre/kliment/voromqa/output/casp/contacts_list_targets

for i in {0..5}
do
	sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
	-l /scratch/lustre/kliment/voromqa/output/casp/contacts_list \
	-e energy_with_tags_iss${i}_global \
	-o /scratch/lustre/kliment/voromqa/output/casp/energy/energy_with_tags_iss${i}_global
	
	sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
	-l /scratch/lustre/kliment/voromqa/output/casp/contacts_list_models \
	-e energy_with_tags_iss${i}_atoms \
	-o /scratch/lustre/kliment/voromqa/output/casp/energy/models/energy_with_tags_iss${i}_atoms
	
	sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
	-l /scratch/lustre/kliment/voromqa/output/casp/contacts_list_targets \
	-e energy_with_tags_iss${i}_atoms \
	-o /scratch/lustre/kliment/voromqa/output/casp/energy/targets/energy_with_tags_iss${i}_atoms
done
