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

#####################################

mean_addition=0.3;

mean_homo=mean(t$V2);
sd_homo=sd(t$V2);

scores_homo=rep(0, length(t$V2));
scores_hetero=scores_homo;
for(name in names)
{
	sel=which(t$V1==name);
	scores_homo[sel]=(1-pnorm((t$V2[sel]-(mean_homo+mean_addition))/sd_homo, mean=0, sd=1));
	mt_hetero=mt[which(mt$names==name),];
	scores_hetero[sel]=(1-pnorm((t$V2[sel]-(mt_hetero$means[1]+mean_addition))/mt_hetero$sds[1], mean=0, sd=1));
}

quantile(scores_homo);
quantile(scores_hetero);

histbreaks=seq(min(c(scores_homo, scores_hetero))-0.05, max(c(scores_homo, scores_hetero))+0.05, 0.02);
hist(scores_hetero, breaks=histbreaks, col=rgb(0, 0, 1, 0.5), freq=FALSE);
hist(scores_homo, breaks=histbreaks, col=rgb(1, 0, 0, 0.5), freq=FALSE, add=TRUE);

sample_sel=sample(1:length(scores_homo), 30000);
plot(x=scores_homo[sample_sel], y=scores_hetero[sample_sel], cex=0.2, xlim=c(0, 1), ylim=c(0, 1));
