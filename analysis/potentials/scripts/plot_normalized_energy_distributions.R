args=commandArgs(TRUE);

t=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);

histbreaks=seq(min(t$V2)-0.05, max(t$V2)+0.05, 0.01);

hist(t$V2, breaks=histbreaks, col=rgb(1, 0, 0, 0.5), freq=FALSE, main="All atoms");

names=sort(union(t$V1, t$V1));

means=c();
sds=c();
for(name in names)
{
	st=t[which(t$V1==name),];
	means=c(means, mean(st$V2));
	sds=c(sds, sd(st$V2));
	hist(st$V2, breaks=histbreaks, col=rgb(0, 0, 1, 0.5), freq=FALSE, main=name);
}

mt=data.frame(names=names, means=means, sds=sds);
mt=mt[order(mt$means),];

write.table(mt, "means_and_sds", col.names=FALSE, row.names=FALSE, quote=FALSE);

plot(mt$means, mt$sds);

mean(t$V2);
quantile(mt$means);

sd(t$V2);
quantile(mt$sds);
