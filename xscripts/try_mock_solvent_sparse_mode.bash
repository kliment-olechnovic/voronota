#!/bin/bash

INFILE="$1"

if [ ! -s "$INFILE" ]
then
	exit 1
fi

mkdir -p ./tmp

rm ./tmp/mock_balls.py ./tmp/mock_surface.py

cat "$INFILE" \
| ./voronota get-balls-from-atoms-file --annotated \
| ./voronota x-calculate-mock-solvent --solvent-distance 1 --solvent-radius 1.4 --sparse-mode \
| ./voronota query-balls --match 'c<w>' --set-adjuncts 'r=0;g=1;b=1' \
| ./voronota x-draw-balls --representation vdw --default-color 0xFFFF00 --adjuncts-rgb --drawing-name mock_balls --drawing-for-pymol ./tmp/mock_balls.py \
> /dev/null

cat "$INFILE" \
| ./voronota get-balls-from-atoms-file --annotated --radii-file ./resources/radii \
| ./voronota x-calculate-mock-solvent --solvent-distance 1 --solvent-radius 1.4 --sparse-mode \
| ./voronota calculate-contacts --annotated --draw --probe 2.8 \
| ./voronota query-contacts --match-second 'c<solvent>' --preserve-graphics \
| ./voronota draw-contacts --default-color 0xFF00FF --drawing-name mock_surface --drawing-for-pymol ./tmp/mock_surface.py \
> /dev/null

pymol ./tmp/mock_balls.py ./tmp/mock_surface.py
