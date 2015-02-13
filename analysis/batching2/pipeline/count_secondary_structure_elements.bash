#!/bin/bash

cat ./input/hqchains.list \
| xargs -L 100 -P 1 sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash "/scratch/lustre/kliment/voromqa/bin/count_secondary_structure_elements.bash /scratch/lustre/kliment/voromqa/bin /scratch/lustre/kliment/voromqa/output"
