#!/bin/bash

cd $(dirname "$0")

SOURCES_DIR="$1"

if [ ! -d "./cosmocc" ]
then
	mkdir ./cosmocc
	cd ./cosmocc
	wget https://cosmo.zip/pub/cosmocc/cosmocc.zip
	unzip ./cosmocc.zip
	cd ..
fi

rm -f "./cadscore-lt"
rm -f "./cadscore-lt.aarch64.elf"
rm -f "./cadscore-lt.com.dbg"

if [ -z "$SOURCES_DIR" ]
then
	./cosmocc/bin/cosmoc++ -O3 -fopenmp -I "../../expansion_lt/src" -o ./cadscore-lt "../src/cadscore_lt.cpp"
else
	./cosmocc/bin/cosmoc++ -O3 -fopenmp -I "${SOURCES_DIR}" -o ./cadscore-lt "${SOURCES_DIR}/cadscore_lt.cpp"
fi

rm -f "./cadscore-lt.aarch64.elf"
rm -f "./cadscore-lt.com.dbg"
