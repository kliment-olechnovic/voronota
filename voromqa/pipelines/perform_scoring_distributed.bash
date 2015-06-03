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
		$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p 1 \
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
		$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p 1 \
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
		$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p 1 \
		  -c "$BINDIR/get_balls_from_RosettaDecoys_set.bash -i $INPUTFILE -o $OUTPUTDIR/entries" \
		  -l $OUTPUTDIR/scheduling/logs__balls_RosettaDecoys
	done > $OUTPUTDIR/scheduling/scheduled__balls_RosettaDecoys
fi

for STEP in balls_CASP balls_decoys99 balls_RosettaDecoys
do
	if [ -s "$OUTPUTDIR/scheduling/scheduled__$STEP" ]
	then
		cat $OUTPUTDIR/scheduling/scheduled__$STEP
	fi
done > $OUTPUTDIR/scheduling/scheduled__all_balls

function submit_step
{
	DEPENDENCY=$1
	STEP=$2
	INPUT_PREPARATION_COMMAND=$3
	STEP_COMMAND=$4
	INLINE_INPUT=$5
	$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
	  -d $OUTPUTDIR/scheduling/scheduled__$DEPENDENCY \
	  -w -c "$INPUT_PREPARATION_COMMAND > $OUTPUTDIR/scheduling/preparation__$STEP" \
	  -l $OUTPUTDIR/scheduling/logs__preparation__$STEP \
	> $OUTPUTDIR/scheduling/scheduled__preparation__$STEP
	if [ "$INLINE_INPUT" == "yes" ]
	then
		$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
		  -d $OUTPUTDIR/scheduling/scheduled__preparation__$STEP \
		  -c "$STEP_COMMAND $OUTPUTDIR/scheduling/preparation__$STEP" \
		  -l $OUTPUTDIR/scheduling/logs__$STEP \
		> $OUTPUTDIR/scheduling/scheduled__$STEP
	else
		$BINDIR/schedule_jobs.bash -b $BINDIR -s $SCHEDULER -p $CPUCOUNT \
		  -d $OUTPUTDIR/scheduling/scheduled__preparation__$STEP \
		  -c "$STEP_COMMAND" \
		  -a $OUTPUTDIR/scheduling/preparation__$STEP \
		  -l $OUTPUTDIR/scheduling/logs__$STEP \
		> $OUTPUTDIR/scheduling/scheduled__$STEP
	fi
}

if [[ $STEPNAMES == *"[raw_contacts]"* ]]
then
	submit_step all_balls raw_contacts \
	  "find $OUTPUTDIR/entries/ -type f -name balls -not -empty | sed 's/balls$//'" \
	  "$BINDIR/calc_raw_contacts_from_balls.bash -c -d" no
fi

if [[ $STEPNAMES == *"[contacts_and_summary]"* ]]
then
	submit_step raw_contacts contacts_and_summary \
	  "find $OUTPUTDIR/entries/ -type f -name raw_contacts -not -empty | sed 's/raw_contacts$//'" \
	  "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -d" no
fi

if [[ $STEPNAMES == *"[potential]"* ]]
then
	submit_step contacts_and_summary potential \
	  "find $OUTPUTDIR/entries/ -type f -name summary -not -empty" \
	  "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential -i" yes
fi

if [[ $STEPNAMES == *"[energies]"* ]]
then
	submit_step contacts_and_summary energies \
	  "find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | sed 's/contacts$//'" \
	  "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $BINDIR/potential -d" no
fi

if [[ $STEPNAMES == *"[quality_scores]"* ]]
then
	submit_step energies quality_scores \
	  "find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty | sed 's/atom_energies$//'" \
	  "$BINDIR/calc_quality_scores_from_energies.bash -e $BINDIR/energy_means_and_sds -d" no
fi

if [[ $STEPNAMES == *"[atoms]"* ]]
then
	submit_step all_balls atoms \
	 "find $OUTPUTDIR/entries/ -type f -name balls -not -empty | sed 's/balls$//'" \
	 "$BINDIR/collect_atoms_from_balls.bash -d" no
fi

if [[ $STEPNAMES == *"[goap_scores]"* ]]
then
	submit_step atoms goap_scores \
	  "find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//'" \
	  "$BINDIR/calc_goap_scores_from_atoms.bash -d" no
fi

if [[ $STEPNAMES == *"[rwplus_score]"* ]]
then
	submit_step atoms rwplus_score \
	  "find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//'" \
	  "$BINDIR/calc_rwplus_score_from_atoms.bash" no
fi

if [[ $STEPNAMES == *"[doop_score]"* ]]
then
	submit_step atoms doop_score \
	  "find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | sed 's/atoms.pdb$//'" \
	  "$BINDIR/calc_doop_score_from_atoms.bash" no
fi

if [[ $STEPNAMES == *"[cad_scores]"* ]]
then
	submit_step contacts_and_summary cad_scores \
	  "find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | grep -v '/target/contacts$' | sed 's/contacts$//'" \
	  "$BINDIR/calc_cad_scores_from_model_and_target_contacts.bash -d" no
fi

if [[ $STEPNAMES == *"[tmscore]"* ]]
then
	submit_step atoms tmscore \
	  "find $OUTPUTDIR/entries/ -type f -name atoms.pdb -not -empty | grep -v '/target/atoms.pdb$' | sed 's/atoms.pdb$//'" \
	  "$BINDIR/calc_tmscore_from_model_and_target_atoms.bash -d" no
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
	  "find $OUTPUTDIR/entries/ -type f -name global_quality_score -not -empty | sed 's/global_quality_score$//'" \
	  "$BINDIR/collect_scores_list_from_working_directory.bash -d" no
fi

if [[ $STEPNAMES == *"[concatenated_scores_lists]"* ]]
then
	submit_step scores_list concatenated_scores_lists \
	  "find $OUTPUTDIR/entries/ -type f -name scores_list -not -empty" \
	  "$BINDIR/concatenate_files_from_list_of_files.bash -s -t -o $OUTPUTDIR/concatenated_scores_lists -i" yes
fi

if [[ $STEPNAMES == *"[local_scores_evaluation]"* ]]
then
	submit_step quality_scores local_scores_evaluation \
	  "( find $OUTPUTDIR/entries/ -type d -name target | sed 's/target$//' ; echo $OUTPUTDIR/entries )" \
	  "$BINDIR/calc_local_scores_evaluation_from_target_models_local_scores.bash -d" no
fi

if [[ $STEPNAMES == *"[concatenated_local_scores_evaluations]"* ]]
then
	submit_step local_scores_evaluation concatenated_local_scores_evaluations \
	  "find $OUTPUTDIR/entries/ -type f -name local_scores_evaluation -not -empty" \
	  "$BINDIR/concatenate_files_from_list_of_files.bash -h target -s -t -o $OUTPUTDIR/concatenated_local_scores_evaluations -i" yes
fi
