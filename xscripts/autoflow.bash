#!/bin/bash

VERSION_MAJOR=$(cat ./src/main.cpp | grep '"Voronota version ' | sed 's/.*"Voronota version \(\S\+\)";/\1/')
VERSION_MINOR=$(hg branches | egrep '^default' | tr ':' ' ' | awk '{print $2}')
VERSIONID="${VERSION_MAJOR}.${VERSION_MINOR}"

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

cp -r ./src "$TMPLDIR/"
cp ./voronota-resources ./voronota-voromqa ./voronota-contacts ./voronota-cadscore ./voronota-volumes "$TMPLDIR/"

cd $TMPLDIR

{
cat << 'EOF'
AC_INIT([voronota], [VERSIONID])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CXX
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
EOF
} \
| sed "s/VERSIONID/$VERSIONID/" \
> "./configure.ac"

{
cat << EOF
bin_PROGRAMS = voronota
voronota_SOURCES = $(find src/ -type f | tr '\n' ' ')
dist_bin_SCRIPTS = voronota-resources voronota-voromqa voronota-contacts voronota-cadscore voronota-volumes
EOF
} \
> "./Makefile.am"

autoreconf -i

make maintainer-clean

./configure
make dist

cd -
cp "$TMPLDIR/voronota-$VERSIONID.tar.gz" "./voronota-$VERSIONID.tar.gz"
