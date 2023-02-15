#!/bin/bash

cd $(dirname "$0")

TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

################################################################################

{

cat ./resources/texts/intro.markdown

echo -e "\n# Wrapper scripts"

echo -e "\n## VoroMQA dark and light methods\n"

cat << 'EOF'
'voronota-js-voromqa' script provides an interface to VoroMQA dark (newer) and light (classic) methods.

### Script interface

EOF

./voronota-js-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'


echo -e "\n## VoroMQA-based membrane protein structure assessment\n"

cat << 'EOF'
'voronota-js-membrane-voromqa' script provides an interface to the VoroMQA-based method for assessing membrane protein structures.

### Script interface

EOF

./voronota-js-membrane-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'


echo -e "\n## VoroMQA-based collection of protein-protein complex features\n"

cat << 'EOF'
'voronota-js-ifeatures-voromqa' script computes multiple VoroMQA-based features of protein-protein complexes.

### Script interface

EOF

./voronota-js-ifeatures-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'


echo -e "\n## Protein-ligand interface variation of CAD-score\n"

cat << 'EOF'
'voronota-js-ligand-cadscore' script computes protein-ligand variation of CAD-score.

### Script interface

EOF

./voronota-js-ligand-cadscore -h 2>&1 | tail -n +3 | sed 's/^/    /'


} > $TMPLDIR/documentation.markdown

################################################################################

cat > $TMPLDIR/include_in_header.html << 'EOF'
<style type="text/css">
a { color: #0000CC; }
td { padding-right: 1em; }
div#TOC > ul > li > ul > li ul { display: none; }
</style>
EOF

echo "<h1>$(../voronota | head -1 | sed 's/Voronota/Voronota-JS/')</h1>" > $TMPLDIR/include_before_body.html

pandoc $TMPLDIR/documentation.markdown -f markdown -t html --toc -H $TMPLDIR/include_in_header.html -B $TMPLDIR/include_before_body.html -s -o ./index.html

################################################################################

cat $TMPLDIR/documentation.markdown \
| sed 's|\./index.html|./README.markdown|g' \
| sed 's|expansion_js/index.html|expansion_js/README.markdown|g' \
| sed 's|expansion_gl/index.html|expansion_gl/README.markdown|g' \
> ./README.markdown

################################################################################

{
cat ./resources/texts/ligand_cadscore_info.markdown

cat << 'EOF'
# Software usage reference

'voronota-js-ligand-cadscore' script computes receptor-ligand variation of CAD-score.

### Script interface

EOF

./voronota-js-ligand-cadscore -h 2>&1 | tail -n +3 | sed 's/^/    /'

} > $TMPLDIR/ligand_cadscore_info.markdown

################################################################################

cat > $TMPLDIR/include_in_header.html << 'EOF'
<style type="text/css">
a { color: #0000CC; }
td { padding-right: 1em; }
div#TOC > ul > li > ul > li ul { display: none; }
</style>
EOF

echo "<h1>Protein-ligand variation of CAD-score</h1>" > $TMPLDIR/include_before_body.html

pandoc $TMPLDIR/ligand_cadscore_info.markdown -f markdown -t html --toc -H $TMPLDIR/include_in_header.html -B $TMPLDIR/include_before_body.html -s -o ./ligand_cadscore_info.html

################################################################################

