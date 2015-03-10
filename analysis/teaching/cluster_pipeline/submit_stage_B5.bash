#!/bin/bash

for i in {1..6}
do
	sbatch /scratch/lustre/kliment/voromqa/bin/estimate_distributions.bash \
	-i /scratch/lustre/kliment/voromqa/output/casp/energy/energy_without_tags_iss${i}_atoms \
	-p /scratch/lustre/kliment/voromqa/output/casp/energy/energy_without_tags_iss${i}_atoms_histograms.pdf \
	-t /scratch/lustre/kliment/voromqa/output/casp/energy/energy_without_tags_iss${i}_atoms_means_and_sds
	
	sbatch /scratch/lustre/kliment/voromqa/bin/estimate_distributions.bash \
	-i /scratch/lustre/kliment/voromqa/output/casp/energy/energy_with_tags_iss${i}_atoms \
	-p /scratch/lustre/kliment/voromqa/output/casp/energy/energy_with_tags_iss${i}_atoms_histograms.pdf \
	-t /scratch/lustre/kliment/voromqa/output/casp/energy/energy_with_tags_iss${i}_atoms_means_and_sds
done
