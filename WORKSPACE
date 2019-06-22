workspace(name = "negainoid_icfpc2019")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

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
    sha256 = "700ad2f65fe00f7b0d293016d2ac865af9ad038b64a334433a0931c691223a85",
    strip_prefix = "abseil-cpp-0238ab0a831f179518c1a814f9584e99da2d75a3",
    urls = [
        "https://github.com/abseil/abseil-cpp/archive/0238ab0a831f179518c1a814f9584e99da2d75a3.tar.gz",
    ],
)

http_archive(
    name = "com_google_googletest",
    sha256 = "927827c183d01734cc5cfef85e0ff3f5a92ffe6188e0d18e909c5efebf28a0c7",
    strip_prefix = "googletest-release-1.8.1",
    urls = ["https://github.com/google/googletest/archive/release-1.8.1.zip"],
)

http_archive(
    name = "jsoncpp_git",
    build_file = "@//:third_party/jsoncpp.BUILD",
    sha256 = "c49deac9e0933bcb7044f08516861a2d560988540b23de2ac1ad443b219afdb6",
    strip_prefix = "jsoncpp-1.8.4",
    urls = [
        "https://github.com/open-source-parsers/jsoncpp/archive/1.8.4.tar.gz",
    ],
)
