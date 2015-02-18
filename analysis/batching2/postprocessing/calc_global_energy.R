args=commandArgs(TRUE);

t1=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
t2=read.table(args[2], header=FALSE, stringsAsFactors=FALSE);
t=merge(t1, t2, by=c("V1", "V2", "V3"));

energy=sum(t$V4.x*t$V4.y);
area=sum(t$V4.x);

energy
area

energy/area;
