#!/bin/bash

cd $(dirname "$0")

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

{

cat ./resources/texts/intro.markdown

echo -e "\n## VoroMQA dark and light method script\n"
cat << EOF
'voronota-js-voromqa' script provides an interface to VoroMQA dark (newer) and light (older) methods.
The script interface is presented below:
EOF
./voronota-js-voromqa -h 2>&1 | tail -n +3 | sed 's/^/    /'

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

