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

$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/potential -i $OUTPUTDIR/list_of_summaries"

yes 4 | head -10 \
| xargs -L 2 $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_potential_from_summaries.bash -o $OUTPUTDIR/partial_potentials -i $OUTPUTDIR/list_of_summaries -r"

##############################

find $OUTPUTDIR/partial_potentials/ -type f -name potential -not -empty > $OUTPUTDIR/list_of_partial_potentials

$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_potentials_stats_from_potentials.bash -o $OUTPUTDIR/partial_potentials_stats -i $OUTPUTDIR/list_of_partial_potentials"

##############################

find $OUTPUTDIR/entries/ -type f -name contacts -not -empty | sed 's/contacts$//' > $OUTPUTDIR/list_of_entries_with_contacts

cat $OUTPUTDIR/list_of_entries_with_contacts \
| xargs -L 3 $BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_energies_from_contacts_and_potential.bash -p $OUTPUTDIR/potential/potential -d"

##############################

find $OUTPUTDIR/entries/ -type f -name atom_energies -not -empty > $OUTPUTDIR/list_of_atom_energies

$BINDIR/run_calc_script.bash $BINDIR "$BINDIR/calc_energies_stats_from_energies.bash -o $OUTPUTDIR/energies_stats -i $OUTPUTDIR/list_of_atom_energies"
