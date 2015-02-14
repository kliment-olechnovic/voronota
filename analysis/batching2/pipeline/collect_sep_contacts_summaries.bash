#!/bin/bash

readonly CONTACTSNAME=$1
readonly SEQSEP=$2

cat /scratch/lustre/home/kliment/voromqa/output_derivatives/sizes \
| awk '{if($2>500) print "/scratch/lustre/home/kliment/voromqa/output/" $1 "/CONTACTSNAME_seqsep_SEQSEP_summary"}' \
| sed "s/CONTACTSNAME/$CONTACTSNAME/" \
| sed "s/SEQSEP/$SEQSEP/" \
| /scratch/lustre/home/kliment/voromqa/bin/voronota score-contacts-potential --input-file-list \
--potential-file "/scratch/lustre/home/kliment/voromqa/output_derivatives/${CONTACTSNAME}_seqsep_${SEQSEP}_summary_potential" \
> "/scratch/lustre/home/kliment/voromqa/output_derivatives/${CONTACTSNAME}_seqsep_${SEQSEP}_summary"
