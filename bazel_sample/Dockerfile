FROM l.gcr.io/google/bazel:0.17.1 AS build-env

WORKDIR /app
COPY . .

RUN bazel build //main:hello-world

FROM gcr.io/distroless/cc
COPY --from=build-env /app/bazel-bin/main/hello-world /bin/hello-world
CMD ["/bin/hello-world"]
