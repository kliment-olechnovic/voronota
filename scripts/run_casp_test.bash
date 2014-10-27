#!/bin/bash

BIN_DIR=$1
DOWNLOADS_DIR=$2
OUTPUT_DIR=$3
CLEARSPACE=$4
TARGETS_TARBALL_URL=$5
MODELS_TARBALL_URL=$6

TARGETS_TARBALL_BASENAME=$(basename $TARGETS_TARBALL_URL)
MODELS_TARBALL_BASENAME=$(basename $MODELS_TARBALL_URL)
TARGET_NAME=$(basename $MODELS_TARBALL_BASENAME .tar.gz)
TARGET_NAME=$(basename $TARGET_NAME .tgz)

mkdir -p $DOWNLOADS_DIR
cd $DOWNLOADS_DIR

if [ ! -f "$TARGETS_TARBALL_BASENAME" ]
then
	wget $TARGETS_TARBALL_URL &> /dev/null
	tar -xzf $TARGETS_TARBALL_BASENAME
fi

wget --timestamping $MODELS_TARBALL_URL &> /dev/null
tar -xzf $MODELS_TARBALL_BASENAME

###################################################

mkdir -p $OUTPUT_DIR/$TARGET_NAME
cd $OUTPUT_DIR/$TARGET_NAME

mkdir -p balls
mkdir -p sequences
mkdir -p alignments
mkdir -p contacts
mkdir -p qscores
mkdir -p cadscores

(echo $DOWNLOADS_DIR/$TARGET_NAME.pdb ; find $DOWNLOADS_DIR/$TARGET_NAME -type f) | egrep -v '_2|_3|_4|_5' | while read MODEL_FILE
do
	MODEL_NAME=$(basename $MODEL_FILE .pdb)
	MODEL_NAME=$(basename $MODEL_NAME .ent)
	if [ ! -f "balls/$MODEL_NAME" ] || [ ! -f "contacts/$MODEL_NAME" ]
	then
		$BIN_DIR/voronota get-balls-from-atoms-file --radii-file $BIN_DIR/radii --annotated < $MODEL_FILE | $BIN_DIR/voronota query-balls --drop-altloc-indicators --drop-atom-serials --seq-output sequences/$MODEL_NAME | sed 's/c<.>//' > balls/raw_$MODEL_NAME
		$BIN_DIR/voronota query-balls --set-ref-seq-num-adjunct sequences/$TARGET_NAME --ref-seq-alignment alignments/$MODEL_NAME < balls/raw_$MODEL_NAME | $BIN_DIR/voronota query-balls --renumber-from-adjunct refseq > balls/$MODEL_NAME ; rm balls/raw_$MODEL_NAME
		$BIN_DIR/voronota calculate-contacts --annotated < balls/$MODEL_NAME > contacts/$MODEL_NAME
	fi
	$BIN_DIR/voronota query-contacts --match-min-seq-sep 2 < contacts/$MODEL_NAME | $BIN_DIR/voronota score-contacts --potential-file $BIN_DIR/potential --erf-mean 0.4 --erf-sd 0.2 | sed "s/^/$TARGET_NAME $MODEL_NAME /" > qscores/$MODEL_NAME
	$BIN_DIR/voronota query-contacts --match-min-seq-sep 1 --no-solvent < contacts/$MODEL_NAME | $BIN_DIR/voronota compare-contacts --target-contacts-file <($BIN_DIR/voronota query-contacts --match-min-seq-sep 1 --no-solvent < contacts/$TARGET_NAME) | sed "s/^/$TARGET_NAME $MODEL_NAME /" > cadscores/$MODEL_NAME
done

(echo "target model qa_score qa_normalized_energy qa_energy_score qa_actuality_score qa_total_area qa_strange_area qa_energy qa_contacts_count" ; cat qscores/* | egrep 'global' | sed 's/global //') > all_qscores_global
(echo "target model qasa_count qasa_score qasa_normalized_energy qasa_energy_score qasa_actuality_score" ; cat qscores/* | egrep 'atom_level_summary' | sed 's/atom_level_summary //') > all_qscores_summary_atom
(echo "target model qasr_count qasr_score qasr_normalized_energy qasr_energy_score qasr_actuality_score" ; cat qscores/* | egrep 'residue_level_summary' | sed 's/residue_level_summary //') > all_qscores_summary_residue
(echo "target model csa_score csa_target_area_sum csa_model_area_sum csa_raw_differences_sum csa_constrained_differences_sum" ; cat cadscores/* | egrep 'atom_level_global' | sed 's/atom_level_global //') > all_cadscores_atom
(echo "target model csr_score csr_target_area_sum csr_model_area_sum csr_raw_differences_sum csr_constrained_differences_sum" ; cat cadscores/* | egrep 'residue_level_global' | sed 's/residue_level_global //') > all_cadscores_residue

if [ "$CLEARSPACE" == "yes" ]
then
	rm -r ./balls ./contacts $DOWNLOADS_DIR/$MODELS_TARBALL_BASENAME $DOWNLOADS_DIR/$TARGET_NAME
fi

###################################################

R --vanilla << 'EOF' > /dev/null

all_qscores_global=read.table("all_qscores_global", header=TRUE, stringsAsFactors=FALSE);
all_qscores_summary_atom=read.table("all_qscores_summary_atom", header=TRUE, stringsAsFactors=FALSE);
all_qscores_summary_residue=read.table("all_qscores_summary_residue", header=TRUE, stringsAsFactors=FALSE);
all_cadscores_atom=read.table("all_cadscores_atom", header=TRUE, stringsAsFactors=FALSE);
all_cadscores_residue=read.table("all_cadscores_residue", header=TRUE, stringsAsFactors=FALSE);

t=all_qscores_global;
t=merge(t, all_qscores_summary_atom);
t=merge(t, all_qscores_summary_residue);
t=merge(t, all_cadscores_atom);
t=merge(t, all_cadscores_residue);

tt=t[which(t$target==t$model),];
t$qasa_normalized_score=(t$qasa_score/tt$qasa_count);
t$qasr_normalized_score=(t$qasr_score/tt$qasr_count);
tt=t[which(t$target==t$model),];

write.table(t, "merged_table", quote=FALSE, row.names=FALSE);

st=t[which(t$target!=t$model),];
st=st[which(st$qasa_count/tt$qasa_count>0.97),];
st=st[which(st$qasr_count/tt$qasr_count>0.97),];

png("csr_score__vs__qa_score.png", height=4, width=4.5, units="in", res=100);
plot(x=c(0, 1), y=c(0, 1), type="l", xlab="csr_score", ylab="qa_score", main=paste("cor =", cor(st$csr_score, st$qa_score)));
points(st$csr_score, st$qa_score);
points(tt$csr_score, tt$qa_score);
dev.off();

png("csa_score__vs__qa_score.png", height=4, width=4.5, units="in", res=100);
plot(x=c(0, 1), y=c(0, 1), type="l", xlab="csa_score", ylab="qa_score", main=paste("cor =", cor(st$csa_score, st$qa_score)));
points(st$csa_score, st$qa_score);
points(tt$csa_score, tt$qa_score);
dev.off();

png("csr_score__vs__qasr_normalized_score.png", height=4, width=4.5, units="in", res=100);
plot(x=c(0, 1), y=c(0, 1), type="l", xlab="csr_score", ylab="qasr_normalized_score", main=paste("cor =", cor(st$csr_score, st$qasr_normalized_score)));
points(st$csr_score, st$qasr_normalized_score);
points(tt$csr_score, tt$qasr_normalized_score);
dev.off();

png("csr_score__vs__qasa_normalized_score.png", height=4, width=4.5, units="in", res=100);
plot(x=c(0, 1), y=c(0, 1), type="l", xlab="csr_score", ylab="qasa_normalized_score", main=paste("cor =", cor(st$csr_score, st$qasa_normalized_score)));
points(st$csr_score, st$qasa_normalized_score);
points(tt$csr_score, tt$qasa_normalized_score);
dev.off();

png("csa_score__vs__qasa_normalized_score.png", height=4, width=4.5, units="in", res=100);
plot(x=c(0, 1), y=c(0, 1), type="l", xlab="csa_score", ylab="qasa_normalized_score", main=paste("cor =", cor(st$csa_score, st$qasa_normalized_score)));
points(st$csa_score, st$qasa_normalized_score);
points(tt$csa_score, tt$qasa_normalized_score);
dev.off();

write(x=c(cor(st$csr_score, st$qa_score), cor(st$csa_score, st$qa_score), cor(st$csr_score, st$qasr_normalized_score), cor(st$csr_score, st$qasa_normalized_score), cor(st$csa_score, st$qasa_normalized_score)), file="cors", ncolumns=100);

EOF

echo `cat cors | sed "s/^/$TARGET_NAME /"` > cors
