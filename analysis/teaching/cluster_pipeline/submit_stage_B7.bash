#!/bin/bash

cat /scratch/lustre/kliment/voromqa/output/casp/contacts_list_models \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_casp_model_cadscore.bash -b /scratch/lustre/kliment/voromqa/bin -d"
