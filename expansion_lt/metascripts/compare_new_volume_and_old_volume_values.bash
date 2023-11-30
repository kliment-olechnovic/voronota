#!/bin/bash

readonly TMPLDIR="$(mktemp -d)"
trap "rm -r $TMPLDIR" EXIT

cat > "${TMPLDIR}/balls"

cat "${TMPLDIR}/balls" | voronota calculate-contacts --probe 1.4 --volumes-output "${TMPLDIR}/old_volumes" > /dev/null

{
echo "ID_index volume"
cat "${TMPLDIR}/old_volumes"
} \
> "${TMPLDIR}/volumes1"

cat "${TMPLDIR}/balls" | ./voronota-lt -probe 1.4 -print-cells > "${TMPLDIR}/volumes2"

head "${TMPLDIR}/volumes1"
echo
head "${TMPLDIR}/volumes2"

cd "$TMPLDIR"

R --vanilla << 'EOF'
dt1=read.table("volumes1", header=TRUE);
dt2=read.table("volumes2", header=TRUE);
dt=merge(dt1, dt2, by="ID_index", all=TRUE);
dt$volume.x[which(!is.finite(dt$volume.x))]=0.0;
dt$volume.y[which(!is.finite(dt$volume.y))]=0.0;
dt$diffs=abs(dt$volume.x-dt$volume.y);
sel=rev(order(dt$diffs))[1:5];
dt[sel,];
png("plot.png", width=800, height=800);
plot(x=dt$volume.x, y=dt$volume.y, xlab="old volume", ylab="new volume", main="old volume vs new volume", col=densCols(dt$volume.x, dt$volume.y));
points(x=c(-1000, 1000), y=c(-1000, 1000), type="l");
dev.off();
EOF

display ./plot.png
