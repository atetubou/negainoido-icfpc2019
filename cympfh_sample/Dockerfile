FROM rust:1.31-slim

WORKDIR /app
COPY . .

RUN cargo build --release
CMD "./target/release/cympfh_sample"
