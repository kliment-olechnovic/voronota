args=commandArgs(TRUE);

t0=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
t1=read.table(args[2], header=FALSE, stringsAsFactors=FALSE);

t=merge(t0, t1, by=c("V1", "V2", "V3"));

length(t0[,1]);
length(t1[,1]);
length(t[,1]);

cols=rep("black", length(t[,1]));
cols[which(t$V3=="hb")]="blue";
cols[which(t$V3=="ds")]="purple";
cols[which(t$V2=="c<solvent>")]="red";

png(args[3],  height=10, width=10, units="in", res=300);

plot(t$V4.x, t$V4.y, col=cols);
points(c(-100, 100), c(-100, 100), type="l", col="yellow");
points(c(-100, 100), c(0, 0), type="l", col="yellow");
points(c(0, 0), c(-100, 100), type="l", col="yellow");

dev.off();
