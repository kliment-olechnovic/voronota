#!/bin/bash

for i in {10..11}
do
	cat ./input/casp${i}_ids \
	| xargs -L 1 -P 1 \
	sbatch /scratch/lustre/kliment/voromqa/bin/prepare_casp_target_data.bash \
	-b /scratch/lustre/kliment/voromqa/bin \
	-c $i \
	-o /scratch/lustre/kliment/voromqa/output/casp \
	-t
done
