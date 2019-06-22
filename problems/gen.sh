#!/bin/bash

for f in *desc
do
    python3 convert_readable.py $f || echo "error caught with" $f
done
