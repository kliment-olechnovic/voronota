#!/bin/bash

cd $(dirname "$0")

function write_resource_file
{
	RESOURCE_FILE=$1
	RESOURCE_BASENAME=$(basename $RESOURCE_FILE)
	echo 'if [ "$RESOURCE_NAME" == "RESOURCE_NAME_VALUE" ]' | sed "s/RESOURCE_NAME_VALUE/$RESOURCE_BASENAME/"
	echo "then"
	echo "cat << 'EOF'"
	cat $RESOURCE_FILE
	echo "EOF"
	echo "fi"
}

{
	echo '#!/bin/bash'
	echo ''
	echo 'RESOURCE_NAME=$1'
	find ./resources/ -type f -not -empty | sort | while read RESOURCE_FILE
	do
		echo ''
		write_resource_file $RESOURCE_FILE
	done
} > ./voronota-resources

chmod +x ./voronota-resources
