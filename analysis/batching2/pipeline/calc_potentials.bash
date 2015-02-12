#!/bin/bash

cat /scratch/lustre/home/kliment/voromqa/output_derivatives/sizes \
| awk '{if($2>500) print "/scratch/lustre/home/kliment/voromqa/output/" $1 "/contacts_summary"}' \
| /scratch/lustre/home/kliment/voromqa/bin/voronota score-contacts-potential --input-file-list --potential-file /scratch/lustre/home/kliment/voromqa/output_derivatives/potential \
> /scratch/lustre/home/kliment/voromqa/output_derivatives/contacts_summary

cat /scratch/lustre/home/kliment/voromqa/output_derivatives/sizes \
| awk '{if($2>500) print "/scratch/lustre/home/kliment/voromqa/output/" $1 "/contacts_whb_summary"}' \
| /scratch/lustre/home/kliment/voromqa/bin/voronota score-contacts-potential --input-file-list --potential-file /scratch/lustre/home/kliment/voromqa/output_derivatives/potential_whb \
> /scratch/lustre/home/kliment/voromqa/output_derivatives/contacts_whb_summary

