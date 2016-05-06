#!/bin/bash

for f in $(find . -name "*.fbs");do
  flatc -o include/schemas/ -c $f 
done
