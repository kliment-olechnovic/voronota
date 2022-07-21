#!/bin/bash

cd $(dirname "$0")

function write_resource_file
{
	NAME=$1
	FILE=$2
	echo "inline const char* ${NAME}()"
	echo "{"
	echo "static const char value[]={"
	cat "$FILE" | xxd -i | sed -e '$s/$/, 0x00/'
	echo "};"
	echo "return value;"
	echo "}"
}

{

cat << 'EOF'
#ifndef UV_STOCKED_DEFAULT_SHADERS_H_
#define UV_STOCKED_DEFAULT_SHADERS_H_

namespace voronota
{

namespace uv
{
EOF

echo
write_resource_file default_shader_vertex_simple shader_vertex_simple.glsl
echo
write_resource_file default_shader_vertex_with_instancing shader_vertex_with_instancing.glsl
echo
write_resource_file default_shader_vertex_with_impostoring shader_vertex_with_impostoring.glsl
echo
write_resource_file default_shader_fragment_simple shader_fragment_simple.glsl
echo
write_resource_file default_shader_fragment_with_impostoring shader_fragment_with_impostoring.glsl
echo

cat << 'EOF'
}

}

#endif /* UV_STOCKED_DEFAULT_SHADERS_H_ */
EOF

} > ./src/uv/stocked_default_shaders.h

