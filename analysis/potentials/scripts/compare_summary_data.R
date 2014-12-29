scolors=c(rgb(1, 0, 0, 1), rgb(0, 1, 0, 1), rgb(0, 0, 1, 1), rgb(0, 1, 1, 1), rgb(1, 0, 1, 1), rgb(1, 1, 0, 1));

args=commandArgs(TRUE);

t1=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
s1=args[2];
t2=read.table(args[3], header=FALSE, stringsAsFactors=FALSE);
flag=args[4];

xylims=c(min(t1$V4, t2$V4), max(t1$V4, t2$V4));

st1=t1[which(t1$V3==s1),];
if(flag=="solvent")
{
	st1=st1[which(st1$V2=="c<solvent>"),];
}
df1=data.frame(a=st1$V1, b=st1$V2, c1=st1$V4);

sset2=union(t2$V3, t2$V3);

s1;
sset2;

colorid=1;
png("plot.png", width=900, height=900);
plot(x=xylims, y=xylims, type="l", col="black", xlab="v1", ylab="v2", main="");
for(s2 in sset2)
{
	st2=t2[which(t2$V3==s2),];
	df2=data.frame(a=st2$V1, b=st2$V2, c2=st2$V4);
	df=merge(df1, df2);
	points(df$c1, df$c2, col=scolors[colorid], cex=0.3);
	colorid=min(length(scolors), colorid+1);
}
dev.off();

colorid=1;
for(s2 in sset2)
{
	st2=t2[which(t2$V3==s2),];
	df2=data.frame(a=st2$V1, b=st2$V2, c2=st2$V4);
	df=merge(df1, df2);
	plot(x=xylims, y=xylims, type="l", col="black", xlab="v1", ylab="v2", main=paste(s1, "vs", s2));
	points(df$c1, df$c2, col=scolors[colorid], cex=0.3);
	colorid=min(length(scolors), colorid+1);
}
