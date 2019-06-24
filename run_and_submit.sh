#!/bin/bash

set -e

cd $(dirname $0)

if [[ -z $1 ]]; then
    echo "
Usage:

$ $0 <main source file>

e.g.

$ $0 tikuta_solver/main.cc

" 1>&2
    exit 1
fi


fullname=$(bazel query $1)
target=$(bazel query "attr('srcs', $fullname, ${fullname//:*/}:*)")

bazel build $target

solver=./bazel-bin/$(echo $target | sed -e 's$:$/$g' -e 's$//$$g')
validator=./bazel-bin/binary_validator/binary_validator

for task in `seq -w 1 999`
do
    if [ ! -f problems/prob-$task.in ]; then
        continue
    fi
    $solver < problems/prob-$task.in > problems/prob-$task.out
    echo "prob-${task}.in"
    solver_name=$1-@$(git rev-list -n1 HEAD)
    curl -k https://negainoido.dip.jp/score/solution -F score=$score -F file=@problems/prob-$task.out -F solver="${solver_name}" -F task=$task
    ## if you want to use buy file, please replace "buy" with your buy file name.
    # curl -k https://negainoido.dip.jp/score/solution -F score=$score -F file=@ans -F buyFile=@buy -F solver="${solver_name}" -F task=$task
done
