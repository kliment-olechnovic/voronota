#!/bin/bash

R --vanilla < ./analyze.R

cat quantiles_by_length | column -t | sponge quantiles_by_length
