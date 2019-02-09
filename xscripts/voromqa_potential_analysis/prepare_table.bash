#!/bin/bash

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

{
	echo "atom1 atom2 value_central"
	{
		cat ../../resources/voromqa_v1_energy_potential | awk '{if($3=="central_sep2"){print $1 " " $2 " " $4}}'
		cat ../../resources/voromqa_v1_energy_potential | awk '{if($3=="central_sep2"){print $2 " " $1 " " $4}}'
	} | sort | uniq
} > "$TMPLDIR/table_central"

{
	echo "atom1 atom2 value_noncentral"
	{
		cat ../../resources/voromqa_v1_energy_potential | awk '{if($3=="sep2"){print $1 " " $2 " " $4}}'
		cat ../../resources/voromqa_v1_energy_potential | awk '{if($3=="sep2"){print $2 " " $1 " " $4}}'
	} | sort | uniq
} > "$TMPLDIR/table_noncentral"

{
	echo "atom1 value_solvent1"
	cat ../../resources/voromqa_v1_energy_potential | grep 'solvent' | awk '{print $1 " " $4}'
} > "$TMPLDIR/table_solvent1"

{
	echo "atom2 value_solvent2"
	cat ../../resources/voromqa_v1_energy_potential | grep 'solvent' | awk '{print $1 " " $4}'
} > "$TMPLDIR/table_solvent2"

{
	echo "atom1 atom2 area_central"
	{
		cat ../../resources/voromqa_v1_energy_potential_source/contact_areas | awk '{if($3=="central_sep2"){print $1 " " $2 " " $4}}'
		cat ../../resources/voromqa_v1_energy_potential_source/contact_areas | awk '{if($3=="central_sep2"){print $2 " " $1 " " $4}}'
	} | sort | uniq
} > "$TMPLDIR/table_area_central"

{
	echo "atom1 atom2 area_noncentral"
	{
		cat ../../resources/voromqa_v1_energy_potential_source/contact_areas | awk '{if($3=="sep2"){print $1 " " $2 " " $4}}'
		cat ../../resources/voromqa_v1_energy_potential_source/contact_areas | awk '{if($3=="sep2"){print $2 " " $1 " " $4}}'
	} | sort | uniq
} > "$TMPLDIR/table_area_noncentral"

{
	echo "atom1 area_solvent1"
	cat ../../resources/voromqa_v1_energy_potential_source/contact_areas | grep 'solvent' | awk '{print $1 " " $4}'
} > "$TMPLDIR/table_area_solvent1"

{
	echo "atom2 area_solvent2"
	cat ../../resources/voromqa_v1_energy_potential_source/contact_areas | grep 'solvent' | awk '{print $1 " " $4}'
} > "$TMPLDIR/table_area_solvent2"

cat ../../resources/voromqa_v1_energy_potential_source/contact_areas \
| ../../voronota score-contacts-potential \
  --input-contributions ../../resources/voromqa_v1_energy_potential_source/contact_categories \
  --probabilities-file ./probabilities \
> /dev/null

{
	echo "atom1 atom2 prob_obs_central prob_exp_central"
	{
		cat ./probabilities | awk '{if($3=="central_sep2"){print $1 " " $2 " " $4 " " $5}}'
		cat ./probabilities | awk '{if($3=="central_sep2"){print $2 " " $1 " " $4 " " $5}}'
	} | sort | uniq
} > "$TMPLDIR/table_prob_central"

{
	echo "atom1 atom2 prob_obs_noncentral prob_exp_noncentral"
	{
		cat ./probabilities | awk '{if($3=="sep2"){print $1 " " $2 " " $4 " " $5}}'
		cat ./probabilities | awk '{if($3=="sep2"){print $2 " " $1 " " $4 " " $5}}'
	} | sort | uniq
} > "$TMPLDIR/table_prob_noncentral"

cd "$TMPLDIR"

R --vanilla << 'EOF'
table_central=read.table("table_central", header=TRUE, stringsAsFactors=FALSE);
table_noncentral=read.table("table_noncentral", header=TRUE, stringsAsFactors=FALSE);
table_solvent1=read.table("table_solvent1", header=TRUE, stringsAsFactors=FALSE);
table_solvent2=read.table("table_solvent2", header=TRUE, stringsAsFactors=FALSE);
table_area_central=read.table("table_area_central", header=TRUE, stringsAsFactors=FALSE);
table_area_noncentral=read.table("table_area_noncentral", header=TRUE, stringsAsFactors=FALSE);
table_area_solvent1=read.table("table_area_solvent1", header=TRUE, stringsAsFactors=FALSE);
table_area_solvent2=read.table("table_area_solvent2", header=TRUE, stringsAsFactors=FALSE);
table_prob_central=read.table("table_prob_central", header=TRUE, stringsAsFactors=FALSE);
table_prob_noncentral=read.table("table_prob_noncentral", header=TRUE, stringsAsFactors=FALSE);
result=merge(table_central, table_noncentral);
result=merge(result, merge(table_solvent1, table_solvent2));
result=merge(result, merge(table_area_central, table_area_noncentral));
result=merge(result, merge(table_area_solvent1, table_area_solvent2));
result=merge(result, merge(table_prob_central, table_prob_noncentral));
write.table(result, "result", quote=FALSE, col.names=TRUE, row.names=FALSE);
EOF

cd - &> /dev/null

cat "$TMPLDIR/result" | column -t > ./table
