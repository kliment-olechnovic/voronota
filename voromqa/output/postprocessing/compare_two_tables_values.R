args=commandArgs(TRUE);

input1="table1";
input2="table2";
filter1_name="";
filter1_value="";
filter2_name="";
filter2_value="";
column1="V1";
column2="V1";
sds_column="";
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
	else if(args[i]=="V-filter1")
	{
		filter1_name=args[i+1];
		filter1_value=args[i+2];
	}
	else if(args[i]=="V-filter2")
	{
		filter2_name=args[i+1];
		filter2_value=args[i+2];
	}
	else if(args[i]=="V-column1")
	{
		column1=args[i+1];
	}
	else if(args[i]=="V-column2")
	{
		column2=args[i+1];
	}
	else if(args[i]=="V-sds-column")
	{
		sds_column=args[i+1];
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
if(filter1_name!="")
{
	t1=t1[which(t1[, filter1_name]==filter1_value),];
}

t2=read.table(input2, header=table_header, stringsAsFactors=FALSE);
if(filter2_name!="")
{
	t2=t2[which(t2[, filter2_name]==filter2_value),];
}

x=c();
y=c();
sds=c();
t=t1;
if(length(mergings)>0)
{
	t=merge(t1, t2, by=mergings);
	x=t[, column1];
	y=t[, column2];
	if(sds_column!="" & is.element(sds_column, colnames(t)))
	{
		sds=t[, sds_column];
	}
} else {
	x=t1[, column1];
	y=t2[, column2];
	if(sds_column!="" & is.element(sds_column, colnames(t2)))
	{
		sds=t2[, sds_column];
	}
}

if(output_image!="")
{
	png(output_image, height=10, width=10, units="in", res=300);

	plot(x, y, type="n", xlab=column1, ylab=column2, main=paste(column1, " vs ", column2, sep=""));
	if(length(sds)>0) { segments(x, y-sds, x, y+sds, col="red"); }
	points(x, y, col="black");
	points(c(-100, 100), c(-100, 100), type="l", col="yellow");
	points(c(-100, 100), c(0, 0), type="l", col="yellow");
	points(c(0, 0), c(-100, 100), type="l", col="yellow");

	dev.off();
}

t[rev(order(abs(x-y)))[1:5],]

cor(x, y);

quantile(x-y);

mean(x-y);

sd(x-y);
