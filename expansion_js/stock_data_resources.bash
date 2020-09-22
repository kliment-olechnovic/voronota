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
#ifndef DUKTAPER_STOCKED_DATA_RESOURCES_H_
#define DUKTAPER_STOCKED_DATA_RESOURCES_H_

namespace voronota
{

namespace duktaper
{

namespace resources
{
EOF

echo
write_resource_file data_radii ../resources/radii
echo
write_resource_file data_voromqa_v1_energy_means_and_sds ../resources/voromqa_v1_energy_means_and_sds
echo
write_resource_file data_voromqa_v1_energy_potential ../resources/voromqa_v1_energy_potential
echo
write_resource_file data_voromqa_v1_energy_potential_alt ../resources/voromqa_v1_energy_potential_alt
echo
write_resource_file data_voromqa_dark_nnport_input_header ../resources/nnport/voromqa_dark/nnport_input_header
echo
write_resource_file data_voromqa_dark_nnport_input_statistics ../resources/nnport/voromqa_dark/nnport_input_statistics
echo
write_resource_file data_voromqa_dark_nnport_input_fdeep_model_json ../resources/nnport/voromqa_dark/nnport_input_fdeep_model.json
echo
write_resource_file data_script_examples ../resources/script_examples
echo

cat << 'EOF'
}

}

}

#endif /* DUKTAPER_STOCKED_DATA_RESOURCES_H_ */
EOF

} > ./src/duktaper/stocked_data_resources.h

