#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/help_printing
mkdir -p $SUBDIR

$VORONOTA &> $SUBDIR/basic_help

$VORONOTA --help > $SUBDIR/full_help
