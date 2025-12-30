#!/bin/bash

cd $(dirname "$0")

SOURCES_DIR="$1"

if [ -z "$SOURCES_DIR" ]
then
	SOURCES_DIR="../src"
fi

if [ ! -d "./cosmocc" ]
then
	mkdir ./cosmocc
	cd ./cosmocc
	wget https://cosmo.zip/pub/cosmocc/cosmocc.zip
	unzip ./cosmocc.zip
	cd ..
fi

rm -f "./voronota-lt"
rm -f "./voronota-lt.aarch64.elf"
rm -f "./voronota-lt.com.dbg"

./cosmocc/bin/cosmoc++ -O3 -fopenmp -o ./voronota-lt "${SOURCES_DIR}/voronota_lt.cpp"

rm -f "./voronota-lt.aarch64.elf"
rm -f "./voronota-lt.com.dbg"
