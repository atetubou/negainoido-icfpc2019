set -e

mkdir -p tmp

if [ -z "$BLOCK" ]; then
    cd lambda-client

    pip3 install -r requirements.txt
    ./lambdad.py &
    sleep 1
    ./lambda-cli.py getmininginfo | tr "'" '"' > ../tmp/miining.json
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
    kill $!
fi

echo "Block $BLOCK"


TASK_DESC=lambda-client/blocks/$BLOCK/task.desc
PUZZLE_COND=lambda-client/blocks/$BLOCK/puzzle.cond
PUZZLE_TXT=$PUZZLE_COND.txt
python3 ./problems/convert_readable.py lambda-client/blocks/$BLOCK/task.desc
python3 ./puzzle_solver/input_formatter.py $PUZZLE_COND
bazel run //puzzle_solver:flowlight_solver < $PUZZLE_TXT > puzzle_solution.txt
bazel run //lambda-coin:reverse_convert < puzzle_solution.txt > puzzle.solution.desc
python3 official_sim/puzzle_checker.py $PUZZLE_COND puzzle.solution.desc

echo "Puzzle $BLOCK was solved! Let's submit puzzle.solutiond.desc!"
