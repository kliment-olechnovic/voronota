#!/bin/bash

cd $(dirname "$0")

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

{

cat ./README.markdown

./Release/voronota \
| grep 'Commands:' -A 999999 \
| sed 's/^Commands:/## List of all commands/' \
| sed 's/^\([[:alpha:]]\S*\)/* \1/'

./Release/voronota --help \
| grep 'Command ' -A 999999 \
| sed "s/^Command\s\+'\(\S\+\)'.*/## Command '\1'\n\n### Command line arguments:\n\nCOMMAND_OPTIONS_TABLE_HEADER1\nCOMMAND_OPTIONS_TABLE_HEADER2/" \
| sed 's/COMMAND_OPTIONS_TABLE_HEADER1/Name                            Type        Description/' \
| sed 's/COMMAND_OPTIONS_TABLE_HEADER2/------------------------------- ------ ---- ------------------------------------------------------------------------/' \
| sed 's/^stdin   <-\s*/\n### Input stream:\n\n/' \
| sed 's/^stdout  ->\s*/\n### Output stream:\n\n/' \
| sed 's/^\s\+(\(.\+\))/\n* \1/'

echo -e "# Wrapper scripts"

echo -e "\n## CAD-score method wrapper script\n"
cat << EOF
The 'cadscore' script is an implementation of CAD-score (Contact Area Difference score) method using Voronota.
EOF
./scripts/cadscore -h 2>&1 | sed 's/^Script.*/The script command line arguments are:\n/'

echo -e "\n## VoroMQA method wrapper script\n"
cat << EOF
The 'voromqa' script is an implementation of VoroMQA (Voronoi diagram-based Model Quality Assessment) method using Voronota.
EOF
./scripts/voromqa -h 2>&1 | sed 's/^Script.*/The script command line arguments are:\n/'

} > $TMPDIR/documentation.markdown

cat > $TMPDIR/include_in_header.html << EOF
<style type="text/css">
td { padding-right: 1em; }
div#TOC > ul > li > ul > li ul { display: none; }
</style>
EOF

echo "<h1>$(./Release/voronota | head -1)</h1>" > $TMPDIR/include_before_body.html

pandoc $TMPDIR/documentation.markdown -f markdown -t html --toc -H $TMPDIR/include_in_header.html -B $TMPDIR/include_before_body.html -s -o ./README.html
