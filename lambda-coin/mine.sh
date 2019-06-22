#!/bin/bash

(
    cd ../lambda-client/;
    python ./lambda-cli.py getblockchaininfo
) | tr "'" '"' >/tmp/blockchaininfo.json

(
    cd ../lambda-client/;
    python ./lambda-cli.py getmininginfo
) | tr "'" '"' > /tmp/mininginfo.json

BLOCK=$( jq .block /tmp/blockchaininfo.json )
echo "Block $BLOCK"

COND_PATH=$(printf "/tmp/block_%08d.cond" "$BLOCK")
PUZZLE_SOLVED_PATH=$(printf "/tmp/block_%08d.solved.desc" "$BLOCK")
DESC_PATH=$(printf "/tmp/block_%08d.desc" "$BLOCK")

# puzzle
jq -r .puzzle /tmp/mininginfo.json > $COND_PATH
../solve-puzzle < $COND_PATH | ../honda/reverse_convert > $PUZZLE_SOLVED_PATH

# task
jq -r .task /tmp/mininginfo.json > $DESC_PATH
python ../problems/convert_readable.py $DESC_PATH

echo $PUZZLE_SOLVED_PATH
echo $DESC_PATH
