#!/bin/bash


names() {
    for d in cympfh*; do
        if [ ! -f $d/Dockerfile ]; then
            continue
        fi
        echo $d | sed 'h;s/_/:/g;G;s/\n/ /g'
    done
}

template() {

    cat <<EOM
steps:
EOM

    names |
        while read name dir; do
            cat <<EOM
- name: 'gcr.io/cloud-builders/docker'
  entrypoint: 'bash'
  args: ['-c', 'docker pull gcr.io/\$PROJECT_ID/$name || true']
- name: 'gcr.io/cloud-builders/docker'
  args: [
  'build', '-t', 'gcr.io/\$PROJECT_ID/$name',
  '--cache-from', 'gcr.io/\$PROJECT_ID/cympfh:latest',
  '$dir'
  ]
- name: 'gcr.io/cloud-builders/docker'
  args: ['run', 'gcr.io/\$PROJECT_ID/$name']
EOM
        done

    cat <<EOM
images:
EOM

    names |
        while read name dir; do
            echo "- 'gcr.io/\$PROJECT_ID/$name'"
        done

    cat <<EOM
tags:
- "cympfh"
EOM

}

template > cympfhbuild.yaml
