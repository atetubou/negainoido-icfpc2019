# negainoido-icfpc2019

masterブランチにマージするときは、pull requestを使ってください。
チェックが通れば勝手にマージされます。

[cympfh builds](https://console.cloud.google.com/cloud-build/builds?project=negainoido-icfpc2019&query=tags%3D%20%22cympfh%22)

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
