#!/bin/bash

readonly TMPLDIR="$(mktemp -d)"
trap "rm -r $TMPLDIR" EXIT

cat > "${TMPLDIR}/balls"

{
echo "ID_index sas_area"
cat "${TMPLDIR}/balls" | voronota calculate-contacts --probe 1.4 | awk '{if($1==$2){print $1 " " $3}}'
} \
> "${TMPLDIR}/areas1"

cat "${TMPLDIR}/balls" | ./voronota-lt -probe 1.4 -print-sas-and-volumes > "${TMPLDIR}/areas2"

head "${TMPLDIR}/areas1"
echo
head "${TMPLDIR}/areas2"

cd "$TMPLDIR"

R --vanilla << 'EOF'
dt1=read.table("areas1", header=TRUE);
dt2=read.table("areas2", header=TRUE);
dt=merge(dt1, dt2, by="ID_index", all=TRUE);
dt$sas_area.x[which(!is.finite(dt$sas_area.x))]=0.0;
dt$sas_area.y[which(!is.finite(dt$sas_area.y))]=0.0;
dt$diffs=abs(dt$sas_area.x-dt$sas_area.y);
sel=rev(order(dt$diffs))[1:5];
dt[sel,];
png("plot.png", width=800, height=800);
plot(x=dt$sas_area.x, y=dt$sas_area.y, xlab="old SASA", ylab="new SASA", main="old SASA vs new SASA", col=densCols(dt$sas_area.x, dt$sas_area.y));
points(x=c(-1000, 1000), y=c(-1000, 1000), type="l");
dev.off();
EOF

display ./plot.png
