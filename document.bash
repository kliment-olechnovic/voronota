#!/bin/bash

cd $(dirname "$0")

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

{

cat ./README.markdown

echo -e "# Command reference"

./voronota \
| grep 'Commands:' -A 999999 \
| sed 's/^Commands:/## List of all commands/' \
| sed 's/^\([[:alpha:]]\S*\)/* \1/'

./voronota --help \
| grep 'Command ' -A 999999 \
| sed "s/^Command\s\+'\(\S\+\)'.*/## Command '\1'\n\n### Command line arguments:\n\nCOMMAND_OPTIONS_TABLE_HEADER1\nCOMMAND_OPTIONS_TABLE_HEADER2/" \
| sed 's/COMMAND_OPTIONS_TABLE_HEADER1/Name                            Type        Description/' \
| sed 's/COMMAND_OPTIONS_TABLE_HEADER2/------------------------------- ------ ---- ------------------------------------------------------------------------/' \
| sed 's/^stdin   <-\s*/\n### Input stream:\n\n/' \
| sed 's/^stdout  ->\s*/\n### Output stream:\n\n/' \
| sed 's/^\s\+(\(.\+\))/\n* \1/'

echo -e "# Wrapper scripts"

echo -e "\n## VoroMQA method script\n"
cat << EOF
'voronota-voromqa' script is an implementation of VoroMQA (Voronoi diagram-based Model Quality Assessment) method using Voronota.
The script interface is presented below:
EOF
./voronota-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'

echo -e "\n## CAD-score method script\n"
cat << EOF
'voronota-cadscore' script is an implementation of CAD-score (Contact Area Difference score) method using Voronota.
The script interface is presented below:
EOF
./voronota-cadscore -h 2>&1 | tail -n +3 | sed 's/^/    /'

echo -e "\n## Contacts calculation convenience script\n"
cat << EOF
The 'voronota-contacts' script provides a way for calculating and querying interatomic contacts with just one command (without the need to construct a pipeline from 'voronota' calls).
EOF
./voronota-contacts -h 2>&1 | sed 's/^Script.*/The script command line arguments are:\n/'

} > $TMPDIR/documentation.markdown

cat > $TMPDIR/include_in_header.html << 'EOF'
<style type="text/css">
a { color: #0000CC; }
td { padding-right: 1em; }
div#TOC > ul > li > ul > li ul { display: none; }
</style>
EOF

echo "<h1>$(./voronota | head -1)</h1>" > $TMPDIR/include_before_body.html

pandoc $TMPDIR/documentation.markdown -f markdown -t html --toc -H $TMPDIR/include_in_header.html -B $TMPDIR/include_before_body.html -s -o ./index.html
