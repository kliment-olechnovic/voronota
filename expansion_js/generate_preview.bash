#!/bin/bash

cd "$(dirname "$0")"

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

mkdir -p "$TMPLDIR/voronota-js-preview/src/expansion_js"

cp "./voronota-js" "$TMPLDIR/voronota-js-preview/voronota-js"
cp "./voronota-js-voromqa" "$TMPLDIR/voronota-js-preview/voronota-js-voromqa"

cp -r "../src" "$TMPLDIR/voronota-js-preview/src/src"
cp -r "../expansion_js/src" "$TMPLDIR/voronota-js-preview/src/expansion_js/src"

cd "$TMPLDIR/voronota-js-preview"

rm -f "./src/src/voronota.cpp"
rm -rf "./src/src/modes"

cat > "./compile.sh" << 'EOF'
#!/bin/bash

cd "$(dirname "$0")"

g++ -std=c++14 -I"./src/expansion_js/src/dependencies" -O3 -o "./voronota-js" \
  "./src/expansion_js/src/dependencies/tmalign/TMalign.cpp" \
  "./src/expansion_js/src/dependencies/duktape/duktape.cpp" \
  "./src/expansion_js/src/voronota_js.cpp"
EOF

chmod +x "./compile.sh"

cd - &> /dev/null

cd "$TMPLDIR"

tar -czf "./voronota-js-preview.tar.gz" "./voronota-js-preview"

cd - &> /dev/null

mkdir -p "./previews"

mv "$TMPLDIR/voronota-js-preview.tar.gz" "./previews/voronota-js-preview.tar.gz"

