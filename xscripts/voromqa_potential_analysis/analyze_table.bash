#!/bin/bash

R --vanilla << 'EOF'

t=read.table("table", header=TRUE, stringsAsFactors=FALSE);

t$value_c_nc=(t$value_central*t$area_central+t$value_noncentral*t$area_noncentral)/(t$area_central+t$area_noncentral);
t$area_c_nc=(t$area_central+t$area_noncentral);

ut=t[!duplicated(t[,"atom1"]),];
ut=data.frame(atom=ut$atom1, value_solvent=ut$value_solvent1, area_solvent=ut$area_solvent1);

ut$value_c_nc=rep(0, nrow(ut));
ut$area_c_nc=rep(0, nrow(ut));

sel_atom2_carbon=grep("A<C", t$atom2);
sel_atom2_sidechain=grep("A<C>|A<CA>|A<N>|A<O>", t$atom2, invert=TRUE);
sel_atom2_carbon_sidechain=intersect(sel_atom2_carbon, sel_atom2_sidechain);
sel_atom2_hydrophobic=intersect(sel_atom2_carbon_sidechain, grep("R<ILE>|R<LEU>", t$atom2));

for(i in 1:nrow(ut))
{
	sel_atom1_relevant=which(t$atom1==ut$atom[i]);
	
	sel=sel_atom1_relevant;
	ut$area_nonsolvent[i]=sum(t$area_c_nc[sel]);
	ut$value_nonsolvent[i]=sum(t$value_c_nc[sel]*t$area_c_nc[sel])/sum(t$area_c_nc[sel]);
	
	sel=intersect(sel_atom1_relevant, sel_atom2_carbon);
	ut$area_nonsolvent_carbon[i]=sum(t$area_c_nc[sel]);
	ut$value_nonsolvent_carbon[i]=sum(t$value_c_nc[sel]*t$area_c_nc[sel])/sum(t$area_c_nc[sel]);
	
	sel=intersect(sel_atom1_relevant, sel_atom2_sidechain);
	ut$area_nonsolvent_sidechain[i]=sum(t$area_c_nc[sel]);
	ut$value_nonsolvent_sidechain[i]=sum(t$value_c_nc[sel]*t$area_c_nc[sel])/sum(t$area_c_nc[sel]);
	
	sel=intersect(sel_atom1_relevant, sel_atom2_carbon_sidechain);
	ut$area_nonsolvent_carbon_sidechain[i]=sum(t$area_c_nc[sel]);
	ut$value_nonsolvent_carbon_sidechain[i]=sum(t$value_c_nc[sel]*t$area_c_nc[sel])/sum(t$area_c_nc[sel]);
	
	sel=intersect(sel_atom1_relevant, sel_atom2_hydrophobic);
	ut$area_nonsolvent_hydrophobic[i]=sum(t$area_c_nc[sel]);
	ut$value_nonsolvent_hydrophobic[i]=sum(t$value_c_nc[sel]*t$area_c_nc[sel])/sum(t$area_c_nc[sel]);
}

####################

sel=grep("A<C>|A<CA>|A<N>|A<O>", ut$atom);
sel_gly=intersect(sel, grep("R<GLY>", ut$atom));

plot(ut$value_solvent, ut$value_nonsolvent, main="solvent vs nonsolvent");
points(ut$value_solvent[sel], ut$value_nonsolvent[sel], col="red", pch=3);
points(ut$value_solvent[sel_gly], ut$value_nonsolvent[sel_gly], col="green", pch=4);
lines(c(-100, 100), c(100, -100));

plot(ut$value_solvent, ut$value_nonsolvent_carbon, main="solvent vs nonsolvent_carbon");
points(ut$value_solvent[sel], ut$value_nonsolvent_carbon[sel], col="red", pch=3);
points(ut$value_solvent[sel_gly], ut$value_nonsolvent_carbon[sel_gly], col="green", pch=4);
lines(c(-100, 100), c(100, -100));

plot(ut$value_solvent, ut$value_nonsolvent_sidechain, main="solvent vs nonsolvent_sidechain");
points(ut$value_solvent[sel], ut$value_nonsolvent_sidechain[sel], col="red", pch=3);
points(ut$value_solvent[sel_gly], ut$value_nonsolvent_sidechain[sel_gly], col="green", pch=4);
lines(c(-100, 100), c(100, -100));

plot(ut$value_solvent, ut$value_nonsolvent_carbon_sidechain, main="solvent vs nonsolvent_carbon_sidechain");
points(ut$value_solvent[sel], ut$value_nonsolvent_carbon_sidechain[sel], col="red", pch=3);
points(ut$value_solvent[sel_gly], ut$value_nonsolvent_carbon_sidechain[sel_gly], col="green", pch=4);
lines(c(-100, 100), c(100, -100));

plot(ut$value_solvent, ut$value_nonsolvent_hydrophobic, main="solvent vs nonsolvent_hydrophobic");
points(ut$value_solvent[sel], ut$value_nonsolvent_hydrophobic[sel], col="red", pch=3);
points(ut$value_solvent[sel_gly], ut$value_nonsolvent_hydrophobic[sel_gly], col="green", pch=4);
lines(c(-100, 100), c(100, -100));

####################

sel=grep("A<C>|A<CA>|A<N>|A<O>", ut$atom);

plot(ut$value_nonsolvent, ut$value_nonsolvent_carbon, main="nonsolvent vs nonsolvent_carbon");
points(ut$value_nonsolvent[sel], ut$value_nonsolvent_carbon[sel], col="red", pch=3);
lines(c(-100, 100), c(-100, 100));

plot(ut$value_nonsolvent, ut$value_nonsolvent_sidechain, main="nonsolvent vs nonsolvent_sidechain");
points(ut$value_nonsolvent[sel], ut$value_nonsolvent_sidechain[sel], col="red", pch=3);
lines(c(-100, 100), c(-100, 100));

plot(ut$value_nonsolvent, ut$value_nonsolvent_carbon_sidechain, main="nonsolvent vs nonsolvent_carbon_sidechain");
points(ut$value_nonsolvent[sel], ut$value_nonsolvent_carbon_sidechain[sel], col="red", pch=3);
lines(c(-100, 100), c(-100, 100));

plot(ut$value_nonsolvent, ut$value_nonsolvent_hydrophobic, main="nonsolvent vs nonsolvent_hydrophobic");
points(ut$value_nonsolvent[sel], ut$value_nonsolvent_hydrophobic[sel], col="red", pch=3);
lines(c(-100, 100), c(-100, 100));

plot(ut$value_nonsolvent_carbon, ut$value_nonsolvent_sidechain, main="nonsolvent_carbon vs nonsolvent_sidechain");
points(ut$value_nonsolvent_carbon[sel], ut$value_nonsolvent_sidechain[sel], col="red", pch=3);
lines(c(-100, 100), c(-100, 100));

plot(ut$value_nonsolvent_carbon, ut$value_nonsolvent_carbon_sidechain, main="nonsolvent_carbon vs nonsolvent_carbon_sidechain");
points(ut$value_nonsolvent_carbon[sel], ut$value_nonsolvent_carbon_sidechain[sel], col="red", pch=3);
lines(c(-100, 100), c(-100, 100));

plot(ut$value_nonsolvent_carbon, ut$value_nonsolvent_hydrophobic, main="nonsolvent_carbon vs nonsolvent_hydrophobic");
points(ut$value_nonsolvent_carbon[sel], ut$value_nonsolvent_hydrophobic[sel], col="red", pch=3);
lines(c(-100, 100), c(-100, 100));

plot(ut$value_nonsolvent_sidechain, ut$value_nonsolvent_carbon_sidechain, main="nonsolvent_sidechain vs nonsolvent_carbon_sidechain");
points(ut$value_nonsolvent_sidechain[sel], ut$value_nonsolvent_carbon_sidechain[sel], col="red", pch=3);
lines(c(-100, 100), c(-100, 100));

plot(ut$value_nonsolvent_sidechain, ut$value_nonsolvent_hydrophobic, main="nonsolvent_sidechain vs nonsolvent_hydrophobic");
points(ut$value_nonsolvent_sidechain[sel], ut$value_nonsolvent_hydrophobic[sel], col="red", pch=3);
lines(c(-100, 100), c(-100, 100));

plot(ut$value_nonsolvent_carbon_sidechain, ut$value_nonsolvent_hydrophobic, main="nonsolvent_carbon_sidechain vs nonsolvent_hydrophobic");
points(ut$value_nonsolvent_carbon_sidechain[sel], ut$value_nonsolvent_hydrophobic[sel], col="red", pch=3);
lines(c(-100, 100), c(-100, 100));

####################

EOF

