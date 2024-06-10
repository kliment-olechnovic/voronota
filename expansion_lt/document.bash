#!/bin/bash

cd $(dirname "$0")

TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

################################################################################

cat ./src/voronota_lt.cpp | egrep '^Voronota-LT' > VERSION.txt

################################################################################

{

cat ./resources/texts/intro.markdown

echo

cat ./src/voronota_lt.cpp | egrep '^Voronota-LT' -A 999 | egrep '^\)\"' -B 999 | egrep -v '^\)\"' | sed 's/^/    /'

echo

echo "# Using Voronota-LT as a C++ library"

echo

cat ./resources/texts/cppapi.markdown

echo

cat ./resources/texts/cppapi_stateful.markdown

echo

cat ./resources/texts/pyapi.markdown

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

DOCUMENTTITLE="$(cat ./src/voronota_lt.cpp | egrep '^Voronota-LT')"

pandoc $TMPLDIR/documentation.markdown -f markdown -t html --metadata title="$DOCUMENTTITLE" -M document-css=false --wrap=none --toc-depth 2 --toc -H $TMPLDIR/include_in_header.html -s -o ./index.html

################################################################################

cat $TMPLDIR/documentation.markdown \
| sed 's|\./index.html|./README.md|g' \
| sed 's|benchmark/index.html|benchmark/README.md|g' \
> ./README.md

################################################################################

./benchmark/document.bash

################################################################################

