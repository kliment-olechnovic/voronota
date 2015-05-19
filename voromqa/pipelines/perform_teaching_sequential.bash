#!/bin/bash

set +e

BINDIR=$1
URLS_LIST_FILE=$2
OUTPUTDIR=$3

##############################

mkdir -p $OUTPUTDIR

cat $URLS_LIST_FILE | xargs -L 3 $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_raw_contacts_from_atoms.bash -u -z -m -o $OUTPUTDIR/entries -i"

##############################

find $OUTPUTDIR/entries/ -type f -name raw_contacts -not -empty | sed 's/raw_contacts$//' > $OUTPUTDIR/list_of_entries_with_raw_contacts

cat $OUTPUTDIR/list_of_entries_with_raw_contacts \
| xargs -L 3 -P 1 $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_contacts_and_summary_from_raw_contacts.bash -d"

##############################

find $OUTPUTDIR/entries/ -type f -name summary -not -empty > $OUTPUTDIR/list_of_summaries

$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential -i" $OUTPUTDIR/list_of_summaries

##############################

find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_contacts

cat $OUTPUTDIR/list_of_entries_with_contacts \
| xargs -L 3 $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $OUTPUTDIR/potential/full/potential -d"

##############################

find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty > $OUTPUTDIR/list_of_atom_energies

$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_energies_stats_from_energies.bash -o $OUTPUTDIR/energies_stats -i" $OUTPUTDIR/list_of_atom_energies
