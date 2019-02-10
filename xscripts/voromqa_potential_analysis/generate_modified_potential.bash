#!/bin/bash

{
	cat ./potential \
	| grep -v solvent
	
	cat ./solvent_vs_nonsolvent \
	| tail -n +2 \
	| awk '{print $1 " c<solvent> . " $10}'
} \
> ./potential_modified
