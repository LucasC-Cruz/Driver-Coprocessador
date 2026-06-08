#!/bin/bash
SUPERPASTA="/mnt/c/Users/lucas/Desktop/test"
OUTPUT="arquivos.txt"
> "$OUTPUT"  # limpa o txt antes

for i in $(seq 0 9); do
    SUBPASTA="$SUPERPASTA/$i"
    find "$SUBPASTA" -type f | while read caminho; do
        echo "$caminho;$i;" >> "$OUTPUT"
    done
done
