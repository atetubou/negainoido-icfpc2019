#!/bin/python3
import sys
import json

def split_input(s):
    arr = s.split('#')
    nums = [int(x) for x in arr[0].split(',')]

    include_vs_tmp = arr[1][1:-1].split('),(')
    include_vs = []
    for x in include_vs_tmp:
        tpl = list(map(int, x.split(',')))
        include_vs.append(tpl)

    exclude_vs_tmp = arr[2][1:-1].split('),(')
    exclude_vs = []
    for x in exclude_vs_tmp:
        tpl = list(map(int, x.split(',')))
        exclude_vs.append(tpl)

    include_vs.sort()
    exclude_vs.sort()

    return nums, include_vs, exclude_vs

def output_puzzle(basename, inp):
    nums, ivs, evs = split_input(inp)

    assert(len(nums) == 11)

    fname = basename + ".txt"

    with open(fname, 'w') as f:
        print("puzzle: {} is created".format(fname))
        f.write(" ".join(map(str,nums)) + "\n")
        f.write("{} {}".format(len(ivs), len(evs)) + "\n")

        for p in ivs:
            f.write("{} {}".format(p[0], p[1]) + "\n")

        for p in evs:
            f.write("{} {}".format(p[0], p[1]) + "\n")
        f.flush()

def main():
    args = sys.argv
    if len(args) < 2:
        print("Usage: {} <cond file>".format(args[0]))
        exit(1)

    fname = args[1]
    with open(fname, 'r') as f:
        inp = f.read()

    output_puzzle(fname, inp)

if __name__ == '__main__':
    main()