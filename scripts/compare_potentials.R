args=commandArgs(TRUE);
ifile1=args[1];
ifile2=args[2];

t1=read.table(ifile1, header=FALSE, stringsAsFactors=FALSE);
t2=read.table(ifile2, header=FALSE, stringsAsFactors=FALSE);

t=merge(data.frame(a=t1$V1, b=t1$V2, v1=t1$V3), data.frame(a=t2$V1, b=t2$V2, v2=t2$V3));

plot(t$v1, t$v2, main=paste(ifile1, "vs", ifile2));
points(c(-100, 100), c(-100, 100), type="l");
