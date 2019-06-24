# Team negainoido

## Member
* Hirokazu Honda
* Kazuhiro Hattori
* Keiichi Watanabe
* Shuichi Hirahara
* Takanori Hayashi
* Taku Terao
* Takumi Shimada
* Takuto Ikuta

## Programming Language
* C++
* Rust
* Hand
* TypeScript
* Python3
* ShellScript

# AI

## tikuta3

How to run
```
$ bazel build //...
$ ./bazel-bin/tikuta_solver/tikuta_solver3 --buy=CCC < problems/prob-002.desc > problems/prob-002.out
```

## cym

An editor writtein in Rust, solves with your Hand.

```
$ cd problems/
$ python ./convert_readable.py ./prob-002.desc
$ cd ../cympfh/
$ cargo run --release ../problems/prob-002.in
```

