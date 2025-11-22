#!/bin/bash

cd $(dirname "$0")

rm -rf "./testvenv"

python3 -m venv testvenv

source ./testvenv/bin/activate

pip install biotite

pip install biopython

pip install pandas

deactivate

exit 0
