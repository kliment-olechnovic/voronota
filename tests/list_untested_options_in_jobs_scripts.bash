#!/bin/bash

cd $(dirname "$0")

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

MODE="unknown"
../Release/voronota --help \
| sed "s/Command\s\+'\([[:alpha:]]\S*\)'/mode-\1 /" \
| tr ' ' '\n' | tr '\t' '\n' \
| egrep '^mode-|^--' \
| while read TOKEN
do
	if [[ "$TOKEN" == mode-* ]]
	then
		MODE=$TOKEN
	else
		echo "$MODE $TOKEN "
	fi
done | sort | uniq > $TMPDIR/all_options

MODE="unknown"
cat ./jobs_scripts/* \
| sed 's/$VORONOTA\s\+\([[:alpha:]]\S*\)/mode-\1 /g' \
| sed 's/$VORONOTA\s\+\(\S*\)//g' \
| tr ' ' '\n' | tr '\t' '\n' \
| egrep '^mode-|^--' \
| while read TOKEN
do
	if [[ "$TOKEN" == mode-* ]]
	then
		MODE=$TOKEN
	else
		echo "$MODE $TOKEN "
	fi
done | sort | uniq > $TMPDIR/tested_options

cat $TMPDIR/all_options | grep -v -f $TMPDIR/tested_options > $TMPDIR/untested_options

(cat $TMPDIR/tested_options | awk '{print $1 " " $2 " tested"}' ; cat $TMPDIR/untested_options | awk '{print $1 " " $2 " untested"}') \
| sed 's/^mode-//' \
| grep -v 'help' \
| sort \
| awk '{print $3 " " $1 " " $2}' \
| column -t
