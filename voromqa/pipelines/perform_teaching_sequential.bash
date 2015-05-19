#!/bin/bash

set +e

BINDIR="./bin"
URLS_LIST_FILE="./input"
OUTPUTDIR="./output"
CPUCOUNT="1"
PARTIAL_POTENTIALS="10"

while getopts "b:i:o:p:r:" OPTION
do
	case $OPTION in
	b)
		BINDIR=$OPTARG
		;;
	i)
		URLS_LIST_FILE=$OPTARG
		;;
	o)
		OUTPUTDIR=$OPTARG
		;;
	p)
		CPUCOUNT=$OPTARG
		;;
	r)
		PARTIAL_POTENTIALS=$OPTARG
		;;
	esac
done

##############################

mkdir -p $OUTPUTDIR

INCOUNT=$(cat $URLS_LIST_FILE | wc -l)

cat $URLS_LIST_FILE \
| xargs -L $(echo "$INCOUNT/$CPUCOUNT" | bc) -P $CPUCOUNT \
$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_raw_contacts_from_atoms.bash -u -z -m -o $OUTPUTDIR/entries -i"

##############################

find $OUTPUTDIR/entries/ -type f -name raw_contacts -not -empty | sed 's/raw_contacts$//' > $OUTPUTDIR/list_of_entries_with_raw_contacts
INCOUNT=$(cat $OUTPUTDIR/list_of_entries_with_raw_contacts | wc -l)

cat $OUTPUTDIR/list_of_entries_with_raw_contacts \
| xargs -L $(echo "$INCOUNT/$CPUCOUNT" | bc) -P $CPUCOUNT \
$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -d"

##############################

find $OUTPUTDIR/entries/ -type f -name summary -not -empty > $OUTPUTDIR/list_of_summaries
INCOUNT=$(cat $OUTPUTDIR/list_of_summaries | wc -l)

$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential -i $OUTPUTDIR/list_of_summaries"

##############################

find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_contacts
INCOUNT=$(cat $OUTPUTDIR/list_of_entries_with_contacts | wc -l)

cat $OUTPUTDIR/list_of_entries_with_contacts \
| xargs -L $(echo "$INCOUNT/$CPUCOUNT" | bc) -P $CPUCOUNT \
$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $OUTPUTDIR/potential/potential -d"

##############################

find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty > $OUTPUTDIR/list_of_atom_energies

$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_energies_stats_from_energies.bash -o $OUTPUTDIR/energies_stats -i $OUTPUTDIR/list_of_atom_energies"

##############################

find $OUTPUTDIR/entries/ -type f -name summary -not -empty > $OUTPUTDIR/list_of_summaries
INCOUNT=$(cat $OUTPUTDIR/list_of_summaries | wc -l)

yes $(echo "$INCOUNT/2" | bc) | head -n $PARTIAL_POTENTIALS \
| xargs -L $(echo "$PARTIAL_POTENTIALS/$CPUCOUNT" | bc) -P $CPUCOUNT $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/partial_potentials -i $OUTPUTDIR/list_of_summaries -r"

##############################

find $OUTPUTDIR/partial_potentials/ -type f -name potential -not -empty > $OUTPUTDIR/list_of_partial_potentials

$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_potentials_stats_from_potentials.bash -o $OUTPUTDIR/partial_potentials_stats -i $OUTPUTDIR/list_of_partial_potentials"
