args=commandArgs(TRUE);

t0=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
t1=read.table(args[2], header=FALSE, stringsAsFactors=FALSE);

png(args[3],  height=10, width=10, units="in", res=300);

plot(t0$V2, t1$V2, cex=0.5);
points(c(-100, 100), c(-100, 100), type="l", col="yellow");
points(c(-100, 100), c(0, 0), type="l", col="yellow");
points(c(0, 0), c(-100, 100), type="l", col="yellow");

dev.off();
