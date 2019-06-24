#!/bin/bash

if [ $# -ne 2 ]; then
    cat <<EOM
Usage: ./runme.sh <TASK_ID> <BUY_ITEMS>

TASK_ID
    decimal

BUY_ITEMS
    e.g.) "CCCB"
EOM
    exit
fi

TASK_ID=$( printf "%03d" $1 )
cargo run --release ../problems/prob-$TASK_ID.in "$2"

echo "$2" > ../problems/prob-$TASK_ID.buy

echo "Submitting /tmp/$TASK_ID.out"
SUBMIT_ID=$(
    curl -k https://negainoido.dip.jp/score/solution \
        -F score=4 \
        -F file=@../problems/prob-$TASK_ID.in.hand.out \
        -F buyFile=@../problems/prob-$TASK_ID.buy \
        -F solver="$(whoami)-hand" \
        -F task=$TASK_ID |
        jq .solution.id
)

echo "Validating $SUBMID_ID"
curl --fail -k https://negainoido.dip.jp/score/solution/$SUBMIT_ID/validate
