#!/bin/bash

cd "$(dirname "$0")"

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

mkdir -p "$TMPLDIR/voronota-duktaper-preview/src/duktaper"

cp "./voronota-duktaper" "$TMPLDIR/voronota-duktaper-preview/voronota-duktaper"
cp "./voronota-duktaper-voromqa" "$TMPLDIR/voronota-duktaper-preview/voronota-duktaper-voromqa"

cp -r "../src" "$TMPLDIR/voronota-duktaper-preview/src/src"
cp -r "../viewer/src" "$TMPLDIR/voronota-duktaper-preview/src/duktaper/src"

cd "$TMPLDIR/voronota-duktaper-preview"

rm -f "./src/src/voronota.cpp"
rm -rf "./src/src/modes"

rm -f "./src/duktaper/src/voronota_viewer.cpp"
rm -rf "./src/duktaper/src/viewer"
rm -rf "./src/duktaper/src/uv"
rm -rf "./src/duktaper/src/dependencies/imgui"

cat > "./compile.sh" << 'EOF'
#!/bin/bash

cd "$(dirname "$0")"

g++ -std=c++14 -I"./src/duktaper/src/dependencies" -O3 -o "./voronota-duktaper" \
  "./src/duktaper/src/dependencies/tmalign/TMalign.cpp" \
  "./src/duktaper/src/dependencies/duktape/duktape.cpp" \
  "./src/duktaper/src/voronota_duktaper.cpp"
EOF

chmod +x "./compile.sh"

cd - &> /dev/null

cd "$TMPLDIR"

tar -czf "./voronota-duktaper-preview.tar.gz" "./voronota-duktaper-preview"

cd - &> /dev/null

mkdir -p "./previews"

mv "$TMPLDIR/voronota-duktaper-preview.tar.gz" "./previews/voronota-duktaper-preview.tar.gz"

