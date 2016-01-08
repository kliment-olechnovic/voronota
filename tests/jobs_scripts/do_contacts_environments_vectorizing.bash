#!/bin/bash

SUBDIR=$OUTPUTDIR/contacts_emvironments_vectorizing
mkdir -p $SUBDIR

cat > $SUBDIR/environment_names << 'EOF'
R<ALA>A<C>
R<ALA>A<CA>
R<ALA>A<CB>
R<ALA>A<N>
R<ALA>A<O>
R<ARG>A<C>
R<ARG>A<CA>
R<ARG>A<CB>
R<ARG>A<CD>
R<ARG>A<CG>
R<ARG>A<CZ>
R<ARG>A<N>
R<ARG>A<NE>
R<ARG>A<NH1>
R<ARG>A<O>
R<ASN>A<C>
R<ASN>A<CA>
R<ASN>A<CB>
R<ASN>A<CG>
R<ASN>A<N>
R<ASN>A<ND2>
R<ASN>A<O>
R<ASN>A<OD1>
R<ASP>A<C>
R<ASP>A<CA>
R<ASP>A<CB>
R<ASP>A<CG>
R<ASP>A<N>
R<ASP>A<O>
R<ASP>A<OD1>
R<CYS>A<C>
R<CYS>A<CA>
R<CYS>A<CB>
R<CYS>A<N>
R<CYS>A<O>
R<CYS>A<SG>
R<GLN>A<C>
R<GLN>A<CA>
R<GLN>A<CB>
R<GLN>A<CD>
R<GLN>A<CG>
R<GLN>A<N>
R<GLN>A<NE2>
R<GLN>A<O>
R<GLN>A<OE1>
R<GLU>A<C>
R<GLU>A<CA>
R<GLU>A<CB>
R<GLU>A<CD>
R<GLU>A<CG>
R<GLU>A<N>
R<GLU>A<O>
R<GLU>A<OE1>
R<GLY>A<C>
R<GLY>A<CA>
R<GLY>A<N>
R<GLY>A<O>
R<HIS>A<C>
R<HIS>A<CA>
R<HIS>A<CB>
R<HIS>A<CD2>
R<HIS>A<CE1>
R<HIS>A<CG>
R<HIS>A<N>
R<HIS>A<ND1>
R<HIS>A<NE2>
R<HIS>A<O>
R<ILE>A<C>
R<ILE>A<CA>
R<ILE>A<CB>
R<ILE>A<CD1>
R<ILE>A<CG1>
R<ILE>A<CG2>
R<ILE>A<N>
R<ILE>A<O>
R<LEU>A<C>
R<LEU>A<CA>
R<LEU>A<CB>
R<LEU>A<CD1>
R<LEU>A<CD2>
R<LEU>A<CG>
R<LEU>A<N>
R<LEU>A<O>
R<LYS>A<C>
R<LYS>A<CA>
R<LYS>A<CB>
R<LYS>A<CD>
R<LYS>A<CE>
R<LYS>A<CG>
R<LYS>A<N>
R<LYS>A<NZ>
R<LYS>A<O>
R<MET>A<C>
R<MET>A<CA>
R<MET>A<CB>
R<MET>A<CE>
R<MET>A<CG>
R<MET>A<N>
R<MET>A<O>
R<MET>A<SD>
R<PHE>A<C>
R<PHE>A<CA>
R<PHE>A<CB>
R<PHE>A<CD1>
R<PHE>A<CE1>
R<PHE>A<CG>
R<PHE>A<CZ>
R<PHE>A<N>
R<PHE>A<O>
R<PRO>A<C>
R<PRO>A<CA>
R<PRO>A<CB>
R<PRO>A<CD>
R<PRO>A<CG>
R<PRO>A<N>
R<PRO>A<O>
R<SER>A<C>
R<SER>A<CA>
R<SER>A<CB>
R<SER>A<N>
R<SER>A<O>
R<SER>A<OG>
R<THR>A<C>
R<THR>A<CA>
R<THR>A<CB>
R<THR>A<CG2>
R<THR>A<N>
R<THR>A<O>
R<THR>A<OG1>
R<TRP>A<C>
R<TRP>A<CA>
R<TRP>A<CB>
R<TRP>A<CD1>
R<TRP>A<CD2>
R<TRP>A<CE2>
R<TRP>A<CE3>
R<TRP>A<CG>
R<TRP>A<CH2>
R<TRP>A<CZ2>
R<TRP>A<CZ3>
R<TRP>A<N>
R<TRP>A<NE1>
R<TRP>A<O>
R<TYR>A<C>
R<TYR>A<CA>
R<TYR>A<CB>
R<TYR>A<CD1>
R<TYR>A<CE1>
R<TYR>A<CG>
R<TYR>A<CZ>
R<TYR>A<N>
R<TYR>A<O>
R<TYR>A<OH>
R<VAL>A<C>
R<VAL>A<CA>
R<VAL>A<CB>
R<VAL>A<CG1>
R<VAL>A<CG2>
R<VAL>A<N>
R<VAL>A<O>
c<solvent>
EOF

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file \
  --radii-file $VORONOTADIR/resources/radii \
  --annotated \
| $VORONOTA calculate-contacts \
  --annotated \
| $VORONOTA query-contacts \
  --match-min-seq-sep 2 \
> $SUBDIR/contacts

cat $SUBDIR/contacts \
| $VORONOTA x-vectorize-contact-environments \
  --names-file $SUBDIR/environment_names \
  --inter-residue \
  --normalize \
  --append \
  --output-files-prefix $SUBDIR/vectorized_environments_of_ \
> $SUBDIR/vectorized_environments_of_all_residues

cat $SUBDIR/contacts \
| $VORONOTA x-vectorize-contact-environments \
  --names-file $SUBDIR/environment_names \
  --binarize \
> $SUBDIR/binarized_vectorized_environments_of_all_atoms

rm $SUBDIR/environment_names $SUBDIR/contacts
