#!/bin/bash

BIN_DIR=$1
DOWNLOADS_DIR=$2
OUTPUT_DIR=$3
TARGETS_TARBALL_URL=$4
MODELS_TARBALL_URL=$5

TARGETS_TARBALL_BASENAME=$(basename $TARGETS_TARBALL_URL)
MODELS_TARBALL_BASENAME=$(basename $MODELS_TARBALL_URL)
TARGET_NAME=$(basename $MODELS_TARBALL_BASENAME .tar.gz)

mkdir -p $DOWNLOADS_DIR
cd $DOWNLOADS_DIR

if [ ! -f "$TARGET_NAME.pdb" ] || [ ! -d "$TARGET_NAME" ]
then
	wget --timestamping $TARGETS_TARBALL_URL &> /dev/null
	tar -xzf $TARGETS_TARBALL_BASENAME
	
	wget --timestamping $MODELS_TARBALL_URL &> /dev/null
	tar -xzf $MODELS_TARBALL_BASENAME
fi

mkdir -p $OUTPUT_DIR
cd $OUTPUT_DIR

mkdir -p $TARGET_NAME
cd $TARGET_NAME

mkdir -p balls
mkdir -p contacts
mkdir -p qscores
mkdir -p cadscores

(echo $DOWNLOADS_DIR/$TARGET_NAME.pdb ; find $DOWNLOADS_DIR/$TARGET_NAME -type f) | egrep -v '_2|_3|_4|_5' | while read MODEL_FILE
do
	MODEL_NAME=$(basename $MODEL_FILE .pdb)
	MODEL_NAME=$(basename $MODEL_NAME .ent)
	if [ ! -f "balls/$MODEL_NAME" ] || [ ! -f "contacts/$MODEL_NAME" ]
	then
		$BIN_DIR/voronota get-balls-from-atoms-file --radii-file $BIN_DIR/radii --annotated < $MODEL_FILE | $BIN_DIR/voronota query-balls --drop-altloc-indicators --drop-atom-serials > balls/$MODEL_NAME
		$BIN_DIR/voronota calculate-contacts --annotated < balls/$MODEL_NAME > contacts/$MODEL_NAME
	fi
	$BIN_DIR/voronota query-contacts --match-min-seq-sep 2 < contacts/$MODEL_NAME | $BIN_DIR/voronota score-contacts --potential-file $BIN_DIR/potential | sed "s/^/$TARGET_NAME $MODEL_NAME /" > qscores/$MODEL_NAME
	$BIN_DIR/voronota query-contacts --match-min-seq-sep 1 --no-solvent < contacts/$MODEL_NAME | $BIN_DIR/voronota compare-contacts --target-contacts-file <($BIN_DIR/voronota query-contacts --match-min-seq-sep 1 --no-solvent < contacts/$TARGET_NAME) | sed "s/^/$TARGET_NAME $MODEL_NAME /" > cadscores/$MODEL_NAME
done

(echo "target model qa_score qa_normalized_energy qa_energy_score qa_actuality_score qa_total_area qa_strange_area qa_energy" ; cat qscores/* | sed 's/global //') > all_qscores
(echo "target model csa_score csa_target_area_sum csa_model_area_sum csa_raw_differences_sum csa_constrained_differences_sum" ; cat cadscores/* | egrep 'atom_level_global' | sed 's/atom_level_global //') > all_cadscores_atom
(echo "target model csr_score csr_target_area_sum csr_model_area_sum csr_raw_differences_sum csr_constrained_differences_sum" ; cat cadscores/* | egrep 'residue_level_global' | sed 's/residue_level_global //') > all_cadscores_residue

R --vanilla << EOF > /dev/null
qscores=read.table("all_qscores", header=TRUE, stringsAsFactors=FALSE);
all_cadscores_atom=read.table("all_cadscores_atom", header=TRUE, stringsAsFactors=FALSE);
all_cadscores_residue=read.table("all_cadscores_residue", header=TRUE, stringsAsFactors=FALSE);
merged_table=merge(qscores, all_cadscores_atom);
merged_table=merge(merged_table, all_cadscores_residue);
write.table(merged_table, "merged_table", quote=FALSE, row.names=FALSE);
EOF
