#!/bin/bash

{
	cat ./potential \
	| grep -v solvent
	
	cat ./solvent_vs_nonsolvent \
	| tail -n +2 \
	| awk '{print $1 " c<solvent> . " $6}'
} \
> ./potential_modified
