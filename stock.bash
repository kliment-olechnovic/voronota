#!/bin/bash

cd $(dirname "$0")

function write_resource_file
{
	RESOURCE_FILE=$1
	RESOURCE_BASENAME=$(basename $RESOURCE_FILE)
	echo -e '\nif [ "$RESOURCE_NAME" == "RESOURCE_NAME_VALUE" ]' | sed "s/RESOURCE_NAME_VALUE/$RESOURCE_BASENAME/"
	echo -e "then\ncat << 'EOF'"
	cat $RESOURCE_FILE
	echo -e "EOF\nfi"
}

{
	
cat << 'EOF'
#!/bin/bash

RESOURCE_NAME=$1

if [ -z $RESOURCE_NAME ]
then
echo -e 'Need resource name as argument, available options are:\n  radii\n  voromqa_v1_energy_potential\n  voromqa_v1_energy_means_and_sds' >&2
fi
EOF

write_resource_file ./resources/radii
write_resource_file ./resources/voromqa_v1_energy_potential
write_resource_file ./resources/voromqa_v1_energy_means_and_sds

} > ./voronota-resources

chmod +x ./voronota-resources
