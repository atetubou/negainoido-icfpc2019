FROM node:8.16.0-jessie

WORKDIR /app
ADD . .
RUN . ./setup-secrets.sh
RUN npm --prefix ./web i ./web
RUN npm run build --prefix ./web 
RUN npm i
RUN npm run tsc
CMD npm start
