#!/bin/bash

function print_help_and_exit
{
cat >&2 << 'EOF'

'fetch.bash' is a script to download molecular structures from PDB

Options:
    --pdb-id | -i              string   *  PDB ID, alternatively can be provided as an unnamed argument
    --assembly-num | -a        number      assembly number
    --help | -h                            flag to display help message and exit

EOF
exit 1
}

command -v curl &> /dev/null || { echo >&2 "Error: 'curl' executable not in binaries path"; exit 1; }

PDB_ID=""
PDB_SELECTED_ASSEMBLY=""
HELP_MODE=false

while [[ $# > 0 ]]
do
	OPTION="$1"
	OPTARG="$2"
	shift
	case $OPTION in
	-i|--pdb-id)
		PDB_ID="$OPTARG"
		shift
		;;
	-a|--assembly-num)
		PDB_SELECTED_ASSEMBLY="$OPTARG"
		shift
		;;
	-h|--help)
		HELP_MODE=true
		;;
	*)
		[ -z "$PDB_ID" ] || { echo >&2 "Error: invalid command line option '$OPTION'"; exit 1; }
		PDB_ID="$OPTION"
		;;
	esac
done

if $HELP_MODE
then
	print_help_and_exit
fi

if [ -z "$PDB_ID" ]
then
	echo >&2 "Error: missing PDB ID"
	exit 1
fi

readonly TMPLDIR=$(mktemp -d)

function on_exit_final
{
	rm -r $TMPLDIR
}

trap on_exit_final EXIT

curl "http://www.rcsb.org/pdb/rest/getEntityInfo?structureId=${PDB_ID}" > "$TMPLDIR/pdbinfo" 2> /dev/null

if [ ! -s "$TMPLDIR/pdbinfo" ]
then
	echo >&2 "Error: failed to get info for PDB ID '$PDB_ID'"
	exit 1
fi

PDB_ID_CONFIRMED="$(cat "$TMPLDIR/pdbinfo" | egrep '<PDB.*structureId' | head -1 | sed 's/.*\sstructureId\s*=\s*\"\s*\(\S\+\)\s*\".*/\1/')"

PDB_METHOD="$(cat "$TMPLDIR/pdbinfo" | egrep '<Method.*name' | head -1 | sed 's/.*\sname\s*=\s*\"\s*\(\S\+\)\s*\".*/\1/')"

PDB_ASSEMBLIES="$(cat "$TMPLDIR/pdbinfo" | egrep '<PDB.*bioAssemblies' | head -1 | sed 's/.*\sbioAssemblies\s*=\s*\"\s*\(\S\+\)\s*\".*/\1/')"

if [ "$PDB_ID_CONFIRMED" != "$PDB_ID" ]
then
	echo >&2 "Error: failed to get info for PDB ID '$PDB_ID'"
	exit 1
fi

mkdir -p "$TMPLDIR/models"

PDB_FILE=""

if [ -n "$PDB_SELECTED_ASSEMBLY" ]
then
	PDB_FILE="$TMPLDIR/models/${PDB_ID}_as${PDB_SELECTED_ASSEMBLY}"
	
	curl "https://files.rcsb.org/download/${PDB_ID}.pdb${PDB_SELECTED_ASSEMBLY}.gz" 2> /dev/null \
	| zcat 2> /dev/null \
	> "$PDB_FILE"
else
	PDB_FILE="$TMPLDIR/models/${PDB_ID}"
		
	curl "https://files.rcsb.org/download/${PDB_ID}.pdb.gz" 2> /dev/null \
	| zcat 2> /dev/null \
	> "$PDB_FILE"
fi

if [ -z "$PDB_FILE" ] || [ ! -s "$PDB_FILE" ]
then
	echo >&2 "Error: failed download and extract relevant atoms"
	exit 1
fi

cat "$PDB_FILE"

