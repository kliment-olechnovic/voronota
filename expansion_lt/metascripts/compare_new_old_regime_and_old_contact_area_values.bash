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

cat "${TMPLDIR}/balls" | voronota calculate-contacts --probe 1.4 | awk '{if($1!=$2){print $1 " " $2 " " $3}}' > "${TMPLDIR}/areas1"

cat "${TMPLDIR}/balls" | ./voronota-lt -output-csa -probe 1.4 -old-regime | egrep '^csa ' | awk '{if($2<$3){print $2 " " $3 " " $4}else{print $3 " " $2 " " $4}}' > "${TMPLDIR}/areas2"

head "${TMPLDIR}/areas1"
echo
head "${TMPLDIR}/areas2"

cd "$TMPLDIR"

R --vanilla << 'EOF'
dt1=read.table("areas1", header=FALSE);
dt1$id_a=dt1$V1;
dt1$id_b=dt1$V2;
dt1$area1=dt1$V3;
dt2=read.table("areas2", header=FALSE);
dt2$id_a=dt2$V1;
dt2$id_b=dt2$V2;
dt2$area2=dt2$V3;
dt=merge(dt1[,c("id_a","id_b","area1")], dt2[,c("id_a","id_b","area2")], by=c("id_a","id_b"), all=TRUE);
dt$area1[which(!is.finite(dt$area1))]=0.0;
dt$area2[which(!is.finite(dt$area2))]=0.0;
dt$diffs=abs(dt$area1-dt$area2);
sel=rev(order(dt$diffs))[1:5];
dt[sel,];
png("plot.png", width=800, height=800);
plot(x=dt$area1, y=dt$area2, xlab="area AW", ylab="area AW-neimpl", main="AW vs AW-neimpl areas", col=densCols(dt$area1, dt$area2));
points(x=c(-1000, 1000), y=c(-1000, 1000), type="l");
dev.off();
EOF

display ./plot.png
