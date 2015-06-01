#!/bin/bash

set +e

BINDIR="./bin"
INPUT_LIST_FILE="./input"
OUTPUTDIR="./output/CASP_scoring"
CPUCOUNT="4"
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

if [[ $STEPNAMES == *"[balls_CASP]"* ]]
then
	mkdir -p $OUTPUTDIR
	
	cat $INPUT_LIST_FILE | while read CASPNAME TARGETNAME
	do
		$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/get_balls_from_CASP_target_models.bash -c $CASPNAME -t $TARGETNAME -o $OUTPUTDIR/entries"
	done
	
	exit 0
fi

if [[ $STEPNAMES == *"[balls_decoys99]"* ]]
then
	mkdir -p $OUTPUTDIR
	
	cat $INPUT_LIST_FILE | while read INPUTFILE
	do
		$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/get_balls_from_decoys99_set.bash -i $INPUTFILE -o $OUTPUTDIR/entries"
	done
	
	exit 0
fi

if [[ $STEPNAMES == *"[balls_RosettaDecoys]"* ]]
then
	mkdir -p $OUTPUTDIR
	
	cat $INPUT_LIST_FILE | while read INPUTFILE
	do
		$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/get_balls_from_RosettaDecoys_set.bash -i $INPUTFILE -o $OUTPUTDIR/entries"
	done
	
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
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -d" $OUTPUTDIR/list_of_entries_with_raw_contacts
	exit 0
fi

if [[ $STEPNAMES == *"[potential]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name summary -not -empty > $OUTPUTDIR/list_of_summaries
	$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential -i $OUTPUTDIR/list_of_summaries"
	exit 0
fi

if [[ $STEPNAMES == *"[energies]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_contacts
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $BINDIR/potential -d" $OUTPUTDIR/list_of_entries_with_contacts
	exit 0
fi

if [[ $STEPNAMES == *"[quality_scores]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty | sed 's/atom_energies$//' > $OUTPUTDIR/list_of_entries_with_atom_energies
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_quality_scores_from_energies.bash -e $BINDIR/energy_means_and_sds -d" $OUTPUTDIR/list_of_entries_with_atom_energies
	exit 0
fi

if [[ $STEPNAMES == *"[atoms]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name balls -not -empty | sed 's/balls$//' > $OUTPUTDIR/list_of_entries_with_balls
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/collect_atoms_from_balls.bash -d" $OUTPUTDIR/list_of_entries_with_balls
	exit 0
fi

if [[ $STEPNAMES == *"[goap_scores]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_atoms
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_goap_scores_from_atoms.bash -d" $OUTPUTDIR/list_of_entries_with_atoms
	exit 0
fi

if [[ $STEPNAMES == *"[rwplus_score]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_atoms
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_rwplus_score_from_atoms.bash" $OUTPUTDIR/list_of_entries_with_atoms
	exit 0
fi

if [[ $STEPNAMES == *"[doop_score]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_atoms
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_doop_score_from_atoms.bash" $OUTPUTDIR/list_of_entries_with_atoms
	exit 0
fi

if [[ $STEPNAMES == *"[cad_scores]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | grep -v '/target/contacts$' | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_models_contacts
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_cad_scores_from_model_and_target_contacts.bash -d" $OUTPUTDIR/list_of_entries_with_models_contacts
	exit 0
fi

if [[ $STEPNAMES == *"[tmscore]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | grep -v '/target/atoms.pdb$' | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_models_atoms
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_tmscore_from_model_and_target_atoms.bash -d" $OUTPUTDIR/list_of_entries_with_models_atoms
	exit 0
fi

if [[ $STEPNAMES == *"[scores_list]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name global_quality_score -not -empty | sed 's/global_quality_score$//' > $OUTPUTDIR/list_of_entries_with_global_quality_score
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/collect_scores_list_from_working_directory.bash -d" $OUTPUTDIR/list_of_entries_with_global_quality_score
	exit 0
fi

if [[ $STEPNAMES == *"[concatenated_scores_lists]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name scores_list -not -empty > $OUTPUTDIR/list_of_scores_lists
	$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/concatenate_files_from_list_of_files.bash $OUTPUTDIR/list_of_scores_lists $OUTPUTDIR/concatenated_scores_lists"
	exit 0
fi

if [[ $STEPNAMES == *"[local_scores_evaluation]"* ]]
then
	find $OUTPUTDIR/entries/ -type d -name target | sed 's/target$//' > $OUTPUTDIR/list_of_target_directories
	$BINDIR/schedule_jobs.bash $BINDIR $SCHEDULER $CPUCOUNT "$BINDIR/calc_local_scores_evaluation_from_target_models_local_scores.bash -d" $OUTPUTDIR/list_of_target_directories
	$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/calc_local_scores_evaluation_from_target_models_local_scores.bash -d $OUTPUTDIR/entries"
	exit 0
fi

if [[ $STEPNAMES == *"[concatenated_local_scores_evaluations]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name local_scores_evaluation -not -empty > $OUTPUTDIR/list_of_local_scores_evaluations
	$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$BINDIR/concatenate_files_from_list_of_files.bash $OUTPUTDIR/list_of_local_scores_evaluations $OUTPUTDIR/concatenated_local_scores_evaluations"
	exit 0
fi

echo "Invalid step name" 1>&2
exit 1
