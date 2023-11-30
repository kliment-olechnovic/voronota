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

{
echo "ID1_index ID2_index area"
cat "${TMPLDIR}/balls" | voronota calculate-contacts --probe 1.4 | awk '{if($1!=$2){print $1 " " $2 " " $3}}'
} \
> "${TMPLDIR}/areas1"

cat "${TMPLDIR}/balls" | ./voronota-lt -probe 1.4 -print-contacts -run-in-aw-diagram-regime > "${TMPLDIR}/areas2"

head "${TMPLDIR}/areas1"
echo
head "${TMPLDIR}/areas2"

cd "$TMPLDIR"

R --vanilla << 'EOF'
dt1=read.table("areas1", header=TRUE);
dt2=read.table("areas2", header=TRUE);
dt=merge(dt1, dt2, by=c("ID1_index", "ID2_index"), all=TRUE);
dt$area.x[which(!is.finite(dt$area.x))]=0.0;
dt$area.y[which(!is.finite(dt$area.y))]=0.0;
dt$diffs=abs(dt$area.x-dt$area.y);
sel=rev(order(dt$diffs))[1:5];
dt[sel,];
png("plot.png", width=800, height=800);
plot(x=dt$area.x, y=dt$area.y, xlab="old AW area", ylab="new AW area", main="old AW vs new AW areas", col=densCols(dt$area.x, dt$area.y));
points(x=c(-1000, 1000), y=c(-1000, 1000), type="l");
dev.off();
EOF

display ./plot.png
