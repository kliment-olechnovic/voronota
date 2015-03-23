args=commandArgs(TRUE);

names=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
values=read.table(args[2], header=FALSE, stringsAsFactors=FALSE);

v=values$V1

targets=sort(union(names$V2, names$V2));

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
		jumpersel=which(sv>sv[stargetsel]);
		
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

solidtargetnames

jumpertargetnames;

length(which(jumpers>0));
mean(jumpers/sizes);
sum(jumpers)/sum(sizes);
