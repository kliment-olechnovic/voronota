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
		$BINDIR/schedule_jobs.bash \
		  -b $BINDIR -s $SCHEDULER -p 1 \
		  -c "$BINDIR/get_balls_from_CASP_target_models.bash -c $CASPNAME -t $TARGETNAME -o $OUTPUTDIR/entries" \
		  -l $OUTPUTDIR/scheduling/logs__balls_CASP
	done > $OUTPUTDIR/scheduling/scheduled__balls_CASP
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
		$BINDIR/schedule_jobs.bash \
		  -b $BINDIR -s $SCHEDULER -p 1 \
		  -c "$BINDIR/get_balls_from_decoys99_set.bash -i $INPUTFILE -o $OUTPUTDIR/entries" \
		  -l $OUTPUTDIR/scheduling/logs__balls_decoys99
	done > $OUTPUTDIR/scheduling/scheduled__balls_decoys99
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
		$BINDIR/schedule_jobs.bash \
		  -b $BINDIR -s $SCHEDULER -p 1 \
		  -c "$BINDIR/get_balls_from_RosettaDecoys_set.bash -i $INPUTFILE -o $OUTPUTDIR/entries" \
		  -l $OUTPUTDIR/scheduling/logs__balls_RosettaDecoys
	done > $OUTPUTDIR/scheduling/scheduled__balls_RosettaDecoys
fi

cat $OUTPUTDIR/scheduling/scheduled__balls_* > $OUTPUTDIR/scheduling/scheduled__all_balls

if [[ $STEPNAMES == *"[raw_contacts]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name balls -not -empty | sed 's/balls$//' > $OUTPUTDIR/list_of_entries_with_balls
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__all_balls \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_raw_contacts_from_balls.bash -c -d" \
	  -a $OUTPUTDIR/list_of_entries_with_balls \
	  -l $OUTPUTDIR/scheduling/logs__raw_contacts \
	> $OUTPUTDIR/scheduling/scheduled__raw_contacts
fi

if [[ $STEPNAMES == *"[contacts_and_summary]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name raw_contacts -not -empty | sed 's/raw_contacts$//' > $OUTPUTDIR/list_of_entries_with_raw_contacts
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__raw_contacts \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_raw_contacts \
	  -l $OUTPUTDIR/scheduling/logs__contacts_and_summary \
	> $OUTPUTDIR/scheduling/scheduled__contacts_and_summary
fi

if [[ $STEPNAMES == *"[potential]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name summary -not -empty > $OUTPUTDIR/list_of_summaries
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__contacts_and_summary \
	  -b $BINDIR -s $SCHEDULER -p 1 \
	  -c "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential -i $OUTPUTDIR/list_of_summaries" \
	  -l $OUTPUTDIR/scheduling/logs__potential \
	> $OUTPUTDIR/scheduling/scheduled__potential
fi

if [[ $STEPNAMES == *"[energies]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_contacts
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__contacts_and_summary \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $BINDIR/potential -d" \
	  -a $OUTPUTDIR/list_of_entries_with_contacts \
	  -l $OUTPUTDIR/scheduling/logs__energies \
	> $OUTPUTDIR/scheduling/scheduled__energies
fi

if [[ $STEPNAMES == *"[quality_scores]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty | sed 's/atom_energies$//' > $OUTPUTDIR/list_of_entries_with_atom_energies
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__energies \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_quality_scores_from_energies.bash -e $BINDIR/energy_means_and_sds -d" \
	  -a $OUTPUTDIR/list_of_entries_with_atom_energies \
	  -l $OUTPUTDIR/scheduling/logs__quality_scores \
	> $OUTPUTDIR/scheduling/scheduled__quality_scores
fi

if [[ $STEPNAMES == *"[atoms]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name balls -not -empty | sed 's/balls$//' > $OUTPUTDIR/list_of_entries_with_balls
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__all_balls \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/collect_atoms_from_balls.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_balls \
	  -l $OUTPUTDIR/scheduling/logs__atoms \
	> $OUTPUTDIR/scheduling/scheduled__atoms
fi

if [[ $STEPNAMES == *"[goap_scores]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_atoms
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__atoms \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_goap_scores_from_atoms.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_atoms \
	  -l $OUTPUTDIR/scheduling/logs__goap_scores \
	> $OUTPUTDIR/scheduling/scheduled__goap_scores
fi

if [[ $STEPNAMES == *"[rwplus_score]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_atoms
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__atoms \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_rwplus_score_from_atoms.bash" \
	  -a $OUTPUTDIR/list_of_entries_with_atoms \
	  -l $OUTPUTDIR/scheduling/logs__rwplus_score \
	> $OUTPUTDIR/scheduling/scheduled__rwplus_score
fi

if [[ $STEPNAMES == *"[doop_score]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_atoms
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__atoms \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_doop_score_from_atoms.bash" \
	  -a $OUTPUTDIR/list_of_entries_with_atoms \
	  -l $OUTPUTDIR/scheduling/logs__doop_score \
	> $OUTPUTDIR/scheduling/scheduled__doop_score
fi

if [[ $STEPNAMES == *"[cad_scores]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | grep -v '/target/contacts$' | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_models_contacts
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__contacts_and_summary \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_cad_scores_from_model_and_target_contacts.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_models_contacts \
	  -l $OUTPUTDIR/scheduling/logs__cad_scores \
	> $OUTPUTDIR/scheduling/scheduled__cad_scores
fi

if [[ $STEPNAMES == *"[tmscore]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | grep -v '/target/atoms.pdb$' | sed 's/atoms.pdb$//' > $OUTPUTDIR/list_of_entries_with_models_atoms
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__atoms \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_tmscore_from_model_and_target_atoms.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_models_atoms \
	  -l $OUTPUTDIR/scheduling/logs__tmscore \
	> $OUTPUTDIR/scheduling/scheduled__tmscore
fi

cat $OUTPUTDIR/scheduling/scheduled__*score* > $OUTPUTDIR/scheduling/scheduled__all_scores

if [[ $STEPNAMES == *"[scores_list]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name global_quality_score -not -empty | sed 's/global_quality_score$//' > $OUTPUTDIR/list_of_entries_with_global_quality_score
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__all_scores \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/collect_scores_list_from_working_directory.bash -d" \
	  -a $OUTPUTDIR/list_of_entries_with_global_quality_score \
	  -l $OUTPUTDIR/scheduling/logs__scores_list \
	> $OUTPUTDIR/scheduling/scheduled__scores_list
fi

if [[ $STEPNAMES == *"[concatenated_scores_lists]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name scores_list -not -empty > $OUTPUTDIR/list_of_scores_lists
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__scores_list \
	  -b $BINDIR -s $SCHEDULER -p 1 \
	  -c "$BINDIR/concatenate_files_from_list_of_files.bash $OUTPUTDIR/list_of_scores_lists $OUTPUTDIR/concatenated_scores_lists" \
	  -l $OUTPUTDIR/scheduling/logs__concatenated_scores_lists \
	> $OUTPUTDIR/scheduling/scheduled__concatenated_scores_lists
fi

if [[ $STEPNAMES == *"[local_scores_evaluation]"* ]]
then
	find $OUTPUTDIR/entries/ -type d -name target | sed 's/target$//' > $OUTPUTDIR/list_of_target_directories
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__quality_scores \
	  -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -c "$BINDIR/calc_local_scores_evaluation_from_target_models_local_scores.bash -d" \
	  -a $OUTPUTDIR/list_of_target_directories \
	  -l $OUTPUTDIR/scheduling/logs__local_scores_evaluation \
	> $OUTPUTDIR/scheduling/scheduled__local_scores_evaluation
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__quality_scores \
	  -b $BINDIR -s $SCHEDULER -p 1 \
	  -c "$BINDIR/calc_local_scores_evaluation_from_target_models_local_scores.bash -d $OUTPUTDIR/entries" \
	  -l $OUTPUTDIR/scheduling/logs__local_scores_evaluation \
	>> $OUTPUTDIR/scheduling/scheduled__local_scores_evaluation
fi

if [[ $STEPNAMES == *"[concatenated_local_scores_evaluations]"* ]]
then
	find $OUTPUTDIR/entries/ -type f -name local_scores_evaluation -not -empty > $OUTPUTDIR/list_of_local_scores_evaluations
	$BINDIR/schedule_jobs.bash \
	  -d $OUTPUTDIR/scheduling/scheduled__local_scores_evaluation \
	  -b $BINDIR -s $SCHEDULER -p 1 \
	  -c "$BINDIR/concatenate_files_from_list_of_files.bash $OUTPUTDIR/list_of_local_scores_evaluations $OUTPUTDIR/concatenated_local_scores_evaluations" \
	  -l $OUTPUTDIR/scheduling/logs__concatenated_local_scores_evaluations \
	> $OUTPUTDIR/scheduling/scheduled__concatenated_local_scores_evaluations
fi
