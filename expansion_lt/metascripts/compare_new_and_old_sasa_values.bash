#!/bin/bash

readonly TMPLDIR="$(mktemp -d)"
trap "rm -r $TMPLDIR" EXIT

cat > "${TMPLDIR}/balls"

cat "${TMPLDIR}/balls" | voronota calculate-contacts --probe 1.4 | awk '{if($1==$2){print $1 " " $3}}' > "${TMPLDIR}/sasa1"

cat "${TMPLDIR}/balls" | ./voronota-lt -output-sasa -probe 1.4 | egrep '^sasa ' | awk '{print $2 " " $3}' > "${TMPLDIR}/sasa2"

head "${TMPLDIR}/sasa1"
echo
head "${TMPLDIR}/sasa2"

cd "$TMPLDIR"

R --vanilla << 'EOF'
dt1=read.table("sasa1", header=FALSE);
dt1$id=dt1$V1;
dt1$sasa1=dt1$V2;
dt2=read.table("sasa2", header=FALSE);
dt2$id=dt2$V1;
dt2$sasa2=dt2$V2;
dt=merge(dt1[,c("id","sasa1")], dt2[,c("id","sasa2")], by="id", all=TRUE);
dt$sasa1[which(!is.finite(dt$sasa1))]=0.0;
dt$sasa2[which(!is.finite(dt$sasa2))]=0.0;
dt$diffs=abs(dt$sasa1-dt$sasa2);
sel=rev(order(dt$diffs))[1:5];
dt[sel,];
png("plot.png");
plot(x=dt$sasa1, y=dt$sasa2, xlab="triangulated SASA", ylab="analytical SASA", main="triangulated vs analytical SASA");
dev.off();
EOF

display ./plot.png
