args=commandArgs(TRUE);

input_file="table_of_global_scores";
testscore_name="qscore_atom";
refscore_name="cadscore_residue";
cor_method="pearson";
filter_file="";
pdf_output_file="";
invert_test_score=FALSE;
normalize_test_score=FALSE;
normalize_test_score_by_area=FALSE;
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
	else if(args[i]=="V-pdf-output")
	{
		pdf_output_file=args[i+1];
	}
	else if(args[i]=="F-invert-testscore")
	{
		invert_test_score=TRUE;
	}
	else if(args[i]=="F-normalize-testscore")
	{
		normalize_test_score=TRUE;
	}
	else if(args[i]=="F-normalize-testscore-by-area")
	{
		normalize_test_score_by_area=TRUE;
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

if(pdf_output_file!="")
{
	pdf(pdf_output_file);
}

t$testscore=t[,testscore_name];
t$refscore=t[,refscore_name];

if (invert_test_score) { t$testscore=(0-t$testscore); }
if (normalize_test_score) { t$testscore=(t$testscore/t$atomscount); }
if (normalize_test_score_by_area) { t$testscore=(t$testscore/t$qarea); }

testscore_limits=c(min(t$testscore), max(t$testscore));
refscore_limits=c(min(t$refscore), max(t$refscore));

targets=sort(union(t$target, t$target));

r=c();

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
	}
}


plot(
		x=c(min(c(r$target_testscore, r$model_best_testscore)), max(c(r$target_testscore, r$model_best_testscore))),
		y=c(min(c(r$target_testscore, r$model_best_testscore)), max(c(r$target_testscore, r$model_best_testscore))),
		type="l", xlab="Target test score", ylab="Highest model test score", main=""
);
points(r$target_testscore, r$model_best_testscore);

plot(
		c(min(c(r$model_best_refscore, r$model_refscore_of_best_testscore)), max(c(r$model_best_refscore, r$model_refscore_of_best_testscore))),
		c(min(c(r$model_best_refscore, r$model_refscore_of_best_testscore)), max(c(r$model_best_refscore, r$model_refscore_of_best_testscore))),
		type="l", xlab="Best reference score", ylab="Reference score corresponding to the best test score", main="");
points(r$model_best_refscore, r$model_refscore_of_best_testscore);

plot(t$refscore, t$testscore, xlab="Reference score", ylab="Test score", main="");


### Results statistics output ###


length(r$target);
failures=which(r$target_testscore<=r$model_best_testscore);
length(failures);
r$target[failures];
r$target_testscore_rank[failures];


quantile(r$target_testscore_zscore);
mean(r$target_testscore_zscore);


quantile(r$cor_testscore_vs_refscore);
mean(r$cor_testscore_vs_refscore);
cor(t$refscore, t$testscore, method=cor_method);
