#!/bin/bash

cd $(dirname "$0")

TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

################################################################################

{

cat ./resources/texts/intro.markdown

echo -e "# Command reference"

./voronota \
| grep 'Commands:' -A 999999 \
| sed 's/^Commands:/## List of all commands/' \
| sed 's/^\([[:alpha:]]\S*\)/* \1/'

./voronota --help \
| grep 'Command ' -A 999999 \
| sed "s/^Command\s\+'\(\S\+\)'.*/## Command '\1'\n\n### Command line arguments:\n\nCOMMAND_OPTIONS_TABLE_HEADER1\nCOMMAND_OPTIONS_TABLE_HEADER2/" \
| sed 's/COMMAND_OPTIONS_TABLE_HEADER1/    Name                            Type        Description/' \
| sed 's/COMMAND_OPTIONS_TABLE_HEADER2/    ------------------------------- ------ ---- ------------------------------------------------------------------------/' \
| sed 's/^\(--[[:alpha:]]\S*\)/    \1/' \
| sed 's/^stdin   <-\s*/\n### Input stream:\n\n    /' \
| sed 's/^stdout  ->\s*/\n### Output stream:\n\n    /' \
| sed 's/^\s\+(\(.\+\))/\n        \1/'

echo -e "# Wrapper scripts"

echo -e "\n## VoroMQA method script\n"
cat << EOF
'voronota-voromqa' script is an implementation of VoroMQA (Voronoi diagram-based Model Quality Assessment) method using Voronota.
The script interface is presented below:
EOF
./voronota-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'

echo -e "\n### Full list of options for the VoroMQA method script\n"
cat << EOF
All options of 'voronota-voromqa' script, in alphabetical order:
EOF
./voronota-voromqa --list-all-options 2>&1 | tail -n +3 | sed 's/^/    /'

echo -e "\n## CAD-score method script\n"
cat << EOF
'voronota-cadscore' script is an implementation of CAD-score (Contact Area Difference score) method using Voronota.
The script interface is presented below:
EOF
./voronota-cadscore -h 2>&1 | tail -n +3 | sed 's/^/    /'

echo -e "\n## Contacts calculation convenience script\n"
cat << EOF
'voronota-contacts' script provides a way for calculating and querying interatomic contacts with just one command (without the need to construct a pipeline from 'voronota' calls).
EOF
./voronota-contacts -h 2>&1 | tail -n +4 | sed 's/^/    /'

echo -e "\n## Volumes calculation convenience script\n"
cat << EOF
'voronota-volumes' script provides a way for calculating and querying atomic volumes with just one command (without the need to construct a pipeline from 'voronota' calls).
EOF
./voronota-volumes -h 2>&1 | tail -n +4 | sed 's/^/    /'

echo -e "\n## Pocket analysis script\n"
cat << EOF
'voronota-pocket' script provides a way for identifying and describing pockets, cavities and channels using the Voronoi tessellation vertices.
EOF
./voronota-pocket -h 2>&1 | tail -n +4 | sed 's/^/    /'

echo -e "\n## Membrane fitting script\n"
cat << EOF
'voronota-membrane' script provides a way for fitting a membrane for a protein struture using VoroMQA-based surface frustration analysis.
EOF
./voronota-membrane -h 2>&1 | tail -n +4 | sed 's/^/    /'

} > $TMPLDIR/documentation.markdown

################################################################################

DOCUMENTTITLE="$(./voronota | head -1)"

{
cat << 'EOF'
<style type="text/css">
a { color: #0000CC; }
td { padding-right: 1em; }
pre { background-color: #DDDDDD; padding: 1em; }
div#TOC > ul > li > ul > li ul { display: none; }
</style>
EOF
} \
> $TMPLDIR/include_in_header.html

{
echo "<h1>$DOCUMENTTITLE</h1>"

cat << 'EOF'
<h2>Quick links</h2>
<ul>
<li><a href="./expansion_js/index.html">Voronota-JS</a> (for advanced scripting using JavaScript)</li>
<li><a href="./expansion_gl/index.html">Voronota-GL</a> (for advanced scripting and visualization)</li>
<li><a href="./expansion_gl/web/index.html">Web Voronota-GL</a> (online version of Voronota-GL)</li>
</ul>

<h2>Table of contents</h2>
EOF
} \
> $TMPLDIR/include_before_body.html

pandoc $TMPLDIR/documentation.markdown -f markdown -t html --metadata title="$DOCUMENTTITLE" --variable title="" -M document-css=false --wrap=none --toc-depth 2 --toc -H $TMPLDIR/include_in_header.html -B $TMPLDIR/include_before_body.html -s -o ./index.html

################################################################################

cat $TMPLDIR/documentation.markdown \
| sed 's|\./index.html|./README.md|g' \
| sed 's|expansion_js/index.html|expansion_js/README.md|g' \
| sed 's|expansion_gl/index.html|expansion_gl/README.md|g' \
> ./README.md

################################################################################

pandoc -s -t man --wrap=none ./resources/texts/manpage.markdown -o "$TMPLDIR/manpage.troff"
mv "$TMPLDIR/manpage.troff" "./voronota.man"

################################################################################

./expansion_js/document.bash

./expansion_gl/document.bash

################################################################################

