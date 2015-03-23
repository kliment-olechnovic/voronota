#!/bin/bash

sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
-l /scratch/lustre/kliment/voromqa/output/casp/contacts_list_models \
-e cadscores \
-o /scratch/lustre/kliment/voromqa/output/casp/comparison/cadscores
