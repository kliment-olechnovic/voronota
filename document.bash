#!/bin/bash

cd $(dirname "$0")

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

./Release/voronota \
| grep 'Commands:' -A 999999 \
| sed 's/^Commands:/## List of all commands/' \
| sed 's/^\([[:alpha:]]\S*\)/* \1/' \
> $TMPDIR/commands_list.markdown

./Release/voronota --help \
| grep 'Command ' -A 999999 \
| sed "s/^Command\s\+'\(\S\+\)'.*/## Command '\1'\n\n### Command line arguments:\n\nCOMMAND_OPTIONS_TABLE_HEADER1\nCOMMAND_OPTIONS_TABLE_HEADER2/" \
| sed 's/COMMAND_OPTIONS_TABLE_HEADER1/Name                            Type        Description/' \
| sed 's/COMMAND_OPTIONS_TABLE_HEADER2/------------------------------- ------ ---- ------------------------------------------------------------------------/' \
| sed 's/^stdin   <-\s*/\n### Input stream:\n\n/' \
| sed 's/^stdout  ->\s*/\n### Output stream:\n\n/' \
| sed 's/^\s\+(\(.\+\))/\n* \1/' \
> $TMPDIR/commands_descriptions.markdown

cat ./README.markdown $TMPDIR/commands_list.markdown $TMPDIR/commands_descriptions.markdown > ./help.markdown

pandoc ./help.markdown -f markdown -t html -s -o ./help.htm
