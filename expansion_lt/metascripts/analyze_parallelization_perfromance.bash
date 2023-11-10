#!/bin/bash

readonly TMPLDIR="$(mktemp -d)"
trap "rm -r $TMPLDIR" EXIT

cat > "${TMPLDIR}/balls"

ALLNUMTHREADS="$1"
OUTFILE="$2"

if [ -z "$ALLNUMTHREADS" ]
then
	ALLNUMTHREADS="1 2 4 8 16 20 26 32 40"
fi

if [ -z "$OUTFILE" ]
then
	OUTFILE="./plot_$(cat ${TMPLDIR}/balls | wc -l).png"
fi

for NUMTHREADS in $ALLNUMTHREADS
do
	cat "${TMPLDIR}/balls" | ./voronota-lt -probe 1.4 -processors "$NUMTHREADS" -measure-time | egrep '^total_|^threads_|^time_' > "${TMPLDIR}/subresults"
	cat "${TMPLDIR}/subresults" | tr -d ':' | awk '{print $1}' | tr '\n' ' ' | sed 's/$/\n/'
	cat "${TMPLDIR}/subresults" | tr -d ':' | awk '{print $2}' | tr '\n' ' ' | sed 's/$/\n/'
done \
| awk '{if(NR==1 || NR%2==0){print $0}}' \
> "${TMPLDIR}/results"

cat "${TMPLDIR}/results"

cd "$TMPLDIR"

R --vanilla > /dev/null << 'EOF'
dt=read.table("results", header=TRUE);

dt$time_tessellation_stage__construct_contacts_ideal=(dt$time_tessellation_stage__construct_contacts[1]*dt$threads_max_number[1])/dt$threads_max_number;
dt$time_full_program_elapsed_ideal=(dt$time_full_program_elapsed[1]*dt$threads_max_number[1])/dt$threads_max_number;

dt$time_tessellation_stage__find_potential_neighbors=(dt$time_tessellation_stage__init_spheres_searcher+dt$time_tessellation_stage__pre.allocate_colliding_IDs+dt$time_tessellation_stage__detect_all_collisions+dt$time_tessellation_stage__count_all_collisions+dt$time_tessellation_stage__collect_relevant_collision_indices+dt$time_tessellation_stage__allocate_possible_contact_summaries);

dt$time_tessellation_stage__collect_contact_summaries=(dt$time_tessellation_stage__count_valid_contact_summaries+dt$time_tessellation_stage__collect_indices_of_valid_contact_summaries+dt$time_tessellation_stage__copy_valid_contact_summaries+dt$time_tessellation_stage__accumulate_total_contacts_summary+dt$time_tessellation_stage__accumulate_cell_summaries+dt$time_tessellation_stage__compute_sas_for_cell_summaries+dt$time_tessellation_stage__accumulate_total_cells_summary);

section_ids=c("time_full_program_elapsed", "time_input_stage__read_balls_from_stdin", "time_tessellation_stage__find_potential_neighbors", "time_tessellation_stage__construct_contacts", "time_tessellation_stage__collect_contact_summaries", "time_full_program_elapsed_ideal", "time_tessellation_stage__construct_contacts_ideal");

section_names=c("full", "input", "find potential neighbors", "construct contacts", "copy contact summaries", "full ideal speed-up", "construct contacts ideal speed-up");
section_colors=c("black", "green", "blue", "red", "purple", "gray", "cyan");

png("plot.png", units="in", width=10, height=10, res=150);
plot(x=dt$threads_max_number, y=dt[,section_ids[1]], col=section_colors[1], ylim=c(0, max(dt[,section_ids[1]])), xlab="num of threads", ylab="miliseconds", main="num of threads vs running miliseconds");
points(x=dt$threads_max_number, y=dt[,section_ids[1]], col=section_colors[1], type="l");
for(i in 2:length(section_ids))
{
	points(x=dt$threads_max_number, y=dt[,section_ids[i]], col=section_colors[i]);
	points(x=dt$threads_max_number, y=dt[,section_ids[i]], col=section_colors[i], type="l");
}
legend("topright", legend=section_names, col=section_colors, lty=rep(1, length(section_names)), cex=1.0);
dev.off();
EOF

cd - &> /dev/null

if [ -n "$OUTFILE" ]
then
	cp "${TMPLDIR}/plot.png" "$OUTFILE"
fi

