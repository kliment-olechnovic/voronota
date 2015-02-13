args=commandArgs(TRUE);

t1=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
t2=read.table(args[2], header=FALSE, stringsAsFactors=FALSE);
t=merge(t1, t2, by=c("V1", "V2", "V3"));
tp=merge(t1[which(t1$V3=="."),], t2[which(t2$V3=="."),], by=c("V1", "V2", "V3"));
tw=merge(t1[which(t1$V2=="c<solvent>"),], t2[which(t2$V2=="c<solvent>"),], by=c("V1", "V2", "V3"));

plot(t$V4.x, t$V4.y);
points(tp$V4.x, tp$V4.y, col="blue");
points(tw$V4.x, tw$V4.y, col="red");
points(c(-100, 100), c(-100, 100), type="l", col="yellow");
points(c(-100, 100), c(0, 0), type="l", col="yellow");
points(c(0, 0), c(-100, 100), type="l", col="yellow");
