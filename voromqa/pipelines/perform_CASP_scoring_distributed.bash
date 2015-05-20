#!/bin/bash

set +e

BINDIR="./bin"
INPUT_LIST_FILE="./input"
OUTPUTDIR="./output"
CPUCOUNT="150"
STEPNAMES=""
SCHEDULER="sbatch"

while getopts "b:i:o:p:s:c:" OPTION
do
	case $OPTION in
	b)
		BINDIR=$OPTARG
		;;
	i)
		INPUT_LIST_FILE=$OPTARG
		;;
	o)
		OUTPUTDIR=$OPTARG
		;;
	p)
		CPUCOUNT=$OPTARG
		;;
	s)
		STEPNAMES=$OPTARG
		;;
	c)
		SCHEDULER=$OPTARG
		;;
	esac
done

if [[ $STEPNAMES == *"[raw_contacts]"* ]]
then
	mkdir -p $OUTPUTDIR
	
	cat $INPUT_LIST_FILE | while read CASPNAME TARGETNAME
	do
		$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_raw_contacts_from_CASP_target_models.bash -c $CASPNAME -t $TARGETNAME -o $OUTPUTDIR/entries"
	done
	
	exit 0
fi

if [[ $STEPNAMES == *"[contacts_and_summary]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name raw_contacts -not -empty | sed 's/raw_contacts$//' > $OUTPUTDIR/list_of_entries_with_raw_contacts
	INCOUNT=$(cat $OUTPUTDIR/list_of_entries_with_raw_contacts | wc -l)
	
	cat $OUTPUTDIR/list_of_entries_with_raw_contacts \
	| xargs -L $(echo "$INCOUNT/$CPUCOUNT" | bc) \
	$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -d"
	
	exit 0
fi

if [[ $STEPNAMES == *"[energies]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_contacts
	INCOUNT=$(cat $OUTPUTDIR/list_of_entries_with_contacts | wc -l)
	
	cat $OUTPUTDIR/list_of_entries_with_contacts \
	| xargs -L $(echo "$INCOUNT/$CPUCOUNT" | bc) \
	$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $BINDIR/potential -d"
	
	exit 0
fi

if [[ $STEPNAMES == *"[quality_scores]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty | sed 's/atom_energies$//' > $OUTPUTDIR/list_of_entries_with_atom_energies
	INCOUNT=$(cat $OUTPUTDIR/list_of_entries_with_atom_energies | wc -l)
	
	cat $OUTPUTDIR/list_of_entries_with_atom_energies \
	| xargs -L $(echo "$INCOUNT/$CPUCOUNT" | bc) \
	$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_quality_scores_from_energies.bash -e $BINDIR/energy_means_and_sds -d"
	
	exit 0
fi
