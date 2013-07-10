#!/bin/bash

WORKING_DIR=$1
PDB_FILE=$2

PDB_FILE_BASENAME=$(basename $PDB_FILE .ent.gz)
PDB_FILE_DOMAIN=$(echo $PDB_FILE_BASENAME | sed 's/pdb.\(..\)./\1/')
WORKING_DIR="$WORKING_DIR/$PDB_FILE_DOMAIN"

FIRST_GAUGE_LOG_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.first_gauge.log"
SECOND_GAUGE_LOG_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.second_gauge.log"
TEST_SUBJECT_LOG_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.test_subject.log"
FIRST_COMPARISON_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.comparison.first"
SECOND_COMPARISON_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.comparison.second"

if [ ! -s "$FIRST_GAUGE_LOG_FILE" ] ; then exit 1 ; fi
if [ ! -s "$SECOND_GAUGE_LOG_FILE" ] ; then exit 1 ; fi
if [ ! -s "$TEST_SUBJECT_LOG_FILE" ] ; then exit 1 ; fi
if [ ! -s "$FIRST_COMPARISON_FILE" ] ; then exit 1 ; fi
if [ ! -s "$SECOND_COMPARISON_FILE" ] ; then exit 1 ; fi

SUMMARY_LIST_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.summary.list"
SUMMARY_TABLE_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.summary.table"

true > $SUMMARY_LIST_FILE
cat $FIRST_GAUGE_LOG_FILE | sed 's/^/first_gauge_/' >> $SUMMARY_LIST_FILE
cat $SECOND_GAUGE_LOG_FILE | sed 's/^/second_gauge_/' >> $SUMMARY_LIST_FILE
cat $TEST_SUBJECT_LOG_FILE | sed 's/^/test_subject_/' >> $SUMMARY_LIST_FILE
cat $FIRST_COMPARISON_FILE | sed 's/^/first_/' >> $SUMMARY_LIST_FILE
cat $SECOND_COMPARISON_FILE | sed 's/^/second_/' >> $SUMMARY_LIST_FILE

( cat $SUMMARY_LIST_FILE | cut --delimiter " " --fields 1 | paste -s ; cat $SUMMARY_LIST_FILE | cut --delimiter " " --fields 2 | paste -s ) | column -t > $SUMMARY_TABLE_FILE

cat $SUMMARY_LIST_FILE
