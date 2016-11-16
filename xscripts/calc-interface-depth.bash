#!/bin/bash

readonly ZEROARG="$0"
INFILE=""
CHAIN=""
OUTDIR=""
PROBE="1.4"
HELP_MODE=false

while [[ $# > 0 ]]
do
	OPTION="$1"
	OPTARG="$2"
	shift
	case $OPTION in
	-i|--input)
		INFILE="$OPTARG"
		shift
		;;
	--chain-name)
		CHAIN="$OPTARG"
		shift
		;;
	--output-dir)
		OUTDIR="$OPTARG"
		shift
		;;
	--probe)
		PROBE="$OPTARG"
		shift
		;;
	-h|--help)
		HELP_MODE=true
		;;
	*)
		echo >&2 "Error: invalid command line option '$OPTION'"
		exit 1
		;;
	esac
done

if [ -z "$INFILE" ] || $HELP_MODE
then
cat >&2 << EOF

Options:
    --input | -i                   string   *  input structure file in PDB format
    --chain-name                   string   *  chain name of interest
    --output-dir                   string   *  output directory
    --probe                        number      probe radius, default is 1.4
    --help | -h                                flag to display help message and exit

EOF
exit 1
fi

if [ -z "$INFILE" ] && [ ! -s "$INFILE" ]
then
	echo >&2 "Error: missing input file"
	exit 1
fi

if [ -z "$CHAIN" ]
then
	echo >&2 "Error: missing chain"
	exit 1
fi

if [ -z "$OUTDIR" ]
then
	echo >&2 "Error: missing output directory"
	exit 1
fi

if [[ "$ZEROARG" == *"/"* ]]
then
	cd $(dirname $ZEROARG)
	export PATH=$(pwd):$PATH
	cd - &> /dev/null
fi

command -v voronota &> /dev/null || { echo >&2 "Error: 'voronota' executable not in binaries path"; exit 1; }
command -v voronota-resources &> /dev/null || { echo >&2 "Error: 'voronota-resources' executable not in binaries path"; exit 1; }

mkdir -p "$OUTDIR"
if [ ! -d "$OUTDIR" ]
then
	echo >&2 "Error: could not create output directory '$OUTDIR'"
	exit 1
fi


readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

cat "$INFILE" \
| voronota get-balls-from-atoms-file \
  --annotated \
  --radii-file <(voronota-resources radii) \
  --include-heteroatoms \
| tee "$TMPLDIR/balls" \
| voronota calculate-contacts \
  --annotated \
  --probe "$PROBE" \
  --tag-peripherial \
> "$TMPLDIR/contacts"

cat "$TMPLDIR/contacts" \
| voronota query-contacts \
  --no-same-chain \
  --no-solvent \
| awk '{print $1 " " $2}' \
| tr ' ' '\n' \
| sort \
| uniq \
| grep "c<$CHAIN>" \
> "$TMPLDIR/iface_atoms"

if [ ! -s "$TMPLDIR/iface_atoms" ]
then
	echo >&2 "Error: no interface atoms for chain '$CHAIN'"
	exit 1
fi

cat "$TMPLDIR/contacts" \
| voronota query-contacts \
  --no-same-chain \
  --no-solvent \
  --match-tags 'peripherial' \
| awk '{print $1 " " $2}' \
| tr ' ' '\n' \
| sort \
| uniq \
| grep "c<$CHAIN>" \
> "$TMPLDIR/iface_atoms_peripherial"

if [ ! -s "$TMPLDIR/iface_atoms_peripherial" ]
then
	echo >&2 "Error: no peripherial interface atoms for chain '$CHAIN'"
	exit 1
fi

{
cat "$TMPLDIR/contacts" \
| voronota query-contacts \
  --match-external-first "$TMPLDIR/iface_atoms" \
  --match-external-second "$TMPLDIR/iface_atoms"

cat "$TMPLDIR/iface_atoms_peripherial" \
| awk '{print $1 " c<solvent>"}'
} \
| voronota x-query-contacts-depth-values \
  --residue-info "$TMPLDIR/residue_depth_values" \
> "$OUTDIR/atom_depth_values.txt"

cat "$TMPLDIR/residue_depth_values" \
| awk '{print $1 " " $2}' \
> "$OUTDIR/residue_depth_values.txt"

cat "$TMPLDIR/balls" \
| voronota query-balls \
  --set-adjuncts 'score=0' \
| voronota query-balls \
  --set-external-adjuncts "$OUTDIR/atom_depth_values.txt" \
  --set-external-adjuncts-name score \
| voronota write-balls-to-atoms-file \
  --pdb-output "$OUTDIR/atom_depth_values.pdb" \
  --pdb-output-b-factor score \
> /dev/null

cat "$TMPLDIR/balls" \
| voronota query-balls \
  --set-adjuncts 'score=0' \
| voronota query-balls \
  --set-external-adjuncts "$OUTDIR/residue_depth_values.txt" \
  --set-external-adjuncts-name score \
| voronota write-balls-to-atoms-file \
  --pdb-output "$OUTDIR/residue_depth_values.pdb" \
  --pdb-output-b-factor score \
> /dev/null
