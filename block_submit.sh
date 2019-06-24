set -e

cd $(dirname $0)
if [ -z "$BLOCK" ]; then
    cd lambda-client
    BLOCK=$(./lambda-cli.py getblockchaininfo | tr "'" '"' | jq .block)
    cd -
fi
set -u
TASK_DESC=lambda-client/blocks/$BLOCK/task.desc
PUZZLE_COND=lambda-client/blocks/$BLOCK/puzzle.cond
SUBMIT_DIR=$(pwd)/tmp/submit/$BLOCK
SCORE=$(python3 official_sim/main.py $TASK_DESC $SUBMIT_DIR/task.sol)
echo "Task socre is $SCORE"
python3 official_sim/puzzle_checker.py $PUZZLE_COND $SUBMIT_DIR/puzzle.solution.desc

cd lambda-client
./lambda-cli.py submit $BLOCK $SUBMIT_DIR/task.sol $SUBMIT_DIR/puzzle.solution.desc

echo "Submission $BLOCK complete"

