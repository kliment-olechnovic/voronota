#!/bin/bash

set +e

BINDIR="./bin"
URLS_LIST_FILE="./input"
OUTPUTDIR="./output/teaching"
CPUCOUNT="4"
PARTIAL_POTENTIALS="10"
STEPNAMES=""
SCHEDULER="sbatch"
FILTER_FILE=""
FILTER_NAME=""

while getopts "b:i:o:p:r:s:c:f:n:" OPTION
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
	f)
		FILTER_FILE=$OPTARG
		;;
	n)
		FILTER_NAME=$OPTARG
		;;
	esac
done

if [[ $STEPNAMES == *"[balls]"* ]]
then
	mkdir -p $OUTPUTDIR
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/get_balls_from_atoms_link.bash -u -z -m -o $OUTPUTDIR/entries -i" $URLS_LIST_FILE
	exit 0
fi

if [[ $STEPNAMES == *"[raw_contacts]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name balls -not -empty | sed 's/balls$//' > $OUTPUTDIR/list_of_entries_with_balls
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_raw_contacts_from_balls.bash -c -d" $OUTPUTDIR/list_of_entries_with_balls
	exit 0
fi

if [[ $STEPNAMES == *"[contacts_and_summary]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name raw_contacts -not -empty | sed 's/raw_contacts$//' > $OUTPUTDIR/list_of_entries_with_raw_contacts
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -m -d" $OUTPUTDIR/list_of_entries_with_raw_contacts
	exit 0
fi

if [[ $STEPNAMES == *"[potential]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name summary -not -empty > $OUTPUTDIR/list_of_summaries
	$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential -i $OUTPUTDIR/list_of_summaries -c $BINDIR/contributions_from_casp_models"
	exit 0
fi

if [[ $STEPNAMES == *"[potential_filtered]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name summary -not -empty | grep -f $FILTER_FILE > $OUTPUTDIR/list_of_summaries_filtered_$FILTER_NAME
	$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential_filtered_$FILTER_NAME -i $OUTPUTDIR/list_of_summaries_filtered_$FILTER_NAME -c $BINDIR/contributions_from_casp_models"
	exit 0
fi

if [[ $STEPNAMES == *"[energies]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_contacts
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $OUTPUTDIR/potential/potential -d" $OUTPUTDIR/list_of_entries_with_contacts
	exit 0
fi

if [[ $STEPNAMES == *"[energies_stats]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty > $OUTPUTDIR/list_of_atom_energies
	$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/calc_energies_stats_from_energies.bash -o $OUTPUTDIR/energies_stats -i $OUTPUTDIR/list_of_atom_energies"
	exit 0
fi

if [[ $STEPNAMES == *"[energies_stats_filtered]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty  | grep -f $FILTER_FILE > $OUTPUTDIR/list_of_atom_energies_filtered_$FILTER_NAME
	$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/calc_energies_stats_from_energies.bash -o $OUTPUTDIR/energies_stats_filtered_$FILTER_NAME -i $OUTPUTDIR/list_of_atom_energies_filtered_$FILTER_NAME"
	exit 0
fi

if [[ $STEPNAMES == *"[partial_potentials]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name summary -not -empty > $OUTPUTDIR/list_of_summaries
	yes $(echo "$(cat $OUTPUTDIR/list_of_summaries | wc -l)/2" | bc) | head -n $PARTIAL_POTENTIALS > $OUTPUTDIR/list_of_partial_potentials_input_sizes
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/partial_potentials -i $OUTPUTDIR/list_of_summaries -c $BINDIR/contributions_from_casp_models -r" $OUTPUTDIR/list_of_partial_potentials_input_sizes
	exit 0
fi

if [[ $STEPNAMES == *"[partial_potentials_stats]"* ]]
then
	find $OUTPUTDIR/partial_potentials/ -type f -name potential -not -empty > $OUTPUTDIR/list_of_partial_potentials
	$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/calc_potentials_stats_from_potentials.bash -o $OUTPUTDIR/partial_potentials_stats -i $OUTPUTDIR/list_of_partial_potentials"
	exit 0
fi
