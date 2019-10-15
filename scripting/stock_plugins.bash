#!/bin/bash

cd $(dirname "$0")

function write_resource_file
{
	RESOURCE_FILE=$1
	RESOURCE_BASENAME=$(basename $RESOURCE_FILE)
	echo -e '\nif [ "$RESOURCE_NAME" == "RESOURCE_NAME_VALUE" ]' | sed "s/RESOURCE_NAME_VALUE/$RESOURCE_BASENAME/"
	echo -e "then\ncat << 'GLOBALEOF'"
	cat $RESOURCE_FILE
	echo -e "GLOBALEOF\nfi"
}

while read FILENAME
do
	if [ ! -s "plugins/$FILENAME" ]
	then
		echo >&2 "Error: missing file 'plugins/$FILENAME'"
		exit 1
	fi
done < list_of_plugins

{
cat << 'EOF'
#!/bin/bash

RESOURCE_NAME=$1

if [ -z $RESOURCE_NAME ]
then
	echo 'Need name as argument, available options are:' >&2
	echo list_of_plugins >&2
	$0 list_of_plugins >&2
fi
EOF

write_resource_file list_of_plugins

cat list_of_plugins \
| while read FILENAME
do
	write_resource_file "plugins/$FILENAME"
done
} > ./voronota-scripting-plugins

chmod +x ./voronota-scripting-plugins

