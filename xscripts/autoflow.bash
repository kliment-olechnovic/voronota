#!/bin/bash

VERSIONID=$(cat ./src/main.cpp | grep '"Voronota version ' | sed 's/.*"Voronota version \(\S\+\)";/\1/')

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

cp -r ./src "$TMPLDIR/"
cp ./voronota-resources ./voronota-voromqa ./voronota-contacts ./voronota-cadscore "$TMPLDIR/"

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
dist_bin_SCRIPTS = voronota-resources voronota-voromqa voronota-contacts voronota-cadscore
EOF
} \
> "./Makefile.am"

autoreconf -i

./configure
make dist

cd -
cp "$TMPLDIR/voronota-$VERSIONID.tar.gz" "./voronota-$VERSIONID.tar.gz"
