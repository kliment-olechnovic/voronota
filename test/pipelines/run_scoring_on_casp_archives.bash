#!/bin/bash

BIN_DIR=$1
DOWNLOADS_DIR=$2
OUTPUT_DIR=$3
CLEARINPUT=$4
CLEARDETAILEDOUTPUT=$5
TARGETS_TARBALL_URL=$6
MODELS_TARBALL_URL=$7

TARGETS_TARBALL_BASENAME=$(basename $TARGETS_TARBALL_URL)
MODELS_TARBALL_BASENAME=$(basename $MODELS_TARBALL_URL)
TARGET_NAME=$(basename $MODELS_TARBALL_BASENAME .tar.gz)
TARGET_NAME=$(basename $TARGET_NAME .tgz)
TARGET_NAME=$(basename $TARGET_NAME .3D.srv)
TARGET_NAME=$(basename $TARGET_NAME .stage1)
TARGET_NAME=$(basename $TARGET_NAME .stage2)

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
mkdir -p qscores_atom
mkdir -p qscores_residue
mkdir -p qscores_residue_atomic
mkdir -p cadscores
mkdir -p cadscores_residue

true > ./qscores_residue_table
true > ./qscores_residue_atomic_table
true > ./cadscores_residue_table

(echo $DOWNLOADS_DIR/$TARGET_NAME.pdb ; find $DOWNLOADS_DIR/$TARGET_NAME -type f) | while read MODEL_FILE
do
	MODEL_NAME=$(basename $MODEL_FILE .pdb)
	MODEL_NAME=$(basename $MODEL_NAME .ent)
	if [ ! -f "balls/$MODEL_NAME" ] || [ ! -f "contacts/$MODEL_NAME" ]
	then
		$BIN_DIR/voronota get-balls-from-atoms-file --radii-file $BIN_DIR/radii --annotated < $MODEL_FILE | $BIN_DIR/voronota query-balls --drop-altloc-indicators --drop-atom-serials --seq-output sequences/$MODEL_NAME | sed 's/c<.>//' > balls/raw_$MODEL_NAME
		$BIN_DIR/voronota query-balls --set-ref-seq-num-adjunct sequences/$TARGET_NAME --ref-seq-alignment alignments/$MODEL_NAME < balls/raw_$MODEL_NAME | $BIN_DIR/voronota query-balls --renumber-from-adjunct refseq > balls/$MODEL_NAME ; rm balls/raw_$MODEL_NAME
		$BIN_DIR/voronota calculate-contacts --annotated < balls/$MODEL_NAME > contacts/$MODEL_NAME
	fi
	cat contacts/$MODEL_NAME | $BIN_DIR/voronota score-contacts --detailed-output --potential-file $BIN_DIR/potential --atom-scores-file qscores_atom/$MODEL_NAME --residue-scores-file qscores_residue/$MODEL_NAME --residue-atomic-scores-file qscores_residue_atomic/$MODEL_NAME | sed "s/^/$TARGET_NAME $MODEL_NAME /" > qscores/$MODEL_NAME
	$BIN_DIR/voronota query-contacts --match-min-seq-sep 1 --no-solvent < contacts/$MODEL_NAME | $BIN_DIR/voronota compare-contacts --detailed-output --residue-scores-file cadscores_residue/$MODEL_NAME --target-contacts-file <($BIN_DIR/voronota query-contacts --match-min-seq-sep 1 --no-solvent < contacts/$TARGET_NAME) | sed "s/^/$TARGET_NAME $MODEL_NAME /" > cadscores/$MODEL_NAME
	cat qscores_residue/$MODEL_NAME | awk '{print $1 " " $2}' | sed "s/^/$TARGET_NAME $MODEL_NAME /" >> ./qscores_residue_table
	cat qscores_residue_atomic/$MODEL_NAME | awk '{print $1 " " $2}' | sed "s/^/$TARGET_NAME $MODEL_NAME /" >> ./qscores_residue_atomic_table
	cat cadscores_residue/$MODEL_NAME | awk '{print $1 " " $2}' | sed "s/^/$TARGET_NAME $MODEL_NAME /" >> ./cadscores_residue_table
done

(echo "target model qa_score qa_normalized_energy qa_energy_score qa_actuality_score qa_total_area qa_strange_area qa_energy qa_contacts_count" ; cat qscores/* | egrep 'global' | sed 's/global //') > all_qscores_global
(echo "target model qasa_quality_score qasa_reference_count qasa_count qasa_score qasa_normalized_energy qasa_energy_score qasa_actuality_score" ; cat qscores/* | egrep 'atom_level_summary' | sed 's/atom_level_summary //') > all_qscores_summary_atom
(echo "target model qasr_quality_score qasr_reference_count qasr_count qasr_score qasr_normalized_energy qasr_energy_score qasr_actuality_score" ; cat qscores/* | egrep 'residue_level_summary' | sed 's/residue_level_summary //') > all_qscores_summary_residue
(echo "target model csa_score csa_target_area_sum csa_model_area_sum csa_raw_differences_sum csa_constrained_differences_sum" ; cat cadscores/* | egrep 'atom_level_global' | sed 's/atom_level_global //') > all_cadscores_atom
(echo "target model csr_score csr_target_area_sum csr_model_area_sum csr_raw_differences_sum csr_constrained_differences_sum" ; cat cadscores/* | egrep 'residue_level_global' | sed 's/residue_level_global //') > all_cadscores_residue

if [ "$CLEARINPUT" == "yes" ]
then
	rm -r ./balls ./contacts $DOWNLOADS_DIR/$MODELS_TARBALL_BASENAME $DOWNLOADS_DIR/$TARGET_NAME
fi

if [ "$CLEARDETAILEDOUTPUT" == "yes" ]
then
	rm -r ./qscores_atom ./qscores_residue ./qscores_residue_atomic ./cadscores_residue
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

png("csr_score__vs__qa_energy.png", height=4, width=4.5, units="in", res=100);
plot(x=c(st$csr_score, tt$csr_score), y=c(st$qa_energy, tt$qa_energy), xlim=c(0, 1), col="red", xlab="csr_score", ylab="qa_energy", main=paste(tt$target[1], "corP =", format(cor(st$csr_score, st$qa_energy), digits=3), "corS =", format(cor(st$csr_score, st$qa_energy, method="spearman"), digits=3)));
dev.off();

png("csa_score__vs__qa_energy.png", height=4, width=4.5, units="in", res=100);
plot(x=c(st$csa_score, tt$csa_score), y=c(st$qa_energy, tt$qa_energy), xlim=c(0, 1), col="purple", xlab="csa_score", ylab="qa_energy", main=paste(tt$target[1], "corP =", format(cor(st$csa_score, st$qa_energy), digits=3), "corS =", format(cor(st$csa_score, st$qa_energy, method="spearman"), digits=3)));
dev.off();

png("csr_score__vs__qa_score.png", height=4, width=4.5, units="in", res=100);
plot(x=c(0, 1), y=c(0, 1), type="l", xlab="csr_score", ylab="qa_score", main=paste(tt$target[1], "corP =", format(cor(st$csr_score, st$qa_score), digits=3), "corS =", format(cor(st$csr_score, st$qa_score, method="spearman"), digits=3)));
points(st$csr_score, st$qa_score, col="black");
points(tt$csr_score, tt$qa_score, col="black");
dev.off();

png("csa_score__vs__qa_score.png", height=4, width=4.5, units="in", res=100);
plot(x=c(0, 1), y=c(0, 1), type="l", xlab="csa_score", ylab="qa_score", main=paste(tt$target[1], "corP =", format(cor(st$csa_score, st$qa_score), digits=3), "corS =", format(cor(st$csa_score, st$qa_score, method="spearman"), digits=3)));
points(st$csa_score, st$qa_score, col="brown");
points(tt$csa_score, tt$qa_score, col="brown");
dev.off();

png("csr_score__vs__qasr_normalized_score.png", height=4, width=4.5, units="in", res=100);
plot(x=c(0, 1), y=c(0, 1), type="l", xlab="csr_score", ylab="qasr_normalized_score", main=paste(tt$target[1], "corP =", format(cor(st$csr_score, st$qasr_normalized_score), digits=3), "corS =", format(cor(st$csr_score, st$qasr_normalized_score, method="spearman"), digits=3)));
points(st$csr_score, st$qasr_normalized_score, col="blue");
points(tt$csr_score, tt$qasr_normalized_score, col="blue");
dev.off();

png("csa_score__vs__qasa_normalized_score.png", height=4, width=4.5, units="in", res=100);
plot(x=c(0, 1), y=c(0, 1), type="l", xlab="csa_score", ylab="qasa_normalized_score", main=paste(tt$target[1], "corP =", format(cor(st$csa_score, st$qasa_normalized_score), digits=3), "corS =", format(cor(st$csa_score, st$qasa_normalized_score, method="spearman"), digits=3)));
points(st$csa_score, st$qasa_normalized_score, col="green");
points(tt$csa_score, tt$qasa_normalized_score, col="green");
dev.off();

write(x=c(cor(st$csr_score, st$qa_energy), cor(st$csa_score, st$qa_energy), cor(st$csr_score, st$qa_score), cor(st$csa_score, st$qa_score), cor(st$csr_score, st$qasr_normalized_score), cor(st$csr_score, st$qasa_normalized_score), cor(st$csa_score, st$qasa_normalized_score)), file="cors", ncolumns=100);

EOF

echo `cat cors | sed "s/^/$TARGET_NAME /"` > cors
cat cors >> cors_history
