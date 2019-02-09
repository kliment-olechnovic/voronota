#!/bin/bash

{
	cat ../../resources/voromqa_v1_energy_potential \
	| grep -v solvent
	
	cat ./solvent_vs_nonsolvent \
	| tail -n +2 \
	| awk '{print $1 " c<solvent> . " $10}'
} \
> ./voromqa_v1_energy_potential_modified
