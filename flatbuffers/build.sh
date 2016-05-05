#!/bin/bash

for f in $(ls *.fbs);do
  flatc -o include/schemas/ -c $f 
done
