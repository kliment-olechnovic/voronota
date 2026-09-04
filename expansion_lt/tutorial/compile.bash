#!/bin/bash

cd $(dirname "$0")

TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

rm -f ./index.html

cat > $TMPLDIR/include_in_header.html << 'EOF'
<style type="text/css">
a { color: #0000CC; }
td { padding-right: 1em; }
pre { background-color: #EEEEEE; padding: 1em; }
div#TOC > ul > li > ul > li ul { display: none; }
</style>
EOF

pandoc ./index.md -f markdown -t html --metadata title="Voronota-LT tutorial" -M document-css=false --wrap=none --toc-depth 2 --toc -H $TMPLDIR/include_in_header.html -s -o ./index.html
