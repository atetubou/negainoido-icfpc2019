#!/bin/bash

IMAGE_ID=$(docker build -q .)
docker stop score_server
docker run --name=score_server --env-file=./env.txt --rm  -d -p 3000:3000 $IMAGE_ID
