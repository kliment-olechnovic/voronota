args=commandArgs(TRUE);

input1="table1";
input2="table2";
column1="V1";
column2="V1";
mergingA=c();
mergingB=c();
mergingC=c();
output_image="";
table_header=FALSE;

for(i in 1:length(args))
{
	if(args[i]=="V-input1")
	{
		input1=args[i+1];
	}
	else if(args[i]=="V-input2")
	{
		input2=args[i+1];
	}
	else if(args[i]=="V-column1")
	{
		column1=args[i+1];
	}
	else if(args[i]=="V-column2")
	{
		column2=args[i+1];
	}
	else if(args[i]=="V-mergingA")
	{
		mergingA=args[i+1];
	}
	else if(args[i]=="V-mergingB")
	{
		mergingB=args[i+1];
	}
	else if(args[i]=="V-mergingC")
	{
		mergingC=args[i+1];
	}
	else if(args[i]=="V-output-image")
	{
		output_image=args[i+1];
	}
	else if(args[i]=="F-table-header")
	{
		table_header=TRUE;
	}
}

mergings=c(mergingA, mergingB, mergingC);

t1=read.table(input1, header=table_header, stringsAsFactors=FALSE);
t2=read.table(input2, header=table_header, stringsAsFactors=FALSE);

x=c();
y=c();
if(length(mergings)>0)
{
	t=merge(t1, t2, by=mergings);
	x=t[, column1];
	y=t[, column2];
} else {
	x=t1[, column1];
	y=t2[, column2];
}

if(output_image!="")
{
	png(output_image, height=10, width=10, units="in", res=300);

	plot(x, y, type="n", xlab=column1, ylab=column2, main=paste(column1, " vs ", column2, sep=""));
	points(x, y, col="black");
	points(c(-100, 100), c(-100, 100), type="l", col="yellow");
	points(c(-100, 100), c(0, 0), type="l", col="yellow");
	points(c(0, 0), c(-100, 100), type="l", col="yellow");

	dev.off();
}


### Results statistics output ###


cor(x, y);

quantile(x-y);

mean(x-y);

sd(x-y);
