#!/bin/bash

readonly TMPLDIR="$(mktemp -d)"
trap "rm -r $TMPLDIR" EXIT

cat > "${TMPLDIR}/balls"

UNIVERSALRADIUS="$1"
RADIUSCOMBOCOEF="$2"

if [ -n "$UNIVERSALRADIUS" ]
then
	if [ -z "$RADIUSCOMBOCOEF" ]
	then
		cat "${TMPLDIR}/balls" | awk -v radius="$UNIVERSALRADIUS" '{print $1 " " $2 " " $3 " " radius}' | sponge "${TMPLDIR}/balls"
	else
		cat "${TMPLDIR}/balls" | awk -v radius="$UNIVERSALRADIUS" -v combocoef="$RADIUSCOMBOCOEF" '{print $1 " " $2 " " $3 " " ((combocoef*$4)+((1-combocoef)*radius))}' | sponge "${TMPLDIR}/balls"
	fi
fi

cat "${TMPLDIR}/balls" | ./voronota-lt -probe 1.4 -print-contacts > "${TMPLDIR}/areas_rt"

cat "${TMPLDIR}/balls" | ./voronota-lt -probe 1.4 -print-contacts -run-in-aw-diagram-regime > "${TMPLDIR}/areas_aw"

echo
head "${TMPLDIR}/areas_rt"
echo
head "${TMPLDIR}/areas_aw"
echo

cd "$TMPLDIR"

R --vanilla << 'EOF'
dt1=read.table("areas_aw", header=TRUE);
dt2=read.table("areas_rt", header=TRUE);
dt=merge(dt1, dt2, by=c("ID1_chain","ID1_residue","ID1_atom","ID2_chain","ID2_residue","ID2_atom"), all=TRUE);
#dt=dt[which(diff(dt$ID1_residue-dt$ID2_residue)>1 | dt$ID1_chain!=dt$ID2_chain),];
dt$area.x[which(!is.finite(dt$area.x))]=0.0;
dt$area.y[which(!is.finite(dt$area.y))]=0.0;
dt$diffs=abs(dt$area.x-dt$area.y);
sel=rev(order(dt$diffs))[1:5];
dt[sel,];
png("plot.png", width=800, height=800);
plot(x=dt$area.x, y=dt$area.y, xlab="area AW", ylab="area RT", main="AW vs RT atom-level areas", col=densCols(dt$area.x, dt$area.y));
points(x=c(-1000, 1000), y=c(-1000, 1000), type="l");
dev.off();
EOF

display ./plot.png
