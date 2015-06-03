#!/bin/bash

set +e

BINDIR=""
INPUT_LIST_FILE=""
OUTPUTDIR=""
CPUCOUNT=""
STEPNAMES=""
SCHEDULER=""

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

if [ -z "$BINDIR" ]
then
	echo "Missing executables directory parameter." 1>&2
	exit 1
fi

if [ -z "$OUTPUTDIR" ]
then
	echo "Missing output directory parameter." 1>&2
	exit 1
fi

if [ -z "$SCHEDULER" ]
then
	echo "Missing scheduler parameter." 1>&2
	exit 1
fi

if [ -z "$CPUCOUNT" ]
then
	echo "Missing CPU count parameter." 1>&2
	exit 1
fi

if [ -z "$STEPNAMES" ]
then
	echo "Missing step names parameter." 1>&2
	exit 1
fi

mkdir -p $OUTPUTDIR/scheduling

if [[ $STEPNAMES == *"[balls_CASP]"* ]]
then
	if [ -z "$INPUT_LIST_FILE" ]
	then
		echo "Missing input list file parameter." 1>&2
		exit 1
	fi
	cat $INPUT_LIST_FILE | while read CASPNAME TARGETNAME
	do
		$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p 1 -c "$BINDIR/get_balls_from_CASP_target_models.bash -c $CASPNAME -t $TARGETNAME -o $OUTPUTDIR/entries"
	done > $OUTPUTDIR/scheduling/scheduled_balls_CASP
fi

if [[ $STEPNAMES == *"[balls_decoys99]"* ]]
then
	if [ -z "$INPUT_LIST_FILE" ]
	then
		echo "Missing input list file parameter." 1>&2
		exit 1
	fi
	cat $INPUT_LIST_FILE | while read INPUTFILE
	do
		$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p 1 -c "$BINDIR/get_balls_from_decoys99_set.bash -i $INPUTFILE -o $OUTPUTDIR/entries"
	done > $OUTPUTDIR/scheduling/scheduled_balls_decoys99
fi

if [[ $STEPNAMES == *"[balls_RosettaDecoys]"* ]]
then
	if [ -z "$INPUT_LIST_FILE" ]
	then
		echo "Missing input list file parameter." 1>&2
		exit 1
	fi
	cat $INPUT_LIST_FILE | while read INPUTFILE
	do
		$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p 1 -c "$BINDIR/get_balls_from_RosettaDecoys_set.bash -i $INPUTFILE -o $OUTPUTDIR/entries"
	done > $OUTPUTDIR/scheduling/scheduled_balls_RosettaDecoys
fi

cat $OUTPUTDIR/scheduling/scheduled_balls_* > $OUTPUTDIR/scheduling/scheduled_all_balls

if [[ $STEPNAMES == *"[raw_contacts]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name balls -not -empty | sed 's/balls$//' > $OUTPUTDIR/list_of_entries_with_balls
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_all_balls \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_raw_contacts_from_balls.bash -c -d" \
	  -a $OUTPUTDIR/list_of_entries_with_balls \
	> $OUTPUTDIR/scheduling/scheduled_raw_contacts
fi

if [[ $STEPNAMES == *"[contacts_and_summary]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name raw_contacts -not -empty | sed 's/raw_contacts$//' > $OUTPUTDIR/list_of_entries_with_raw_contacts
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_raw_contacts \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_raw_contacts \
	> $OUTPUTDIR/scheduling/scheduled_contacts_and_summary
fi

if [[ $STEPNAMES == *"[potential]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name summary -not -empty > $OUTPUTDIR/list_of_summaries
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_contacts_and_summary \
	  -b $BINDIR -s $SCHEDULER -p 1 \
	  -c "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential -i $OUTPUTDIR/list_of_summaries" \
	> $OUTPUTDIR/scheduling/scheduled_potential
fi

if [[ $STEPNAMES == *"[energies]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_contacts
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_contacts_and_summary \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $BINDIR/potential -d" \
	  -a $OUTPUTDIR/list_of_entries_with_contacts \
	> $OUTPUTDIR/scheduling/scheduled_energies
fi

if [[ $STEPNAMES == *"[quality_scores]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty | sed 's/atom_energies$//' > $OUTPUTDIR/list_of_entries_with_atom_energies
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_energies \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_quality_scores_from_energies.bash -e $BINDIR/energy_means_and_sds -d" \
	  -a $OUTPUTDIR/list_of_entries_with_atom_energies \
	> $OUTPUTDIR/scheduling/scheduled_quality_scores
fi

if [[ $STEPNAMES == *"[atoms]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name balls -not -empty | sed 's/balls$//' > $OUTPUTDIR/list_of_entries_with_balls
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_all_balls \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/collect_atoms_from_balls.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_balls \
	> $OUTPUTDIR/scheduling/scheduled_atoms
fi

if [[ $STEPNAMES == *"[goap_scores]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_atoms
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_atoms \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_goap_scores_from_atoms.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_atoms \
	> $OUTPUTDIR/scheduling/scheduled_goap_scores
fi

if [[ $STEPNAMES == *"[rwplus_score]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_atoms
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_atoms \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_rwplus_score_from_atoms.bash" \
	  -a $OUTPUTDIR/list_of_entries_with_atoms \
	> $OUTPUTDIR/scheduling/scheduled_rwplus_score
fi

if [[ $STEPNAMES == *"[doop_score]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_atoms
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_atoms \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_doop_score_from_atoms.bash" \
	  -a $OUTPUTDIR/list_of_entries_with_atoms \
	> $OUTPUTDIR/scheduling/scheduled_doop_score
fi

if [[ $STEPNAMES == *"[cad_scores]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | grep -v '/target/contacts$' | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_models_contacts
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_contacts_and_summary \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_cad_scores_from_model_and_target_contacts.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_models_contacts \
	> $OUTPUTDIR/scheduling/scheduled_cad_scores
fi

if [[ $STEPNAMES == *"[tmscore]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | grep -v '/target/atoms.pdb$' | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_models_atoms
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_atoms \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_tmscore_from_model_and_target_atoms.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_models_atoms \
	> $OUTPUTDIR/scheduling/scheduled_tmscore
fi

cat $OUTPUTDIR/scheduling/*score* > $OUTPUTDIR/scheduling/scheduled_all_scores

if [[ $STEPNAMES == *"[scores_list]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name global_quality_score -not -empty | sed 's/global_quality_score$//' > $OUTPUTDIR/list_of_entries_with_global_quality_score
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_all_scores \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/collect_scores_list_from_working_directory.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_global_quality_score \
	> $OUTPUTDIR/scheduling/scheduled_scores_list
fi

if [[ $STEPNAMES == *"[concatenated_scores_lists]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name scores_list -not -empty > $OUTPUTDIR/list_of_scores_lists
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_scores_list \
	  -b $BINDIR -s $SCHEDULER -p 1 \
	  -c "$BINDIR/concatenate_files_from_list_of_files.bash $OUTPUTDIR/list_of_scores_lists $OUTPUTDIR/concatenated_scores_lists" \
	> $OUTPUTDIR/scheduling/scheduled_concatenated_scores_lists
fi

if [[ $STEPNAMES == *"[local_scores_evaluation]"* ]]
then
	find $OUTPUTDIR/entries/ -type d -name target | sed 's/target$//' > $OUTPUTDIR/list_of_target_directories
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_quality_scores \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_local_scores_evaluation_from_target_models_local_scores.bash -d" \
	  -a $OUTPUTDIR/list_of_target_directories \
	> $OUTPUTDIR/scheduling/scheduled_local_scores_evaluation
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_quality_scores \
	  -b $BINDIR -s $SCHEDULER -p 1 \
	  -c "$BINDIR/calc_local_scores_evaluation_from_target_models_local_scores.bash -d $OUTPUTDIR/entries" \
	>> $OUTPUTDIR/scheduling/scheduled_local_scores_evaluation
fi

if [[ $STEPNAMES == *"[concatenated_local_scores_evaluations]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name local_scores_evaluation -not -empty > $OUTPUTDIR/list_of_local_scores_evaluations
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled_local_scores_evaluation \
	  -b $BINDIR -s $SCHEDULER -p 1 \
	  -c "$BINDIR/concatenate_files_from_list_of_files.bash $OUTPUTDIR/list_of_local_scores_evaluations $OUTPUTDIR/concatenated_local_scores_evaluations" \
	> $OUTPUTDIR/scheduling/scheduled_concatenated_local_scores_evaluations
fi
