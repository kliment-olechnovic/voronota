#!/bin/bash

cat /scratch/lustre/kliment/voromqa/output/casp/contacts_list_targets | while read TPATH
do
	TPATH=$(dirname $TPATH)
	for i in {1..6}
	do
		sbatch /scratch/lustre/kliment/voromqa/bin/analyze_casp_target_and_models_energies.bash \
		-t $TPATH \
		-n energy_without_tags_iss${i}_atoms \
		-o /scratch/lustre/kliment/voromqa/output/casp/energy_plots

		sbatch /scratch/lustre/kliment/voromqa/bin/analyze_casp_target_and_models_energies.bash \
		-t $TPATH \
		-n energy_with_tags_iss${i}_atoms \
		-o /scratch/lustre/kliment/voromqa/output/casp/energy_plots
	done
done
