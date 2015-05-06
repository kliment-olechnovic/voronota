args=commandArgs(TRUE);

names=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
values=read.table(args[2], header=FALSE, stringsAsFactors=FALSE);

v=values$V1

targets=sort(union(names$V2, names$V2));

nonempty_targets=c();
max_scores=c();
target_scores=c();
mean_scores=c();
sd_of_scores=c();
sizes=c();
jumpers=c();
jumpernames=c();
solidtargetnames=c();
jumpertargetnames=c();

for(target in targets)
{
	sel=which(names$V2==target);
	if(length(sel)>1)
	{
		sn=names$V3[sel];
		sv=v[sel];
		stargetsel=which(sn=="target");
		if(length(stargetsel)>0)
		{
			nonempty_targets=c(nonempty_targets, target);
			
			jumpersel=which(sv>sv[stargetsel]);
			
			max_scores=c(max_scores, max(sv));
			target_scores=c(target_scores, sv[stargetsel]);
			mean_scores=c(mean_scores, mean(sv));
			sd_of_scores=c(sd_of_scores, sd(sv));
			
			sizes=c(sizes, length(sv));
			jumpers=c(jumpers, length(jumpersel));
			jumpernames=c(jumpernames, sn[jumpersel]);
			
			if(length(jumpersel)==0)
			{
				solidtargetnames=c(solidtargetnames, target);
			}
			else
			{
				jumpertargetnames=c(jumpertargetnames, target);
			}
			
			cols=rep("black", length(sel));
			cols[stargetsel]="blue";
			cols[jumpersel]="red";
			ordering=order(sv);
			plot(sv[ordering], col=cols[ordering], main=target);
		}
	}
}

solidtargetnames

jumpertargetnames;

length(which(jumpers>0));
mean(jumpers/sizes);
sum(jumpers)/sum(sizes);

result_table=data.frame(target=nonempty_targets, N=(sizes+1), rank=(jumpers+1), Z_score=format(((target_scores-mean_scores)/sd_of_scores), digits=2));
write.table(result_table, "./target_selection_results", row.names=FALSE, quote=FALSE);
