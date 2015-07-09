args=commandArgs(TRUE);
t=read.table(args[1], header=TRUE, stringsAsFactors=FALSE);

percentiles=c(c(0.0, 0.01), seq(0.05, 0.5, 0.05));

min_atomscount_values=c();
cor_values=c();
for(percentile in percentiles)
{
	min_atomscount=as.numeric(quantile(t$atomscount, p=percentile));
	min_atomscount_values=c(min_atomscount_values, min_atomscount);
	sel=which(t$atomscount>=min_atomscount);
	cor_values=c(cor_values, cor(t$atomscount[sel], t$qscore_atom[sel]));
}
data.frame(lower_limit_percentile=percentiles, lower_limit_atomscount=min_atomscount_values, cor_atomscount_vs_score=cor_values);

min_score_values=c();
cor_values=c();
for(percentile in percentiles)
{
	min_score_value=as.numeric(quantile(t$qscore_atom, p=percentile));
	min_score_values=c(min_score_values, min_score_value);
	sel=which(t$qscore_atom>=min_score_value);
	cor_values=c(cor_values, cor(t$atomscount[sel], t$qscore_atom[sel]));
}
data.frame(lower_limit_percentile=percentiles, lower_limit_score=min_score_values, cor_atomscount_vs_score=cor_values);

mean(t$qscore_atom);
sd(t$qscore_atom);
quantile(t$qscore_atom, p=c(0.01, 0.5, 0.99, 1.0));
