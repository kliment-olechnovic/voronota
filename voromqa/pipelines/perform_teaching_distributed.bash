#!/bin/bash

set +e

BINDIR="./bin"
URLS_LIST_FILE="./input"
OUTPUTDIR="./output"
CPUCOUNT="150"
PARTIAL_POTENTIALS="1000"
STEPNAMES=""
SCHEDULER="sbatch"

while getopts "b:i:o:p:r:s:c:" OPTION
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
	
	INCOUNT=$(cat $URLS_LIST_FILE | wc -l)
	
	cat $URLS_LIST_FILE \
	| xargs -L $(echo "$INCOUNT/$CPUCOUNT" | bc) \
	$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_raw_contacts_from_atoms.bash -u -z -m -o $OUTPUTDIR/entries -i"
	
	exit 0
fi

if [[ $STEPNAMES == *"[contacts_and_summary]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name raw_contacts -not -empty | sed 's/raw_contacts$//' > $OUTPUTDIR/list_of_entries_with_raw_contacts
	INCOUNT=$(cat $OUTPUTDIR/list_of_entries_with_raw_contacts | wc -l)
	
	cat $OUTPUTDIR/list_of_entries_with_raw_contacts \
	| xargs -L $(echo "$INCOUNT/$CPUCOUNT" | bc) \
	$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -m -d"
	
	exit 0
fi

if [[ $STEPNAMES == *"[potential]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name summary -not -empty > $OUTPUTDIR/list_of_summaries
	INCOUNT=$(cat $OUTPUTDIR/list_of_summaries | wc -l)
	
	$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential -i $OUTPUTDIR/list_of_summaries -c $BINDIR/contributions_from_casp_models"
	
	exit 0
fi

if [[ $STEPNAMES == *"[energies]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_contacts
	INCOUNT=$(cat $OUTPUTDIR/list_of_entries_with_contacts | wc -l)
	
	cat $OUTPUTDIR/list_of_entries_with_contacts \
	| xargs -L $(echo "$INCOUNT/$CPUCOUNT" | bc) \
	$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $OUTPUTDIR/potential/potential -d"
	
	exit 0
fi

if [[ $STEPNAMES == *"[energies_stats]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty > $OUTPUTDIR/list_of_atom_energies
	
	$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_energies_stats_from_energies.bash -o $OUTPUTDIR/energies_stats -i $OUTPUTDIR/list_of_atom_energies"
	
	exit 0
fi

if [[ $STEPNAMES == *"[partial_potentials]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name summary -not -empty > $OUTPUTDIR/list_of_summaries
	INCOUNT=$(cat $OUTPUTDIR/list_of_summaries | wc -l)
	
	yes $(echo "$INCOUNT/2" | bc) | head -n $PARTIAL_POTENTIALS \
	| xargs -L $(echo "$PARTIAL_POTENTIALS/$CPUCOUNT" | bc) \
	$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/partial_potentials -i $OUTPUTDIR/list_of_summaries -c $BINDIR/contributions_from_casp_models -r"
	
	exit 0
fi

if [[ $STEPNAMES == *"[partial_potentials_stats]"* ]]
then
	find $OUTPUTDIR/partial_potentials/ -type f -name potential -not -empty > $OUTPUTDIR/list_of_partial_potentials
	
	$SCHEDULER $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_potentials_stats_from_potentials.bash -o $OUTPUTDIR/partial_potentials_stats -i $OUTPUTDIR/list_of_partial_potentials"
	
	exit 0
fi
