#!/bin/bash

cd $(dirname "$0")

TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

################################################################################

{

cat ./resources/texts/intro.markdown \
| sed "s/LATEST_VORONOTA_RELEASE_VERSION/$(cat ../latest_release_version.txt | tr -d '\n')/g" \
| sed "s/LATEST_CADSCORELT_RELEASE_VERSION/$(cat ./latest_release_version.txt | tr -d '\n')/g"

echo

cat ./swig/README.md

} > $TMPLDIR/documentation.markdown

################################################################################

cat > $TMPLDIR/include_in_header.html << 'EOF'
<style type="text/css">
a { color: #0000CC; }
td { padding-right: 1em; }
pre { background-color: #DDDDDD; padding: 1em; }
div#TOC > ul > li > ul > li ul { display: none; }
</style>
<script>window.MathJax = { tex: { inlineMath: [['$', '$'], ['\\(', '\\)']] }, options: { skipHtmlTags: ['script','noscript','style','textarea','pre','code'] } };</script>
<script defer src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-chtml.js"></script>
EOF

DOCUMENTTITLE="$(./version.bash major | sed 's/^/CAD-score-LT version /')"

pandoc $TMPLDIR/documentation.markdown \
  -f markdown \
  -t html \
  --metadata title="$DOCUMENTTITLE" \
  -M document-css=false \
  --wrap=none \
  --toc-depth 2 \
  --toc \
  -H $TMPLDIR/include_in_header.html \
  -s \
  -o ./index.html 2>&1 \
| grep -v 'Could not convert TeX math'

################################################################################

cat $TMPLDIR/documentation.markdown \
| sed 's|/index.html|/README.md|g' \
> ./README.md

################################################################################

