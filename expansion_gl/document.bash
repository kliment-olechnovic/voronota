#!/bin/bash

cd $(dirname "$0")

TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

################################################################################

{

cat ./resources/texts/intro.markdown

} > $TMPLDIR/documentation.markdown

################################################################################

cat > $TMPLDIR/include_in_header.html << 'EOF'
<style type="text/css">
a { color: #0000CC; }
td { padding-right: 1em; }
div#TOC > ul > li > ul > li ul { display: none; }
</style>
EOF

echo "<h1>$(../voronota | head -1 | sed 's/Voronota/Voronota-GL/')</h1>" > $TMPLDIR/include_before_body.html

pandoc $TMPLDIR/documentation.markdown -f markdown -t html --toc -H $TMPLDIR/include_in_header.html -B $TMPLDIR/include_before_body.html -s -o ./index.html

################################################################################

cat $TMPLDIR/documentation.markdown \
| sed 's|\./index.html|./README.markdown|g' \
| sed 's|expansion_js/index.html|expansion_js/README.markdown|g' \
| sed 's|expansion_gl/index.html|expansion_gl/README.markdown|g' \
> ./README.markdown

################################################################################

