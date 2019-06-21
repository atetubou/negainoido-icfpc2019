#!/bin/bash

cd $(dirname $0)

if [[ -z $1 ]]; then
    echo "
Usage:

\$ $0 <cc_binary name in BUILD>

e.g.

$ $0 tikuta_solver

" 1>&2
    exit 1
fi


target=$(bazel query "kind(\"cc_binary\", $1)")

bazel build $target

solver=./bazel-bin/$(echo $target | sed -e 's$:$/$g' -e 's$//$$g')

for task in `seq -w 1 150`
do
    $solver < part-1-initial/prob-$task.in > ans
    score=$(wc -c ans | cut -d ' ' -f 1)
    solver=$1-@$(git rev-list -n1 HEAD)
    curl -k https://negainoido.dip.jp/score/solution -F score=$score -F file=@ans -F solver=$solver -F task=$task
done

rm ans
