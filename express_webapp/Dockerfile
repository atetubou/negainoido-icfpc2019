FROM node:8.16.0-jessie

WORKDIR /app
ADD web web
RUN npm --prefix ./web i ./web
RUN npm run build --prefix ./web

COPY package.json .
RUN npm i
ADD . .
RUN npm run tsc
CMD npm start
