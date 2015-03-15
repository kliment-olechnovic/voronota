args=commandArgs(TRUE);

t=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);

s2=t[which(t$V3=="sep2"),];
s3=t[which(t$V3=="sep3"),];
s4=t[which(t$V3=="sep4"),];
s5=t[which(t$V3=="."),];

d2=data.frame(a=s2$V1, b=s2$V2, sep2=s2$V4);
d3=data.frame(a=s3$V1, b=s3$V2, sep3=s3$V4);
d4=data.frame(a=s4$V1, b=s4$V2, sep4=s4$V4);
r=data.frame(a=s5$V1, b=s5$V2, sep5plus=s5$V4);

r=merge(r, d2, by=c("a", "b"), all.x=TRUE);
r$sep2[which(is.na(r$sep2))]=0;

r=merge(r, d3, by=c("a", "b"), all.x=TRUE);
r$sep3[which(is.na(r$sep3))]=0;

r=merge(r, d4, by=c("a", "b"), all.x=TRUE);
r$sep4[which(is.na(r$sep4))]=0;

r=r[which(r$b!="c<solvent>"),];

r$far_ratio=r$sep5plus/(r$sep5plus+r$sep2+r$sep3+r$sep4);
r=r[order(r$far_ratio),];

global_far_ratio=sum(r$sep5plus)/sum(r$sep5plus+r$sep2+r$sep3+r$sep4);

quantile(r$far_ratio, p=seq(0, 0.1, 0.01));

global_far_ratio;

write.table(r, args[2], col.names=FALSE, row.names=FALSE, quote=FALSE);

plot(density(r$far_ratio));
