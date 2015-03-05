#!/bin/bash

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 1 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_without_tags -n energy_without_tags_iss1 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 1 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_with_tags -t -n energy_with_tags_iss1 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 2 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_without_tags -n energy_without_tags_iss2 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 2 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_with_tags -t -n energy_with_tags_iss2 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 3 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_without_tags -n energy_without_tags_iss3 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 3 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_with_tags -t -n energy_with_tags_iss3 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 4 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_without_tags -n energy_without_tags_iss4 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 4 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_with_tags -t -n energy_with_tags_iss4 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 5 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_without_tags -n energy_without_tags_iss5 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 5 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_with_tags -t -n energy_with_tags_iss5 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 6 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_without_tags -n energy_without_tags_iss6 -d"

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_energy.bash -b /scratch/lustre/kliment/voromqa/bin -s -e 2 -y 6 -p /scratch/lustre/kliment/voromqa/output/complexes/potential/full/potential_with_tags -t -n energy_with_tags_iss6 -d"
