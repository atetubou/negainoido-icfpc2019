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
        notify-send -u normal -t 2000 "New Block $BLOCK starts"
    fi
    sleep 10
done


