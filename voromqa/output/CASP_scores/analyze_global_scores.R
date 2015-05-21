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
		
		model_best_goap=min(st$goap[sel_models]);
		sel_model_with_best_goap=sel_models[which(st$goap[sel_models]==model_best_goap)][1];
		
		model_best_dfire=min(st$dfire[sel_models]);
		sel_model_with_best_dfire=sel_models[which(st$dfire[sel_models]==model_best_dfire)][1];
		
		model_best_goap_ag=min(st$goap_ag[sel_models]);
		sel_model_with_best_goap_ag=sel_models[which(st$goap_ag[sel_models]==model_best_goap_ag)][1];
		
		cor_qscore_vs_cadscore=cor(st$qscore_atom[sel_models], st$cadscore_residue[sel_models]);
		cor_goap_vs_cadscore=cor(st$goap[sel_models], st$cadscore_residue[sel_models]);
		cor_dfire_vs_cadscore=cor(st$dfire[sel_models], st$cadscore_residue[sel_models]);
		cor_goap_ag_vs_cadscore=cor(st$goap_ag[sel_models], st$cadscore_residue[sel_models]);
		
		sr=data.frame(
				target=target,
				target_qscore=st$qscore_atom[sel_target],
				target_goap=st$goap[sel_target],
				target_dfire=st$dfire[sel_target],
				target_goap_ag=st$goap_ag[sel_target],
				model_with_best_qscore=st$model[sel_model_with_best_qscore],
				model_best_qscore=model_best_qscore,
				model_with_best_goap=st$model[sel_model_with_best_goap],
				model_best_goap=model_best_goap,
				model_with_best_dfire=st$model[sel_model_with_best_dfire],
				model_best_dfire=model_best_dfire,
				model_with_best_goap_ag=st$model[sel_model_with_best_goap_ag],
				model_best_goap_ag=model_best_goap_ag,
				model_with_best_cadscore=st$model[sel_model_with_best_cadscore],
				model_best_cadscore=model_best_cadscore,
				cor_qscore_vs_cadscore=cor_qscore_vs_cadscore,
				cor_goap_vs_cadscore=cor_goap_vs_cadscore,
				cor_dfire_vs_cadscore=cor_dfire_vs_cadscore,
				cor_goap_ag_vs_cadscore=cor_goap_ag_vs_cadscore,
				model_cadscore_of_best_qscore=st$cadscore_residue[sel_model_with_best_qscore],
				model_cadscore_of_best_goap=st$cadscore_residue[sel_model_with_best_goap],
				model_cadscore_of_best_dfire=st$cadscore_residue[sel_model_with_best_dfire],
				model_cadscore_of_best_goap_ag=st$cadscore_residue[sel_model_with_best_goap_ag]);
		if(length(r)>0) { r=rbind(r, sr); } else { r=sr; }
	}
}

################################

length(r$target);

length(which(r$target_qscore<=r$model_best_qscore));
r$target[which(r$target_qscore<=r$model_best_qscore)];

length(which(r$target_goap>=r$model_best_goap));
r$target[which(r$target_goap>=r$model_best_goap)];

length(which(r$target_dfire>=r$model_best_dfire));
r$target[which(r$target_dfire>=r$model_best_dfire)];

################################

quantile(r$cor_qscore_vs_cadscore);
mean(r$cor_qscore_vs_cadscore);
cor(t$cadscore_residue, t$qscore_atom);

quantile(r$cor_goap_vs_cadscore);
mean(r$cor_goap_vs_cadscore);
cor(t$cadscore_residue, t$goap);

quantile(r$cor_dfire_vs_cadscore);
mean(r$cor_dfire_vs_cadscore);
cor(t$cadscore_residue, t$dfire);

################################

plot(c(0, 1), c(0, 1), type="l", xlab="Target quality score", ylab="Highest model quality score", main="");
points(r$target_qscore, r$model_best_qscore);

plot(
		c(min(c(r$target_goap, r$model_best_goap)), max(c(r$target_goap, r$model_best_goap))),
		c(min(c(r$target_goap, r$model_best_goap)), max(c(r$target_goap, r$model_best_goap))),
		type="l", xlab="Target GOAP score", ylab="Highest model GOAP score", main=""
);
points(r$target_goap, r$model_best_goap);

plot(
		c(min(c(r$target_dfire, r$model_best_dfire)), max(c(r$target_dfire, r$model_best_dfire))),
		c(min(c(r$target_dfire, r$model_best_dfire)), max(c(r$target_dfire, r$model_best_dfire))),
		type="l", xlab="Target DFIRE score", ylab="Highest model DFIRE score", main=""
);
points(r$target_dfire, r$model_best_dfire);

################################

plot(c(0, 1), c(0, 1), type="l", xlab="Best CAD-score", ylab="CAD-score corresponding to the best quality score", main=""); points(r$model_best_cadscore, r$model_cadscore_of_best_qscore);

plot(c(0, 1), c(0, 1), type="l", xlab="Best CAD-score", ylab="CAD-score corresponding to the best GOAP score", main=""); points(r$model_best_cadscore, r$model_cadscore_of_best_goap);

plot(c(0, 1), c(0, 1), type="l", xlab="Best CAD-score", ylab="CAD-score corresponding to the best DFIRE score", main=""); points(r$model_best_cadscore, r$model_cadscore_of_best_dfire);

################################

plot(t$cadscore_residue, t$qscore_atom, xlab="CAD-score", ylab="Quality score", main="");

plot(t$cadscore_residue, t$goap, xlab="CAD-score", ylab="GOAP score", main="");

plot(t$cadscore_residue, t$dfire, xlab="CAD-score", ylab="DFIRE score", main="");
