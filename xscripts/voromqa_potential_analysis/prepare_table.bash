#!/bin/bash

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

cp ../../resources/voromqa_v1_energy_potential ./potential

cat ../../resources/voromqa_v1_energy_potential_source/contact_areas \
| sed 's/central_//g' \
| sed 's/sep1/interatom/g' \
| sed 's/sep2/interatom/g' \
| ../../voronota score-contacts-potential \
> ./contact_areas

cat ./contact_areas \
| ../../voronota score-contacts-potential \
  --probabilities-file ./probabilities \
  --potential-file ./potential_raw \
> /dev/null

{
	echo "atom1 value_solvent1"
	cat ./potential | grep 'solvent' | awk '{print $1 " " $4}'
} > "$TMPLDIR/table_solvent1"

{
	echo "atom2 value_solvent2"
	cat ./potential | grep 'solvent' | awk '{print $1 " " $4}'
} > "$TMPLDIR/table_solvent2"

{
	echo "atom1 atom2 area"
	{
		cat ./contact_areas | awk '{if($3=="interatom"){print $1 " " $2 " " $4}}'
		cat ./contact_areas | awk '{if($3=="interatom"){print $2 " " $1 " " $4}}'
	} | sort | uniq
} > "$TMPLDIR/table_area"

{
	echo "atom1 area_solvent1"
	cat ./contact_areas | grep 'solvent' | awk '{print $1 " " $4}'
} > "$TMPLDIR/table_area_solvent1"

{
	echo "atom2 area_solvent2"
	cat ./contact_areas | grep 'solvent' | awk '{print $1 " " $4}'
} > "$TMPLDIR/table_area_solvent2"

{
	echo "atom1 atom2 prob_obs prob_exp"
	{
		cat ./probabilities | awk '{if($3=="interatom"){print $1 " " $2 " " $4 " " $5}}'
		cat ./probabilities | awk '{if($3=="interatom"){print $2 " " $1 " " $4 " " $5}}'
	} | sort | uniq
} > "$TMPLDIR/table_prob"

cd "$TMPLDIR"

R --vanilla << 'EOF'
table_prob=read.table("table_prob", header=TRUE, stringsAsFactors=FALSE);
table_area=read.table("table_area", header=TRUE, stringsAsFactors=FALSE);
table_solvent1=read.table("table_solvent1", header=TRUE, stringsAsFactors=FALSE);
table_solvent2=read.table("table_solvent2", header=TRUE, stringsAsFactors=FALSE);
table_area_solvent1=read.table("table_area_solvent1", header=TRUE, stringsAsFactors=FALSE);
table_area_solvent2=read.table("table_area_solvent2", header=TRUE, stringsAsFactors=FALSE);
result=merge(table_prob, table_area);
result=merge(result, merge(table_solvent1, table_solvent2));
result=merge(result, merge(table_area_solvent1, table_area_solvent2));
write.table(result, "result", quote=FALSE, col.names=TRUE, row.names=FALSE);
EOF

cd - &> /dev/null

cat "$TMPLDIR/result" | column -t > ./table
