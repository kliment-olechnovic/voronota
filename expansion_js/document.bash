#!/bin/bash

cd $(dirname "$0")

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

{

cat ./resources/texts/intro.markdown

echo -e "\n# Usage"


echo -e "\n## VoroMQA dark and light methods\n"

cat << EOF
'voronota-js-voromqa' script provides an interface to VoroMQA dark (newer) and light (classic) methods.

### Script interface

EOF

./voronota-js-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'


echo -e "\n## VoroMQA-based membrane protein structure assessment\n"

cat << EOF
'voronota-js-membrane-voromqa' script provides an interface to the VoroMQA-based method for assessing membrane protein structures.

### Script interface

EOF

./voronota-js-membrane-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'


echo -e "\n## VoroMQA-based collection of protein-protein complex features\n"

cat << EOF
'voronota-js-ifeatures-voromqa' script computes multiple VoroMQA-based features of protein-protein complexes.

### Script interface

EOF

./voronota-js-ifeatures-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'


echo -e "\n## Protein-ligand interface variation of CAD-score\n"

cat << EOF
'voronota-js-ligand-cadscore' script computes protein-ligand variation of CAD-score.

### Script interface

EOF

./voronota-js-ligand-cadscore -h 2>&1 | tail -n +3 | sed 's/^/    /'


} > $TMPDIR/documentation.markdown

cat > $TMPDIR/include_in_header.html << 'EOF'
<style type="text/css">
a { color: #0000CC; }
td { padding-right: 1em; }
div#TOC > ul > li > ul > li ul { display: none; }
</style>
EOF

echo "<h1>Voronota-JS</h1>" > $TMPDIR/include_before_body.html

pandoc $TMPDIR/documentation.markdown -f markdown -t html --toc -H $TMPDIR/include_in_header.html -B $TMPDIR/include_before_body.html -s -o ./index.html

mv $TMPDIR/documentation.markdown ./README.markdown

