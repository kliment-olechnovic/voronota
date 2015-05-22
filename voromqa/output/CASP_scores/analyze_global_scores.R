args=commandArgs(TRUE);

testscore_name=args[1];
refscore_name=args[2];
invert_test_score=args[3];

t=read.table("table_of_global_scores", header=TRUE, stringsAsFactors=FALSE);

t$testscore=t[,testscore_name];
t$refscore=t[,refscore_name];

if (invert_test_score=="yes") { t$testscore=(0-t$testscore); }

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
		
		cor_testscore_vs_refscore=cor(st$testscore[sel_models], st$refscore[sel_models]);
		
		sr=data.frame(
				target=target,
				target_testscore=st$testscore[sel_target],
				target_testscore_rank=length(which(st$testscore>=st$testscore[sel_target])),
				model_with_best_testscore=st$model[sel_model_with_best_testscore],
				model_best_testscore=model_best_testscore,
				model_with_best_refscore=st$model[sel_model_with_best_refscore],
				model_best_refscore=model_best_refscore,
				cor_testscore_vs_refscore=cor_testscore_vs_refscore,
				model_refscore_of_best_testscore=st$refscore[sel_model_with_best_testscore]);
		if(length(r)>0) { r=rbind(r, sr); } else { r=sr; }
	}
}

################################

length(r$target);

failures=which(r$target_testscore<=r$model_best_testscore);
length(failures);
r$target[failures];
r$target_testscore_rank[failures];

################################

quantile(r$cor_testscore_vs_refscore);
mean(r$cor_testscore_vs_refscore);
cor(t$refscore, t$testscore);

################################

plot(
		x=c(min(c(r$target_testscore, r$model_best_testscore)), max(c(r$target_testscore, r$model_best_testscore))),
		y=c(min(c(r$target_testscore, r$model_best_testscore)), max(c(r$target_testscore, r$model_best_testscore))),
		type="l", xlab="Target test score", ylab="Highest model test score", main=""
);
points(r$target_testscore, r$model_best_testscore);

################################

plot(
		c(min(c(r$model_best_refscore, r$model_refscore_of_best_testscore)), max(c(r$model_best_refscore, r$model_refscore_of_best_testscore))),
		c(min(c(r$model_best_refscore, r$model_refscore_of_best_testscore)), max(c(r$model_best_refscore, r$model_refscore_of_best_testscore))),
		type="l", xlab="Best reference score", ylab="Reference score corresponding to the best test score", main="");
points(r$model_best_refscore, r$model_refscore_of_best_testscore);

################################

plot(t$refscore, t$testscore, xlab="Reference score", ylab="Test score", main="");
