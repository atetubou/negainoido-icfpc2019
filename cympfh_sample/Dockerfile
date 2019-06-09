FROM base
WORKDIR /app
COPY . .

RUN cargo build --release

FROM gcr.io/distroless/cc
COPY --from=build-env /app/target/release/cympfh_sample /bin/cympfh_sample
CMD ["/bin/cympfh_sample"]
