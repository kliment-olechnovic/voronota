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
		
		model_best_cadscore=max(st$cadscore_residue[sel_models]);
		sel_model_with_best_cadscore=sel_models[which(st$cadscore_residue[sel_models]==model_best_cadscore)][1];
		
		cor_qscore_vs_cadscore=cor(st$qscore_atom[sel_models], st$cadscore_residue[sel_models]);
		
		sr=data.frame(
				target=target,
				target_qscore=st$qscore_atom[sel_target],
				model_with_best_qscore=st$model[sel_model_with_best_qscore],
				model_best_qscore=model_best_qscore,
				model_with_best_cadscore=st$model[sel_model_with_best_cadscore],
				model_best_cadscore=model_best_cadscore,
				cor_qscore_vs_cadscore=cor_qscore_vs_cadscore,
				model_cadscore_of_best_qscore=st$cadscore_residue[sel_model_with_best_qscore]);
		if(length(r)>0) { r=rbind(r, sr); } else { r=sr; }
	}
}

length(r$target);
length(which(r$target_qscore<=r$model_best_qscore));

quantile(r$cor_qscore_vs_cadscore);
mean(r$cor_qscore_vs_cadscore);
cor(t$cadscore_residue, t$qscore_atom);

r[which(r$target_qscore<=r$model_best_qscore),];

plot(c(0, 1), c(0, 1), type="l", xlab="Target quality score", ylab="Highest model quality score", main=""); points(r$target_qscore, r$model_best_qscore);

plot(c(0, 1), c(0, 1), type="l", xlab="Best CAD-score", ylab="CAD-score corresponding to the best quality score", main=""); points(r$model_best_cadscore, r$model_cadscore_of_best_qscore);

plot(t$cadscore_residue, t$qscore_atom, xlab="CAD-score", ylab="Quality score", main="");
