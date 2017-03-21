window=50;

t=read.table("scores", header=TRUE, stringsAsFactors=FALSE);

rs=sort(union(t$R, t$R));

N=length(rs);

v_05=rep(0, N);
v_50=rep(0, N);
v_95=rep(0, N);
i=1;
while(i<=N)
{
	buf=c();
	j=(0-window);
	while(j<=window)
	{
		p=(i+j);
		if(p>=1 & p<=N)
		{
			buf=c(buf, t$V[which(t$R==rs[p])]);
		}
		j=j+1;
	}
	v=as.numeric(quantile(buf, p=c(0.05, 0.50, 0.95)));
	v_05[i]=v[1];
	v_50[i]=v[2];
	v_95[i]=v[3];
	i=i+1;
}

sv_05=smooth.spline(x=rs, y=v_05, spar=1);
sv_50=smooth.spline(x=rs, y=v_50, spar=1);
sv_95=smooth.spline(x=rs, y=v_95, spar=1);

fv_05=approxfun(sv_05$x, sv_05$y, rule=1:2);
fv_50=approxfun(sv_50$x, sv_50$y, rule=1:2);
fv_95=approxfun(sv_95$x, sv_95$y, rule=1:2);

plot(x=rs, y=v_50, col="green", type="l", ylim=c(0, 1));
points(x=rs, y=v_05, col="red", type="l");
points(x=rs, y=v_95, col="blue", type="l");

points(x=sv_50$x, y=sv_50$y, col="green", ylim=c(0, 1), type="l", lwd=2);
points(x=sv_05$x, y=sv_05$y, col="red", type="l", lwd=2);
points(x=sv_95$x, y=sv_95$y, col="blue", type="l", lwd=2);

fx=seq(50, 1000, 50);
points(x=fx, y=fv_50(fx), col="green");
points(x=fx, y=fv_05(fx), col="red");
points(x=fx, y=fv_95(fx), col="blue");

points(t$R, t$V, cex=0.5);

result=data.frame(length=fx, quantile05=fv_05(fx), quantile50=fv_50(fx), quantile95=fv_95(fx));
write.table(format(result, digits=3), "quantiles_by_length", quote=FALSE, col.names=TRUE, row.names=FALSE);
