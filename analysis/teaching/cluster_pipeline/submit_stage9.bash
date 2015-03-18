#!/bin/bash

for i in {0..5}
do
	sbatch /scratch/lustre/kliment/voromqa/bin/estimate_distributions.bash \
	-i /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_without_tags_iss${i}_atoms \
	-p /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_without_tags_iss${i}_atoms_histograms.pdf \
	-t /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_without_tags_iss${i}_atoms_means_and_sds
	
	sbatch /scratch/lustre/kliment/voromqa/bin/estimate_distributions.bash \
	-i /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_with_tags_iss${i}_atoms \
	-p /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_with_tags_iss${i}_atoms_histograms.pdf \
	-t /scratch/lustre/kliment/voromqa/output/complexes/energy/energy_with_tags_iss${i}_atoms_means_and_sds
done
