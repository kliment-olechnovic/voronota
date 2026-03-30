#!/bin/bash

df=read.table("comparison_of_cadscore_versions.tsv", header=TRUE, stringsAsFactors=FALSE);

name_CADscore2012="CAD-score-2012";
name_CADscore2021="CAD-score-2021";
name_CADscoreLT="CAD-score-LT";

name_CADscore2012_global=paste(name_CADscore2012, " whole structure score");
name_CADscore2021_global=paste(name_CADscore2021, " whole structure score");
name_CADscoreLT_global=paste(name_CADscoreLT, " whole structure score");

name_CADscore2012_iface=paste(name_CADscore2012, " inter-chain interface score");
name_CADscore2021_iface=paste(name_CADscore2021, " inter-chain interface score");
name_CADscoreLT_iface=paste(name_CADscoreLT, " inter-chain interface score");

#######################################

options(scipen=8);

#######################################

png("./plot_comparing_CADscoreLT_vs_CADscore2012_scores.png", height=5.0, width=10.0, units="in", res=100);
par(mfrow=c(1, 2));

plot(df$CADscore2012_global_score, df$CADscoreLT_global_score, cex=0.6,
  xlab=name_CADscore2012_global, ylab=name_CADscoreLT_global, main=paste0("Whole structure scores\n", name_CADscoreLT, " vs ", name_CADscore2012));
abline(0, 1, col="red");
mtext("a", side=3, adj=-0.17, line=2.5, cex=2.0, font=2);

plot(df$CADscore2012_iface_score, df$CADscoreLT_iface_score, cex=0.6,
  xlab=name_CADscore2012_iface, ylab=name_CADscoreLT_iface, main=paste0("Inter chain interface scores\n", name_CADscoreLT, " vs ", name_CADscore2012));
abline(0, 1, col="red");
mtext("b", side=3, adj=-0.17, line=2.5, cex=2.0, font=2);

dev.off();

#######################################

png("./plot_comparing_CADscoreLT_vs_CADscore2021_scores.png", height=5.0, width=10.0, units="in", res=100);
par(mfrow=c(1, 2));

plot(df$CADscore2021_global_score, df$CADscoreLT_global_score, cex=0.6,
  xlab=name_CADscore2021_global, ylab=name_CADscoreLT_global, main=paste0("Whole structure scores\n", name_CADscoreLT, " vs ", name_CADscore2021));
abline(0, 1, col="red");
mtext("a", side=3, adj=-0.17, line=2.5, cex=2.0, font=2);

plot(df$CADscore2021_iface_score, df$CADscoreLT_iface_score, cex=0.6,
  xlab=name_CADscore2021_iface, ylab=name_CADscoreLT_iface, main=paste0("Inter chain interface scores\n", name_CADscoreLT, " vs ", name_CADscore2021));
abline(0, 1, col="red");
mtext("b", side=3, adj=-0.17, line=2.5, cex=2.0, font=2);

dev.off();

#######################################

col_CADscore2012=palette()[3];
col_CADscore2021=palette()[2];
col_CADscoreLT=palette()[4];
col_CADscore2021modLT=palette()[8];

#######################################

png("./plot_comparing_times_of_CADscore_versions.png", height=10.0, width=10.0, units="in", res=100);
par(mfrow=c(2, 2));

plot(x=c(min(df$total_input_atoms), max(df$total_input_atoms)), y=c(0, max(c(df$CADscore2012_global_time, df$CADscore2021_global_time, df$CADscoreLT_global_time))), type="n",
  xlab="Total input atoms", ylab="Seconds (CPU user time)", main="Whole structure scoring times");
points(x=df$total_input_atoms, y=df$CADscore2012_global_time, col=col_CADscore2012, cex=0.6, pch=2);
points(x=df$total_input_atoms, y=df$CADscore2021_global_time, col=col_CADscore2021, cex=0.6, pch=6);
points(x=df$total_input_atoms, y=df$CADscoreLT_global_time, col=col_CADscoreLT, cex=0.5, pch=1);
legend("topleft", legend=c("CAD-score-2012", "CAD-score-2021", "CAD-score-LT"), col=c(col_CADscore2012, col_CADscore2021, col_CADscoreLT), bty="n", pch=c(2, 6, 1), pt.cex=1.2);
mtext("a", side=3, adj=-0.16, line=1.8, cex=1.5, font=2);

plot(x=c(min(df$total_input_atoms), max(df$total_input_atoms)), y=c(0, 7), type="n",
  xlab="Total input atoms", ylab="Seconds (CPU user time)", main="Whole structure scoring times\n(showing times below 7 seconds)");
points(x=df$total_input_atoms, y=df$CADscore2012_global_time, col=col_CADscore2012, cex=0.6, pch=2);
points(x=df$total_input_atoms, y=df$CADscore2021_global_time, col=col_CADscore2021, cex=0.6, pch=6);
points(x=df$total_input_atoms, y=df$CADscoreLT_global_time, col=col_CADscoreLT, cex=0.5, pch=1);
mtext("b", side=3, adj=-0.16, line=1.8, cex=1.5, font=2);

plot(x=c(min(df$total_input_atoms), max(df$total_input_atoms)), y=c(0, max(c(df$CADscore2012_iface_time, df$CADscore2021_iface_time, df$CADscoreLT_iface_time))), type="n",
  xlab="Total input atoms", ylab="Seconds (CPU user time)", main="Inter-chain interface scoring times");
points(x=df$total_input_atoms, y=df$CADscore2012_iface_time, col=col_CADscore2012, cex=0.6, pch=2);
points(x=df$total_input_atoms, y=df$CADscore2021_iface_time, col=col_CADscore2021, cex=0.6, pch=6);
points(x=df$total_input_atoms, y=df$CADscoreLT_iface_time, col=col_CADscoreLT, cex=0.5, pch=1);
legend("topleft", legend=c("CAD-score-2012", "CAD-score-2021", "CAD-score-LT"), col=c(col_CADscore2012, col_CADscore2021, col_CADscoreLT), bty="n", pch=c(2, 6, 1), pt.cex=1.2);
mtext("c", side=3, adj=-0.16, line=1.8, cex=1.5, font=2);

plot(x=c(min(df$total_input_atoms), max(df$total_input_atoms)), y=c(0, 7), type="n",
  xlab="Total input atoms", ylab="Seconds (CPU user time)", main="Inter-chain interface scoring times\n(showing times below 7 seconds)");
points(x=df$total_input_atoms, y=df$CADscore2012_iface_time, col=col_CADscore2012, cex=0.6);
points(x=df$total_input_atoms, y=df$CADscore2021_iface_time, col=col_CADscore2021, cex=0.6);
points(x=df$total_input_atoms, y=df$CADscoreLT_iface_time, col=col_CADscoreLT, cex=0.5);
mtext("d", side=3, adj=-0.16, line=1.8, cex=1.5, font=2);

dev.off();

#######################################

png("./plot_comparing_times_of_CADscore_LT_and_JS_LT_versions.png", height=5.0, width=10.0, units="in", res=100);
par(mfrow=c(1, 2));

plot(x=c(min(df$total_input_atoms), max(df$total_input_atoms)), y=c(0, max(c(df$CADscore2021_LT_global_time, df$CADscoreLT_global_time))), type="n",
  xlab="Total input atoms", ylab="Seconds (CPU user time)", main="Whole structure scoring times:\n CAD-score-2021-LT-mod vs CAD-score-LT");
points(x=df$total_input_atoms, y=df$CADscore2021_LT_global_time, col=col_CADscore2021modLT, cex=0.6, pch=6);
points(x=df$total_input_atoms, y=df$CADscoreLT_global_time, col=col_CADscoreLT, cex=0.5, pch=1);
legend("topleft", legend=c("CAD-score-2021-LT-mod", "CAD-score-LT"), col=c(col_CADscore2021modLT, col_CADscoreLT), bty="n", pch=c(6, 1), pt.cex=1.2);
mtext("a", side=3, adj=-0.16, line=1.8, cex=1.5, font=2);

plot(x=c(min(df$total_input_atoms), max(df$total_input_atoms)), y=c(0, max(c(df$CADscore2021_LT_iface_time, df$CADscoreLT_iface_time))), type="n",
  xlab="Total input atoms", ylab="Seconds (CPU user time)", main="Inter-chain interface scoring times:\n CAD-score-2021-LT-mod vs CAD-score-LT");
points(x=df$total_input_atoms, y=df$CADscore2021_LT_iface_time, col=col_CADscore2021modLT, cex=0.6, pch=6);
points(x=df$total_input_atoms, y=df$CADscoreLT_iface_time, col=col_CADscoreLT, cex=0.5, pch=1);
legend("topleft", legend=c("CAD-score-2021-LT-mod", "CAD-score-LT"), col=c(col_CADscore2021modLT, col_CADscoreLT), bty="n", pch=c(6, 1), pt.cex=1.2);
mtext("b", side=3, adj=-0.16, line=1.8, cex=1.5, font=2);

dev.off();

