#!/bin/bash

readonly INPUTFILE=$1
readonly TMPDIR=$(mktemp -d)

cp $INPUTFILE $TMPDIR/struct.pdb

cd $TMPDIR

hbplus ./struct.pdb 1>&2

cat ./struct.hb2 | tail -n+9 | awk '
{
	c1=substr($0,1,1); c1="c[" c1 "]";
	r1=substr($0,2,4); sub(/^0+/, "", r1); r1="r[" r1 "]";
	i1=substr($0,6,1); if(i1=="-") { i1="" } else { i1="i[" i1 "]" }
	rn1=substr($0,7,3); gsub(" ", "", rn1); rn1="rn[" rn1 "]";
	an1=substr($0,10,4); gsub(" ", "", an1); an1="an[" an1 "]";
	
	c2=substr($0,15,1); c2="c[" c2 "]";
	r2=substr($0,16,4); sub(/^0+/, "", r2); r2="r[" r2 "]";
	i2=substr($0,20,1); if(i2=="-") { i2="" } else { i2="i[" i2 "]" }
	rn2=substr($0,21,3); gsub(" ", "", rn2); rn2="rn[" rn2 "]";
	an2=substr($0,24,4); gsub(" ", "", an2); an2="an[" an2 "]";
	
	print c1 r1 i1 rn1 an1 " " c2 r2 i2 rn2 an2;
}'

rm -r $TMPDIR
