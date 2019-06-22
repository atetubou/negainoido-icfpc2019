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

for task in `seq -w 1 220`
do
    $solver < problems/prob-$task.in > ans
    # score=$(grep -o '[A-Z]' ans | wc -l)
    result=0
    score=$(python official_sim/main.py problems/prob-$task.desc ans) || result=$?
    if [ ! "$result" = "0" ]; then
        echo "ERROR: Invalid result for prob-${task}.in!!!!!!!!!!!!!! Skip!"
        continue
    fi
    solver_name=$1-@$(git rev-list -n1 HEAD)
    curl -k https://negainoido.dip.jp/score/solution -F score=$score -F file=@ans -F solver="${solver_name}" -F task=$task
done

rm ans
