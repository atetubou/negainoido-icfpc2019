# negainoido-icfpc2019

masterブランチにマージするときは、pull requestを使ってください。
チェックが通れば勝手にマージされます。

[TOC]

## bazel

example commands.

### run

run specified target.

```
bazel run //sandbox:hello_world
```

### build

build all buildable targets.

```
bazel build //...
```

build specified target.

```
bazel build //sandbox:fib
```

### test

test all tetable targets.

```
bazel test //...
```

test specified target.

```
bazel build //sandbox:fib_test
```
