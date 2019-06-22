#!/bin/bash

npm --prefix ./web i ./web && npm run build --prefix ./web || exit 1
IMAGE_ID=$(docker build -q --no-cache .)
OLD_IMAGE_ID=$(docker ps -f "name=score_server" --format "{{.Image}}")
docker stop score_server && docker rmi $OLD_IMAGE_ID
docker run --name=score_server --env-file=./env.txt -v /dev/shm:/dev/shm --rm  -d -p 3000:3000 $IMAGE_ID
