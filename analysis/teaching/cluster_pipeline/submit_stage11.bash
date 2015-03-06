#!/bin/bash

sbatch /scratch/lustre/kliment/voromqa/bin/cat_list_of_files.bash \
-l /scratch/lustre/kliment/voromqa/output/complexes/contacts_list \
-e count_of_residues_and_atoms \
-o /scratch/lustre/kliment/voromqa/output/complexes/counts_of_residues_and_atoms
