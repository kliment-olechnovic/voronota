#!/bin/bash

set +e

BINDIR=""
INPUT_LIST_FILE=""
OUTPUTDIR=""
CPUCOUNT=""
STEPNAMES=""
SCHEDULER=""
PARTIAL_POTENTIALS=""
TEACHING=false;

while getopts "b:i:o:p:s:c:r:t" OPTION
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
	r)
		PARTIAL_POTENTIALS=$OPTARG
		;;
	t)
		TEACHING=true
		;;
	esac
done

function exit_with_error_message
{
	echo $1 1>&2
	exit 1
}

if [ -z "$BINDIR" ]
then
	exit_with_error_message "Missing executables directory parameter."
fi

if [ -z "$OUTPUTDIR" ]
then
	exit_with_error_message "Missing output directory parameter."
fi

if [ -z "$SCHEDULER" ]
then
	exit_with_error_message "Missing scheduler parameter."
fi

if [ -z "$CPUCOUNT" ]
then
	exit_with_error_message "Missing CPU count parameter."
fi

if [ -z "$STEPNAMES" ]
then
	exit_with_error_message "Missing step names parameter."
fi

if [ -z "$INPUT_LIST_FILE" ] && [[ $STEPNAMES == *"[balls_"* ]]
then
	exit_with_error_message "Missing input list file parameter."
fi

if [ -z "$PARTIAL_POTENTIALS" ] && [[ $STEPNAMES == *"[partial_potentials]"* ]]
then
	exit_with_error_message "Missing partial potentials parameter."
fi

mkdir -p $OUTPUTDIR/scheduling

if [[ $STEPNAMES == *"[balls_"* ]] || [[ $STEPNAMES == *"[list_of_balls]"* ]]
then
	if [[ $STEPNAMES == *"[balls_PDB]"* ]]
	then
		$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
		  -c "$BINDIR/get_balls_from_atoms_link.bash -u -z -m -o $OUTPUTDIR/entries -i" \
		  -a $INPUT_LIST_FILE \
		  -l $OUTPUTDIR/scheduling/logs__balls_PDB \
		> $OUTPUTDIR/scheduling/scheduled__balls_PDB
	elif [[ $STEPNAMES == *"[balls_CASP]"* ]]
	then
		cat $INPUT_LIST_FILE | while read CASPNAME TARGETNAME
		do
			$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p 1 \
			  -c "$BINDIR/get_balls_from_CASP_target_models.bash -c $CASPNAME -t $TARGETNAME -o $OUTPUTDIR/entries" \
			  -l $OUTPUTDIR/scheduling/logs__balls_CASP
		done > $OUTPUTDIR/scheduling/scheduled__balls_CASP
	elif [[ $STEPNAMES == *"[balls_RosettaDecoys]"* ]]
	then
		cat $INPUT_LIST_FILE | while read INPUTFILE
		do
			$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p 1 \
			  -c "$BINDIR/get_balls_from_RosettaDecoys_set.bash -i $INPUTFILE -o $OUTPUTDIR/entries" \
			  -l $OUTPUTDIR/scheduling/logs__balls_RosettaDecoys
		done > $OUTPUTDIR/scheduling/scheduled__balls_RosettaDecoys
	elif [[ $STEPNAMES == *"[balls_decoys99]"* ]]
	then
		cat $INPUT_LIST_FILE | while read INPUTFILE
		do
			$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p 1 \
			  -c "$BINDIR/get_balls_from_decoys99_set.bash -i $INPUTFILE -o $OUTPUTDIR/entries" \
			  -l $OUTPUTDIR/scheduling/logs__balls_decoys99
		done > $OUTPUTDIR/scheduling/scheduled__balls_decoys99
	fi
	if [[ $STEPNAMES == *"[list_of_balls]"* ]]
	then
		for STEP in balls_CASP balls_decoys99 balls_RosettaDecoys
		do
			if [ -s "$OUTPUTDIR/scheduling/scheduled__$STEP" ]
			then
				cat $OUTPUTDIR/scheduling/scheduled__$STEP
			fi
		done > $OUTPUTDIR/scheduling/scheduled__all_balls
		$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p 1 \
		  -d $OUTPUTDIR/scheduling/scheduled__all_balls \
		  -w -c "find $OUTPUTDIR/entries/ -type f -name balls -not -empty > $OUTPUTDIR/list_of_balls" \
		  -l $OUTPUTDIR/scheduling/logs__list_of_balls \
		> $OUTPUTDIR/scheduling/scheduled__list_of_balls
	fi
	exit
fi

if [ ! -s "$OUTPUTDIR/list_of_balls" ]
then
	exit_with_error_message "No balls to process."
fi

function submit_step
{
	DEPENDENCY=$1
	STEP=$2
	STEP_COMMAND=$3
	INPUT_LIST_FILE=$4
	INPUT_OPTION=""
	if [ -n "$INPUT_LIST_FILE" ]
	then
		INPUT_OPTION="-a $INPUT_LIST_FILE"
	fi
	$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -d $OUTPUTDIR/scheduling/scheduled__$DEPENDENCY \
	  -c "$STEP_COMMAND" $INPUT_OPTION \
	  -l $OUTPUTDIR/scheduling/logs__$STEP \
	> $OUTPUTDIR/scheduling/scheduled__$STEP
}

cat $OUTPUTDIR/list_of_balls | sed 's|/balls$||' > $OUTPUTDIR/scheduling/input_list_for__entries_operations

if [[ $STEPNAMES == *"[sequences_pairings_stats]"* ]]
then
	submit_step list_of_balls sequences_pairings_stats \
	  "$BINDIR/calc_sequences_pairings_stats_from_balls.bash -o $OUTPUTDIR/sequences_pairings_stats -i $OUTPUTDIR/list_of_balls"
fi

if [[ $STEPNAMES == *"[raw_contacts]"* ]]
then
	submit_step none raw_contacts \
	  "$BINDIR/calc_raw_contacts_from_balls.bash -c -d" $OUTPUTDIR/scheduling/input_list_for__entries_operations
fi

if [[ $STEPNAMES == *"[contacts_and_summary]"* ]]
then
	if $TEACHING
	then
		submit_step raw_contacts contacts_and_summary \
		  "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -m -d" $OUTPUTDIR/scheduling/input_list_for__entries_operations
	else
		submit_step raw_contacts contacts_and_summary \
		  "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -d" $OUTPUTDIR/scheduling/input_list_for__entries_operations
	fi
fi

if [[ $STEPNAMES == *"[potential]"* ]]
then
	cat $OUTPUTDIR/list_of_balls | sed 's|/balls$|/summary|' > $OUTPUTDIR/scheduling/input_list_for__potential
	if $TEACHING
	then
		submit_step contacts_and_summary potential \
		  "$BINDIR/calc_potential_from_summaries.bash -c $BINDIR/contributions_from_casp_models -o $OUTPUTDIR/potential -i $OUTPUTDIR/scheduling/input_list_for__potential"
	else
		submit_step contacts_and_summary potential \
		  "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential -i $OUTPUTDIR/scheduling/input_list_for__potential"
	fi
fi

if [[ $STEPNAMES == *"[partial_potentials]"* ]]
then
	cat $OUTPUTDIR/list_of_balls | sed 's|/balls$|/summary|' > $OUTPUTDIR/scheduling/input_list_for__potential
	yes $(echo "$(cat $OUTPUTDIR/scheduling/input_list_for__potential | wc -l)/2" | bc) | head -n $PARTIAL_POTENTIALS > $OUTPUTDIR/scheduling/input_list_for__partial_potentials
	if $TEACHING
	then
		submit_step contacts_and_summary partial_potentials \
		  "$BINDIR/calc_potential_from_summaries.bash -c $BINDIR/contributions_from_casp_models -o $OUTPUTDIR/partial_potentials -i $OUTPUTDIR/scheduling/input_list_for__potential -r" $OUTPUTDIR/scheduling/input_list_for__partial_potentials
	else
		submit_step contacts_and_summary partial_potentials \
		  "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/partial_potentials -i $OUTPUTDIR/scheduling/input_list_for__potential -r" $OUTPUTDIR/scheduling/input_list_for__partial_potentials
	fi
fi

if [[ $STEPNAMES == *"[partial_potentials_stats]"* ]]
then
	submit_step partial_potentials partial_potentials_stats \
	  "$BINDIR/calc_potentials_stats_from_potentials.bash -o $OUTPUTDIR/partial_potentials_stats -i $OUTPUTDIR/partial_potentials"
fi

if [[ $STEPNAMES == *"[energies]"* ]]
then
	if $TEACHING
	then
		submit_step potential energies \
		  "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $OUTPUTDIR/potential/potential -d" $OUTPUTDIR/scheduling/input_list_for__entries_operations
	else
		submit_step contacts_and_summary energies \
		  "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $BINDIR/potential -d" $OUTPUTDIR/scheduling/input_list_for__entries_operations
	fi
fi

if [[ $STEPNAMES == *"[energies_stats]"* ]]
then
	cat $OUTPUTDIR/list_of_balls | sed 's|/balls$|/atom_energies|' > $OUTPUTDIR/scheduling/input_list_for__energies_stats
	submit_step energies energies_stats \
	  "$BINDIR/calc_energies_stats_from_energies.bash -o $OUTPUTDIR/energies_stats -i $OUTPUTDIR/scheduling/input_list_for__energies_stats"
fi

if [[ $STEPNAMES == *"[quality_scores]"* ]]
then
	if $TEACHING
	then
		submit_step energies_stats quality_scores \
		  "$BINDIR/calc_quality_scores_from_energies.bash -e $OUTPUTDIR/energies_stats/energy_means_and_sds -d" $OUTPUTDIR/scheduling/input_list_for__entries_operations
	else
		submit_step energies quality_scores \
		  "$BINDIR/calc_quality_scores_from_energies.bash -e $BINDIR/energy_means_and_sds -d" $OUTPUTDIR/scheduling/input_list_for__entries_operations
	fi
fi

if [[ $STEPNAMES == *"[atoms]"* ]]
then
	submit_step list_of_balls atoms \
	  "$BINDIR/collect_atoms_from_balls.bash -d" $OUTPUTDIR/scheduling/input_list_for__entries_operations
fi

if [[ $STEPNAMES == *"[goap_scores]"* ]]
then
	submit_step atoms goap_scores \
	  "$BINDIR/calc_goap_scores_from_atoms.bash -d" $OUTPUTDIR/scheduling/input_list_for__entries_operations
fi

if [[ $STEPNAMES == *"[rwplus_score]"* ]]
then
	submit_step atoms rwplus_score \
	  "$BINDIR/calc_rwplus_score_from_atoms.bash" $OUTPUTDIR/scheduling/input_list_for__entries_operations
fi

if [[ $STEPNAMES == *"[doop_score]"* ]]
then
	submit_step atoms doop_score \
	  "$BINDIR/calc_doop_score_from_atoms.bash" $OUTPUTDIR/scheduling/input_list_for__entries_operations
fi

if [[ $STEPNAMES == *"[cad_scores]"* ]]
then
	cat $OUTPUTDIR/list_of_balls | grep -v '/target/balls$' | sed 's|/balls$||' > $OUTPUTDIR/scheduling/input_list_for__cad_scores
	submit_step contacts_and_summary cad_scores \
	  "$BINDIR/calc_cad_scores_from_model_and_target_contacts.bash -d" $OUTPUTDIR/scheduling/input_list_for__cad_scores
fi

if [[ $STEPNAMES == *"[tmscore]"* ]]
then
	cat $OUTPUTDIR/list_of_balls | grep -v '/target/balls$' | sed 's|/balls$||' > $OUTPUTDIR/scheduling/input_list_for__tmscore
	submit_step atoms tmscore \
	  "$BINDIR/calc_tmscore_from_model_and_target_atoms.bash -d" $OUTPUTDIR/scheduling/input_list_for__tmscore
fi

for STEP in energies quality_scores goap_scores rwplus_score doop_score cad_scores tmscore
do
	if [ -s "$OUTPUTDIR/scheduling/scheduled__$STEP" ]
	then
		cat $OUTPUTDIR/scheduling/scheduled__$STEP
	fi
done > $OUTPUTDIR/scheduling/scheduled__all_scores

if [[ $STEPNAMES == *"[scores_list]"* ]]
then
	submit_step all_scores scores_list \
	  "$BINDIR/collect_scores_list_from_working_directory.bash -d" $OUTPUTDIR/scheduling/input_list_for__entries_operations
fi

if [[ $STEPNAMES == *"[concatenated_scores_lists]"* ]]
then
	cat $OUTPUTDIR/list_of_balls | sed 's|/balls$|/scores_list|' > $OUTPUTDIR/scheduling/input_list_for__concatenated_scores_lists
	submit_step scores_list concatenated_scores_lists \
	  "$BINDIR/concatenate_files_from_list_of_files.bash -s -t -o $OUTPUTDIR/concatenated_scores_lists -i $OUTPUTDIR/scheduling/input_list_for__concatenated_scores_lists"
fi

if [[ $STEPNAMES == *"[local_scores_evaluation]"* ]]
then
	( cat $OUTPUTDIR/list_of_balls | grep '/target/balls$' | sed 's|/target/balls$||' ; echo $OUTPUTDIR/entries ) > $OUTPUTDIR/scheduling/input_list_for__local_scores_evaluation
	submit_step quality_scores local_scores_evaluation \
	  "$BINDIR/calc_local_scores_evaluation_from_target_models_local_scores.bash -d" $OUTPUTDIR/scheduling/input_list_for__local_scores_evaluation
fi

if [[ $STEPNAMES == *"[concatenated_local_scores_evaluations]"* ]]
then
	( cat $OUTPUTDIR/list_of_balls | grep '/target/balls$' | sed 's|/target/balls$|/local_scores_evaluation|' ; echo $OUTPUTDIR/entries/local_scores_evaluation ) > $OUTPUTDIR/scheduling/input_list_for__concatenated_local_scores_evaluations
	submit_step local_scores_evaluation concatenated_local_scores_evaluations \
	  "$BINDIR/concatenate_files_from_list_of_files.bash -h -s -t -o $OUTPUTDIR/concatenated_local_scores_evaluations -i $OUTPUTDIR/scheduling/input_list_for__concatenated_local_scores_evaluations"
fi
