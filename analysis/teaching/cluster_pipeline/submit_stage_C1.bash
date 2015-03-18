#!/bin/bash

cat ./input/artificial_decoys_target_list \
| xargs -L 1 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/prepare_artificial_decoys_target_data.bash \
-b /scratch/lustre/kliment/voromqa/bin \
-o /scratch/lustre/kliment/voromqa/output/decoys/contacts \
-t
