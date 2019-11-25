#!/bin/bash

cd $(dirname "$0")

VERSIONID=$(./version.bash)

PACKAGE_NAME_PREFIX="voronota"
PACKAGE_NAME="voronota-$VERSIONID"

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

cp -r ./src "$TMPLDIR/"
cp ./voronota-resources ./voronota-voromqa ./voronota-contacts ./voronota-cadscore ./voronota-volumes ./voronota-pocket ./voronota-membrane "$TMPLDIR/"

pandoc -s -t man ./resources/texts/manpage.markdown -o "$TMPLDIR/manpage.troff"
mv "$TMPLDIR/manpage.troff" "$TMPLDIR/voronota.man"

cd $TMPLDIR

{
cat << 'EOF'
AC_INIT([voronota], [VERSIONID], [kliment@ibt.lt], [PACKAGE_NAME_PREFIX])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CXX
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
EOF
} \
| sed "s/VERSIONID/$VERSIONID/" \
| sed "s/PACKAGE_NAME_PREFIX/$PACKAGE_NAME_PREFIX/" \
> "./configure.ac"

{
cat << EOF
bin_PROGRAMS = voronota
voronota_SOURCES = $(find src/ -type f | tr '\n' ' ')
dist_bin_SCRIPTS = voronota-resources voronota-voromqa voronota-contacts voronota-cadscore voronota-volumes voronota-pocket voronota-membrane
dist_man1_MANS = voronota.man
EOF
} \
> "./Makefile.am"

autoreconf -i

make maintainer-clean

./configure
make dist

cd -
mv "$TMPLDIR/$PACKAGE_NAME.tar.gz" "./$PACKAGE_NAME.tar.gz"
