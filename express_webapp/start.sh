#!/bin/bash

echo 'build web directory...'
npm --prefix ./web i ./web && npm run build --prefix ./web || exit 1
echo 'build server application...'
npm i && npm run tsc || exit 1
npm start