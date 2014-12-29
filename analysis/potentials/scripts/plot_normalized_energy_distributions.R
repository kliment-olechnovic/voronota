args=commandArgs(TRUE);

t=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);

histbreaks=seq(min(t$V2)-0.05, max(t$V2)+0.05, 0.01);

hist(t$V2, breaks=histbreaks, col=rgb(1, 0, 0, 0.5), freq=FALSE, main="All atoms");

names=sort(union(t$V1, t$V1));

means=c();
for(name in names)
{
	st=t[which(t$V1==name),];
	means=c(means, mean(st$V2));
	hist(st$V2, breaks=histbreaks, col=rgb(0, 0, 1, 0.5), freq=FALSE, main=name);
}

mean(t$V2);
quantile(means);

names[which(means==min(means))];
names[which(means==max(means))];
