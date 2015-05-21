t=read.table("table_of_global_scores", header=TRUE, stringsAsFactors=FALSE);

targets=sort(union(t$target, t$target));

r=c();

for(target in targets)
{
	st=t[which(t$target==target),];
	sel_target=which(st$model=="target");
	sel_models=which(st$model!="target");
	if(length(sel_target)==1 & length(sel_models)>1)
	{
		model_best_qscore=max(st$qscore_atom[sel_models]);
		sel_model_with_best_qscore=sel_models[which(st$qscore_atom[sel_models]==model_best_qscore)][1];
		sr=data.frame(
				target=target,
				target_qscore=st$qscore_atom[sel_target],
				model_with_best_qscore=st$model[sel_model_with_best_qscore],
				model_qscore=model_best_qscore);
		if(length(r)>0) { r=rbind(r, sr); } else { r=sr; }
	}
}

length(r$target);
length(which(r$target_qscore<=r$model_qscore));

r[which(r$target_qscore<=r$model_qscore),];

plot(c(0, 1), c(0, 1), type="l", xlab="Target quality score", ylab="Highest model quality score", main=""); points(r$target_qscore, r$model_qscore);
