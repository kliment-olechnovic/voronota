#!/bin/bash

readonly TMPLDIR="$(mktemp -d)"
trap "rm -r $TMPLDIR" EXIT

seq 1000 1000 100000 \
| while read -r HEADCUT
do
	INNAME="balls_${HEADCUT}"
	INFILE="${TMPLDIR}/${INNAME}"
	
	cat "./tests/input/balls_7br8.xyzr" | head -n "$HEADCUT" > "$INFILE"
	
	BALLS_NUM="$(cat ${INFILE} | wc -l)"
	
	time -p (cat "$INFILE" | ./voronota-lt -processors 20 -probe 1.4 -run-in-aw-diagram-regime > /dev/null) 2> "${TMPLDIR}/${INNAME}.time_new_old"
	TIME_NEW_OLD="$(cat ${TMPLDIR}/${INNAME}.time_new_old | egrep '^real ' | awk '{print $2}')"
	
	time -p (cat "$INFILE" | ./voronota-lt -processors 20 -probe 1.4 > /dev/null) 2> "${TMPLDIR}/${INNAME}.time_new"
	TIME_NEW="$(cat ${TMPLDIR}/${INNAME}.time_new | egrep '^real ' | awk '{print $2}')"
	
	echo "$BALLS_NUM $TIME_NEW_OLD $TIME_NEW"
done \
> "${TMPLDIR}/table"

{
echo "balls_num time_new_old time_new"
cat "${TMPLDIR}/table"
} \
| sponge "${TMPLDIR}/table"

cat "${TMPLDIR}/table" | column -t

cd "$TMPLDIR"

R --vanilla > /dev/null << 'EOF'
dt=read.table("table", header=TRUE);

png("plot.png", units="in", width=10, height=10, res=150);

plot(x=dt$balls_num, y=dt$time_new_old, col="green", ylim=c(0, max(dt$time_new_old)), xlab="num of balls", ylab="seconds", main="num of balls vs 20-threaded running seconds");
points(x=dt$balls_num, y=dt$time_new_old, col="green", type="l");

points(x=dt$balls_num, y=dt$time_new, col="blue");
points(x=dt$balls_num, y=dt$time_new, col="blue", type="l");

legend("topleft", legend=c("new old-style", "new"), col=c("green", "blue"), lty=c(1, 1), cex=1.0);

dev.off();

EOF

cd - &> /dev/null

cp "${TMPLDIR}/plot.png" "./times_new_parallel.png"

