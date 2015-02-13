#!/bin/bash

cat /scratch/lustre/home/kliment/voromqa/output_derivatives/sizes | awk '{if($2>500) print $1}' | while read INPUTID
do
	paste \
	/scratch/lustre/home/kliment/voromqa/output/$INPUTID/size_summary \
	/scratch/lustre/home/kliment/voromqa/output/$INPUTID/atom_qscores_average \
	/scratch/lustre/home/kliment/voromqa/output/$INPUTID/atom_qscores_whb_average \
	/scratch/lustre/home/kliment/voromqa/output/$INPUTID/secondary_structure_element_counts
done > /scratch/lustre/home/kliment/voromqa/output_derivatives/global_values
