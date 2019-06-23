#!/bin/bash

if [ $# -ne 1 ]; then
    cat <<EOM
Usage: ./runme.sh 39  # <= solve the problem #039
EOM
    exit
fi

TASK_ID=$( printf "%03d" $1 )
cargo run ../problems/prob-$TASK_ID.in

echo "Submitting /tmp/$TASK_ID.out"
SUBMIT_ID=$(
    curl -k https://negainoido.dip.jp/score/solution -F score=4 -F file=@../problems/prob-$TASK_ID.in.hand.out -F solver="$(whoami)-hand" -F task=$TASK_ID |
        jq .solution.id
)

echo "Validating $SUBMID_ID"
curl --fail -k https://negainoido.dip.jp/score/solution/$SUBMIT_ID/validate
