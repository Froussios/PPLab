#!/bin/bash

datafile="data.txt"
outseq="$(./sequential $datafile)"
outomp="$(./openmp $datafile)"

#echo "$outseq" | sed -n '1p'
outseqR="$(echo "$outseq" | head -2)"
outompR="$(echo "$outomp" | head -2)"

if [ "$outseqR" == "$outompR" ]
then
	echo "Comparison success"
fi


