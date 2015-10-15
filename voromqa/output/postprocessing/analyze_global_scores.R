args=commandArgs(TRUE);

input_file="table_of_global_scores";
testscore_name="qscore_atom";
refscore_name="cadscore_residue";
cor_method="pearson";
filter_file="";
output_prefix="";
invert_testscore=FALSE;
normalize_testscore=FALSE;
normalize_testscore_by_area=FALSE;
invert_refscore=FALSE;
normalize_refscore=FALSE;
normalize_refscore_by_area=FALSE;
plot_per_target=FALSE;

for(i in 1:length(args))
{
	if(args[i]=="V-input")
	{
		input_file=args[i+1];
	}
	else if(args[i]=="V-testscore-name")
	{
		testscore_name=args[i+1];
	}
	else if(args[i]=="V-refscore-name")
	{
		refscore_name=args[i+1];
	}
	else if(args[i]=="V-cor-method")
	{
		cor_method=args[i+1];
	}
	else if(args[i]=="V-filter")
	{
		filter_file=args[i+1];
	}
	else if(args[i]=="V-output-prefix")
	{
		output_prefix=args[i+1];
	}
	else if(args[i]=="F-invert-testscore")
	{
		invert_testscore=TRUE;
	}
	else if(args[i]=="F-normalize-testscore")
	{
		normalize_testscore=TRUE;
	}
	else if(args[i]=="F-normalize-testscore-by-area")
	{
		normalize_testscore_by_area=TRUE;
	}
	else if(args[i]=="F-invert-refscore")
	{
		invert_refscore=TRUE;
	}
	else if(args[i]=="F-normalize-refscore")
	{
		normalize_refscore=TRUE;
	}
	else if(args[i]=="F-normalize-refscore-by-area")
	{
		normalize_refscore_by_area=TRUE;
	}
	else if(args[i]=="F-plot-per-target")
	{
		plot_per_target=TRUE;
	}
}

t=read.table(input_file, header=TRUE, stringsAsFactors=FALSE);

if(filter_file!="")
{
	filter_t=read.table(filter_file, header=TRUE, stringsAsFactors=FALSE);
	t=merge(t, filter_t);
}

t$testscore=t[,testscore_name];
t$refscore=t[,refscore_name];

if (invert_testscore) { t$testscore=(0-t$testscore); }
if (normalize_testscore) { t$testscore=(t$testscore/t$atomscount); }
if (normalize_testscore_by_area) { t$testscore=(t$testscore/t$qarea); }
if (invert_refscore) { t$refscore=(0-t$refscore); }
if (normalize_refscore) { t$refscore=(t$refscore/t$atomscount); }
if (normalize_refscore_by_area) { t$refscore=(t$refscore/t$qarea); }

testscore_limits=c(min(t$testscore), max(t$testscore));
refscore_limits=c(min(t$refscore), max(t$refscore));

targets=sort(union(t$target, t$target));


if(plot_per_target)
{
	pdf(paste(output_prefix, "plots_per_target.pdf", sep=""));
}

r=c();
models_testscore_zscores=c();
models_refscorescore_zscores=c();

for(target in targets)
{
	st=t[which(t$target==target),];
	sel_target=which(st$model=="target");
	sel_models=which(st$model!="target");
	if(length(sel_target)==1 & length(sel_models)>1)
	{
		model_best_testscore=max(st$testscore[sel_models]);
		sel_model_with_best_testscore=sel_models[which(st$testscore[sel_models]==model_best_testscore)][1];
		
		model_best_refscore=max(st$refscore[sel_models]);
		sel_model_with_best_refscore=sel_models[which(st$refscore[sel_models]==model_best_refscore)][1];
		
		cor_testscore_vs_refscore=cor(st$testscore[sel_models], st$refscore[sel_models], method=cor_method);
		
		sr=data.frame(
				target=target,
				target_testscore=st$testscore[sel_target],
				target_testscore_zscore=((st$testscore[sel_target]-mean(st$testscore[sel_models]))/sd(st$testscore[sel_models])),
				target_testscore_rank=length(which(st$testscore>=st$testscore[sel_target])),
				model_with_best_testscore=st$model[sel_model_with_best_testscore],
				model_best_testscore=model_best_testscore,
				model_with_best_refscore=st$model[sel_model_with_best_refscore],
				model_best_refscore=model_best_refscore,
				cor_testscore_vs_refscore=cor_testscore_vs_refscore,
				model_refscore_of_best_testscore=st$refscore[sel_model_with_best_testscore]);
		if(length(r)>0) { r=rbind(r, sr); } else { r=sr; }
		
		if(plot_per_target)
		{
			plot(x=st$refscore, y=st$testscore, xlim=refscore_limits, ylim=testscore_limits,
					xlab="Reference score", ylab="Test score", main=paste(target, "       cor=", format(cor_testscore_vs_refscore, digits=2), sep=""));
		}
		
		models_testscore_zscores=c(models_testscore_zscores, (st$testscore[sel_models]-mean(st$testscore[sel_models]))/sd(st$testscore[sel_models]));
		models_refscorescore_zscores=c(models_refscorescore_zscores, (st$refscore[sel_models]-mean(st$refscore[sel_models]))/sd(st$refscore[sel_models]));
	}
}

write.table(r, file=paste(output_prefix, "results_table", sep=""), row.names=FALSE, quote=FALSE);

r_summary=data.frame(
		test_score=testscore_name,
		reference_score=refscore_name,
		targets_count=length(r$target),
		failures_count=length(which(r$target_testscore<=r$model_best_testscore)),
		mean_target_testscore_zscore=mean(r$target_testscore_zscore),
		mean_cor_testscore_vs_refscore=mean(r$cor_testscore_vs_refscore));

write.table(r_summary, file=paste(output_prefix, "results_summary", sep=""), row.names=FALSE, quote=FALSE);

pdf(paste(output_prefix, "plots_summary.pdf", sep=""));

testscore_range=c(min(c(r$target_testscore, r$model_best_testscore)), max(c(r$target_testscore, r$model_best_testscore)));
plot(x=testscore_range, y=testscore_range, type="l", xlab="Target test score", ylab="Highest model test score", main="");
points(r$target_testscore, r$model_best_testscore);

refscore_range=c(min(c(r$model_best_refscore, r$model_refscore_of_best_testscore)), max(c(r$model_best_refscore, r$model_refscore_of_best_testscore)));
plot(x=refscore_range, y=refscore_range, type="l", xlab="Best reference score", ylab="Reference score corresponding to the best test score", main="");
points(r$model_best_refscore, r$model_refscore_of_best_testscore);

plot(t$refscore, t$testscore, xlab="Reference score", ylab="Test score", main="", col=densCols(t$refscore, t$testscore));

plot(models_refscorescore_zscores, models_testscore_zscores, xlab="Reference score Z-score", ylab="Test score Z-score", main="", col=densCols(models_refscorescore_zscores, models_testscore_zscores));


png(paste(output_prefix, "correlation_of_scores.png", sep=""), height=5, width=6, units="in", res=300);
t_without_targets=t[which(t$model!="target"),];
plot(t_without_targets$refscore, t_without_targets$testscore, xlab="Reference score", ylab="Test score", main=paste("cor. =", cor(t_without_targets$refscore, t_without_targets$testscore)), col=densCols(t_without_targets$refscore, t_without_targets$testscore));

png(paste(output_prefix, "correlation_of_zscores.png", sep=""), height=5, width=6, units="in", res=300);
plot(models_refscorescore_zscores, models_testscore_zscores, xlab="Reference score Z-score", ylab="Test score Z-score", main=paste("cor. =", cor(models_refscorescore_zscores, models_testscore_zscores)), col=densCols(models_refscorescore_zscores, models_testscore_zscores));
