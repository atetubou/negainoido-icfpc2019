FROM node:8.16.0-jessie

WORKDIR /app
ADD . .
RUN npm i
RUN npm run tsc
CMD npm start
