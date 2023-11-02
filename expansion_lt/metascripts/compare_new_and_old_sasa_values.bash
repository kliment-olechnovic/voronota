#!/bin/bash

readonly TMPLDIR="$(mktemp -d)"
trap "rm -r $TMPLDIR" EXIT

cat > "${TMPLDIR}/balls"

cat "${TMPLDIR}/balls" | voronota calculate-contacts | awk '{if($1==$2){print $1 " " $3}}' > "${TMPLDIR}/sasa1"

cat "${TMPLDIR}/balls" | ./voronota-lt -output-sasa | egrep '^sasa ' | awk '{print $2 " " $3}' > "${TMPLDIR}/sasa2"

head "${TMPLDIR}/sasa1"
echo
head "${TMPLDIR}/sasa2"

cd "$TMPLDIR"

R --vanilla << 'EOF'
dt1=read.table("sasa1", header=FALSE);
dt2=read.table("sasa2", header=FALSE);
dt=merge(dt1, dt2, by="V1");
dt$diffs=abs(dt$V2.x-dt$V2.y);
sel=rev(order(dt$diffs))[1:5];
dt[sel,];
png("plot.png");
plot(x=dt$V2.x, y=dt$V2.y, xlab="triangulated SASA", ylab="analytical SASA", main="triangulated vs analytical SASA");
dev.off();
EOF

display ./plot.png
