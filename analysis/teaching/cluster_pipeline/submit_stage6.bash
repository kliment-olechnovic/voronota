#!/bin/bash

sbatch /scratch/lustre/kliment/voromqa/bin/analyze_potential_vectors.bash \
-d /scratch/lustre/kliment/voromqa/output/complexes/potential \
-n potential_without_tags \
-o /scratch/lustre/kliment/voromqa/output/complexes/sampled_potentials

sbatch /scratch/lustre/kliment/voromqa/bin/analyze_potential_vectors.bash \
-d /scratch/lustre/kliment/voromqa/output/complexes/potential \
-n potential_with_tags \
-o /scratch/lustre/kliment/voromqa/output/complexes/sampled_potentials
