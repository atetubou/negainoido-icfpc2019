set -e

cd $(dirname $0)
cd lambda-client
./lambdad.py &
BLOCK=$(./lambda-cli.py getblockchaininfo | tr "'" '"' | jq .block)
echo "Current block is $BLOCK"

while true
do
    NEW_BLOCK=$(./lambda-cli.py getblockchaininfo | tr "'" '"' | jq .block)
    if [ "$BLOCK" -ne "$NEW_BLOCK" ]; then
        echo "NEW BLOCK is $NEW_BLOCK"
        BLOCK=$NEW_BLOCK
        notify-send -u normal -t 60000 "New Block $BLOCK started"
        curl -X POST -H 'Content-type: application/json' --data "{\"text\":\"New block $BLOCK started\"}" https://hooks.slack.com/services/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
        BLOCK=$BLOCK SOLVER=//tikuta_solver:tikuta_solver3 ../solve_task.sh || curl -X POST -H 'Content-type: application/json' --data "{\"text\":\"tikuta failed\"}" https://hooks.slack.com/services/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
        SUBMIT=1 ../solve_latest_puzzle.sh || (curl -X POST -H 'Content-type: application/json' --data "{\"text\":\"@channel failed to submit $BLOCK! Please help!!!\"}" https://hooks.slack.com/services/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 && exit 1)
        curl -X POST -H 'Content-type: application/json' --data "{\"text\":\"Submitted $BLOCK\"}" https://hooks.slack.com/services/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
        say "Submitted" || true
    fi
    echo "Current block is $BLOCK"
    sleep 10
done


