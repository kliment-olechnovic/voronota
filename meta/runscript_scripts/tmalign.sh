function print_help_and_exit
{
cat >&2 << 'EOF'

'tmalign.sh' is a script that can be called from the 'voronota-runscript'
environment to use TMalign to superpose two protein objects (model on target)

Options:
    --target | -t          string   *  target name
    --model | -m           string   *  model name
    --target-sel | -ts     string      target atoms selection expression
    --model-sel | -ms      string      model atoms selection expression
    --help | -h                        flag to display help message and exit

EOF
exit 1
}

vassert

TARGET_NAME=""
MODEL_NAME=""
TARGET_SEL="[]"
MODEL_SEL="[]"
HELP_MODE=false

while [[ $# > 0 ]]
do
	OPTION="$1"
	OPTARG="$2"
	shift
	case $OPTION in
	-t|--target)
		TARGET_NAME="$OPTARG"
		shift
		;;
	-m|--model)
		MODEL_NAME="$OPTARG"
		shift
		;;
	-ts|--target-sel)
		TARGET_SEL="$OPTARG"
		shift
		;;
	-ms|--model-sel)
		MODEL_SEL="$OPTARG"
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

if $HELP_MODE
then
	print_help_and_exit
fi

if [ -z "$TARGET_NAME" ]
then
	echo >&2 "Error: missing target name"
	exit 1
fi

if [ -z "$MODEL_NAME" ]
then
	echo >&2 "Error: missing model name"
	exit 1
fi

command -v TMalign &> /dev/null || { echo >&2 "Error: 'TMalign' executable not in binaries path"; exit 1; }

vq "list-objects '$TARGET_NAME'"

if [ "$V_TAIL_SUCCESS" != "true" ]
then
	echo >&2 "Error: no target object '$TARGET_NAME'"
	exit 1
fi

vq "list-objects '$MODEL_NAME'"

if [ "$V_TAIL_SUCCESS" != "true" ]
then
	echo >&2 "Error: no model object '$MODEL_NAME'"
	exit 1
fi

readonly TMPLDIR=$(mktemp -d)

function on_exit_final
{
	vq "unpick-objects"
	rm -r $TMPLDIR
}

trap on_exit_final EXIT

vq "pick-objects '$TARGET_NAME'"
vq "save-atoms -use '(($TARGET_SEL) and ([-aname CA]))' -as-pdb -file '$TMPLDIR/target.pdb'"

if [ ! -s "$TMPLDIR/target.pdb" ]
then
	echo >&2 "Error: no target atoms for selection '$TARGET_SEL'"
	exit 1
fi

vq "pick-objects '$MODEL_NAME'"
vq "save-atoms -use '(($MODEL_SEL) and ([-aname CA]))' -as-pdb -file '$TMPLDIR/model.pdb'"

if [ ! -s "$TMPLDIR/model.pdb" ]
then
	echo >&2 "Error: no model atoms for selection '$MODEL_SEL'"
	exit 1
fi

TMalign "$TMPLDIR/model.pdb" "$TMPLDIR/target.pdb" -m "$TMPLDIR/matrix" > "$TMPLDIR/tmalign.out"

TMSCORE="$(cat "$TMPLDIR/tmalign.out" | egrep '^TM-score= ' | grep 'Chain_2' | sed 's/^TM-score=\s*\(\S*\)\s*.*/\1/')"

if [ -z "$TMSCORE" ] || [ ! -s "$TMPLDIR/matrix" ]
then
	echo >&2 "Error: TMalign execution failed"
	exit 1
fi

echo "TM-score = $TMSCORE"

TRANSLATION=$(cat "$TMPLDIR/matrix" | head -5 | tail -3 | awk '{print $2}' | tr '\n' ' ')
ROTATION=$(cat "$TMPLDIR/matrix" | head -5 | tail -3 | awk '{print $3 " " $4 " " $5}' | tr '\n' ' ')

v "pick-objects '$MODEL_NAME'"
v "move-atoms -rotate-by-matrix $ROTATION -translate $TRANSLATION"

if [ "$V_TAIL_SUCCESS" != "true" ]
then
	echo >&2 "Error: failed to move model atoms"
	exit 1
fi

if [ "$V_GUI" == "true" ]
then
	v "pick-objects '$TARGET_NAME'"
	v "zoom-by-atoms -use '$TARGET_SEL'"
	v "unpick-objects"
fi

