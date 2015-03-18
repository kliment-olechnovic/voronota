#!/bin/bash

find /scratch/lustre/kliment/voromqa/output/decoys/contacts/ -type f -name contacts -not -empty | while read FNAME
do
	echo $(dirname $FNAME)
done > /scratch/lustre/kliment/voromqa/output/decoys/contacts_list

cat ./output/decoys/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_tags.bash -b /scratch/lustre/kliment/voromqa/bin -l -p -s -d"
