workspace(name = "negainoid_icfpc2019")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

### For buildfier
# buildifier is written in Go and hence needs rules_go to be built.
# See https://github.com/bazelbuild/rules_go for the up to date setup instructions.
http_archive(
    name = "io_bazel_rules_go",
    sha256 = "3743a20704efc319070957c45e24ae4626a05ba4b1d6a8961e87520296f1b676",
    url = "https://github.com/bazelbuild/rules_go/releases/download/0.18.4/rules_go-0.18.4.tar.gz",
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains()

http_archive(
    name = "com_github_bazelbuild_buildtools",
    sha256 = "c3cd67954de9c1971e37d8e0abeccef4110bbf9fd3fa9886ffc2ef3b73d9ecbe",
    strip_prefix = "buildtools-882724efbd6169961bac0932892bcc0281c6d6f5",
    url = "https://github.com/bazelbuild/buildtools/archive/882724efbd6169961bac0932892bcc0281c6d6f5.tar.gz",
)

load("@com_github_bazelbuild_buildtools//buildifier:deps.bzl", "buildifier_dependencies")

buildifier_dependencies()

### For main programs
http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = [
        "https://github.com/gflags/gflags/archive/v2.2.2.tar.gz",
    ],
)

http_archive(
    name = "com_google_glog",
    sha256 = "f28359aeba12f30d73d9e4711ef356dc842886968112162bc73002645139c39c",
    strip_prefix = "glog-0.4.0",
    urls = [
        "https://github.com/google/glog/archive/v0.4.0.tar.gz",
    ],
)

http_archive(
    name = "com_google_absl",
    sha256 = "c44f5a87695925aa0c9c4a207c7b4d77c21011f9627717337827fe25ccb867a2",
    strip_prefix = "abseil-cpp-0cbdc774b97f7e80ab60dbe2ed4eaca3b2e33fc8",
    urls = [
        "https://github.com/abseil/abseil-cpp/archive/0cbdc774b97f7e80ab60dbe2ed4eaca3b2e33fc8.tar.gz",
    ],
)
