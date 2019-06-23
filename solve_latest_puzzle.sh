set -e

cd $(dirname $0)

mkdir -p tmp

if [ -z "$BLOCK" ]; then
    cd lambda-client

    pip3 install -r requirements.txt
    ./lambdad.py &
    sleep 1
    ./lambda-cli.py getmininginfo | tr "'" '"' > ../tmp/mining.json
    ./lambda-cli.py getblockchaininfo | tr "'" '"' > ../tmp/block.json

    cd -

    BLOCK=$( jq .block ./tmp/block.json )
    while [ ! -e "lambda-client/blocks/$BLOCK/task.desc" ]
    do
        echo "Wait for task.desc"
        sleep 1
    done
    while [ ! -e "lambda-client/blocks/$BLOCK/puzzle.cond" ]
    do
        echo "Wait for puzzle.desc"
        sleep 1
    done
    sleep 1
    kill $! || true
fi

echo "Block $BLOCK"

SOLVER=${SOLVER:-"//flowlight/main:flowlight_solver"}
TASK_DESC=lambda-client/blocks/$BLOCK/task.desc
TASK_TXT=lambda-client/blocks/$BLOCK/task.in
PUZZLE_COND=lambda-client/blocks/$BLOCK/puzzle.cond
PUZZLE_TXT=$PUZZLE_COND.txt

SUBMIT_DIR=$(pwd)/tmp/submit/$BLOCK
mkdir -p $SUBMIT_DIR

python3 ./problems/convert_readable.py $TASK_DESC
bazel run $SOLVER < $TASK_TXT > task.sol
SCORE=$(python3 official_sim/main.py $TASK_DESC task.sol)
if [ -e "$SUBMIT_DIR/task.sol" ]; then
    CUR_SCORE=$(python3 official_sim/main.py $TASK_DESC $SUBMIT_DIR/task.sol)
    if [ $SCORE -lt $CUR_SCORE ]; then
        echo "new one is better"
        cp task.sol $SUBMIT_DIR
    else
        echo "old one is better"
        SCORE=$CUR_SCORE
    fi
else
    cp task.sol $SUBMIT_DIR
fi
echo "TASK $BLOCK was solved!"

python3 ./puzzle_solver/input_formatter.py $PUZZLE_COND
bazel run //puzzle_solver:flowlight_solver < $PUZZLE_TXT > puzzle_solution.txt
bazel run //lambda-coin:reverse_convert < puzzle_solution.txt > puzzle.solution.desc
python3 official_sim/puzzle_checker.py $PUZZLE_COND puzzle.solution.desc
cp puzzle.solution.desc $SUBMIT_DIR
echo "Puzzle $BLOCK was solved! Let's submit puzzle.solutiond.desc! Task Score: $SCORE"

if [ -n "$SUBMIT" ]; then
    cd lambda-client
    ./lambdad.py &
    sleep 1
    RES=$(./lambda-cli.py submit $BLOCK $SUBMIT_DIR/task.sol $SUBMIT_DIR/puzzle.solution.desc)
    curl -X POST -H 'Content-type: application/json' --data "{\"text\":\"submit result: $RES\"}" https://hooks.slack.com/services/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
    kill $! || true

    echo "Submission $BLOCK complete"
fi
