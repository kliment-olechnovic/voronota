#!/bin/bash

cd $(dirname "$0")

TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

################################################################################

{

cat ./resources/texts/intro.markdown

echo

cat ./resources/texts/data_graph_generation_info.markdown

echo

cat ./resources/texts/ligand_cadscore_info.markdown

echo -e "\n# Wrapper scripts"

########################################

echo -e "\n## VoroMQA dark and light methods\n"

cat << 'EOF'
'voronota-js-voromqa' script provides an interface to VoroMQA dark (newer) and light (classic) methods.

### Script interface

EOF

./voronota-js-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## VoroMQA for only global scores with fast caching\n"

cat << 'EOF'
'voronota-js-only-global-voromqa' script computes global VoroMQA scores and can use fast caching.

### Script interface

EOF

./voronota-js-only-global-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## VoroMQA-based membrane protein structure assessment\n"

cat << 'EOF'
'voronota-js-membrane-voromqa' script provides an interface to the VoroMQA-based method for assessing membrane protein structures.

### Script interface

EOF

./voronota-js-membrane-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## VoroMQA-based collection of protein-protein complex features\n"

cat << 'EOF'
'voronota-js-ifeatures-voromqa' script computes multiple VoroMQA-based features of protein-protein complexes.

### Script interface

EOF

./voronota-js-ifeatures-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## Fast inter-chain interface VoroMQA energy\n"

cat << 'EOF'
'voronota-js-fast-iface-voromqa' script rapidly computes VoroMQA-based interface energy of protein complexes.

### Script interface

EOF

./voronota-js-fast-iface-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## Fast inter-chain interface CAD-score\n"

cat << 'EOF'
'voronota-js-fast-iface-cadscore' script rapidly computes interface CAD-score for two protein complexes.

### Script interface

EOF

./voronota-js-fast-iface-cadscore -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## Fast inter-chain interface CAD-score matrix\n"

cat << 'EOF'
'voronota-js-fast-iface-cadscore-matrix' script rapidly computes interface CAD-score between complexes.

### Script interface

EOF

./voronota-js-fast-iface-cadscore-matrix -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## Fast inter-chain interface contacts\n"

cat << 'EOF'
'voronota-js-fast-iface-contacts' script rapidly computes contacts of inter-chain interface in a molecular complex.

### Script interface

EOF

./voronota-js-fast-iface-contacts -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## Computation of inter-chain interface graphs\n"

cat << 'EOF'
'voronota-js-fast-iface-data-graph' script generates interface data graphs of protein complexes.

### Script interface

EOF

./voronota-js-fast-iface-data-graph -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## VoroIF-GNN method for scoring models of protein-protein complexes\n"

cat << 'EOF'
'voronota-js-voroif-gnn' scores protein-protein interfaces using the VoroIF-GNN method

### Script interface

EOF

./voronota-js-voroif-gnn -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## Protein-ligand interface variation of CAD-score\n"

cat << 'EOF'
'voronota-js-ligand-cadscore' script computes protein-ligand variation of CAD-score.

### Script interface

EOF

./voronota-js-ligand-cadscore -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## Global CAD-score, with an option to use the Voronota-LT algorithm\n"

cat << 'EOF'
'voronota-js-global-cadscore' script computes global CAD-score between molecular structures.

### Script interface

EOF

./voronota-js-global-cadscore -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

echo -e "\n## Global CAD-score matrix, with an option to use the Voronota-LT algorithm\n"

cat << 'EOF'
'voronota-js-global-cadscore-matrix' script computes global CAD-score for all pairs of provided molecular structures.

### Script interface

EOF

./voronota-js-global-cadscore-matrix -h 2>&1 | tail -n +3 | sed 's/^/    /'

########################################

} > $TMPLDIR/documentation.markdown

################################################################################

cat > $TMPLDIR/include_in_header.html << 'EOF'
<style type="text/css">
a { color: #0000CC; }
td { padding-right: 1em; }
pre { background-color: #DDDDDD; padding: 1em; }
div#TOC > ul > li > ul > li ul { display: none; }
</style>
EOF

DOCUMENTTITLE="$(./voronota-js --version | head -1)"

pandoc $TMPLDIR/documentation.markdown -f markdown -t html --metadata title="$DOCUMENTTITLE" -M document-css=false --wrap=none --toc-depth 2 --toc -H $TMPLDIR/include_in_header.html -s -o ./index.html

################################################################################

cat $TMPLDIR/documentation.markdown \
| sed 's|\./index.html|./README.md|g' \
| sed 's|expansion_js/index.html|expansion_js/README.md|g' \
| sed 's|expansion_gl/index.html|expansion_gl/README.md|g' \
> ./README.md

################################################################################

