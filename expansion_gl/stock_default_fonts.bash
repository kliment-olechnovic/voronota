#!/bin/bash

cd $(dirname "$0")

function write_resource_file
{
	NAME=$1
	FILE=$2
	
	echo "inline unsigned char* ${NAME}_data()"
	echo "{"
	echo "static unsigned char value[]={"
	cat "$FILE" | xxd -i
	echo "};"
	echo "return value;"
	echo "}"
	echo
	echo "inline int ${NAME}_data_size()"
	echo "{"
	echo "static const int size=$(cat ${FILE} | xxd -i | tr ',' '\n' | tr ' ' '\n' | egrep '^0x' | wc -l);"
	echo "return size;"
	echo "}"
}

{

cat << 'EOF'
#ifndef UV_STOCKED_DEFAULT_FONTS_H_
#define UV_STOCKED_DEFAULT_FONTS_H_

namespace voronota
{

namespace uv
{
EOF

echo
write_resource_file default_font_mono_regular ./resources/fonts/ubuntu_mono_regular.ttf
echo

cat << 'EOF'
}

}

#endif /* UV_STOCKED_DEFAULT_FONTS_H_ */
EOF

} > ./src/uv/stocked_default_fonts.h

