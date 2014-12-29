scolors=c(rgb(1, 0, 0, 1), rgb(0, 1, 0, 1), rgb(0, 0, 1, 1), rgb(0, 1, 1, 1), rgb(1, 0, 1, 1), rgb(1, 1, 0, 1));

args=commandArgs(TRUE);

t1=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
s1=args[2];
t2=read.table(args[3], header=FALSE, stringsAsFactors=FALSE);
flags=args[grep("flag_", args)];

xylims=c(min(t1$V4, t2$V4), max(t1$V4, t2$V4));

st1=t1[which(t1$V3==s1),];

wholesel=1:length(st1$V1);
finalsel=c();
if(length(flags)==0)
{
	finalsel=wholesel;
} else {
	mcsel1=c();
	mcsel1=union(mcsel1, grep("A<C>", st1$V1));
	mcsel1=union(mcsel1, grep("A<CA>", st1$V1));
	mcsel1=union(mcsel1, grep("A<N>", st1$V1));
	mcsel1=union(mcsel1, grep("A<O>", st1$V1));
	mcsel2=c();
	mcsel2=union(mcsel2, grep("A<C>", st1$V2));
	mcsel2=union(mcsel2, grep("A<CA>", st1$V2));
	mcsel2=union(mcsel2, grep("A<N>", st1$V2));
	mcsel2=union(mcsel2, grep("A<O>", st1$V2));
	mcsel=intersect(mcsel1, mcsel2);
	scsel=intersect(setdiff(wholesel, mcsel1), setdiff(wholesel, mcsel2));
	mixsel=intersect(setdiff(wholesel, mcsel), setdiff(wholesel, scsel));
	if(is.element("flag_solvent", flags))
	{
		finalsel=union(finalsel, which(st1$V2=="c<solvent>"));
	}
	if(is.element("flag_mainchain", flags))
	{
		finalsel=union(finalsel, mcsel);
	}
	if(is.element("flag_sidechain", flags))
	{
		finalsel=union(finalsel, scsel);
	}
	if(is.element("flag_mixchain", flags))
	{
		finalsel=union(finalsel, mixsel);
	}
}
st1=st1[finalsel,];

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
