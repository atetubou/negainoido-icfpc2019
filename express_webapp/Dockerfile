FROM score_server_base
WORKDIR /app
ADD web web
RUN npm --prefix ./web i ./web
RUN npm run build --prefix ./web
ADD web/dist web/dist

ENV PATH $PATH:/app/bin
COPY package.json .
RUN npm i
ADD . .
RUN npm run tsc
CMD npm start
