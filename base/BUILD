package(default_visibility = ["//visibility:public"])

cc_library(
    name = "base",
    srcs = ["base.cc"],
    hdrs = ["base.h"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_glog//:glog",
        "@jsoncpp_git//:jsoncpp",
    ],
)

cc_library(
    name = "flags",
    hdrs = ["flags.h"],
    deps = [
        "@com_github_gflags_gflags//:gflags",
    ],
)

cc_library(
    name = "geometry",
    srcs = ["geometry.cc"],
    hdrs = ["geometry.h"],
    deps = [
    ],
)

cc_library(
    name = "ai",
    srcs = ["ai.cc"],
    hdrs = ["ai.h"],
    deps = [
        ":base",
        ":geometry",
        "@com_google_absl//absl/strings",
        "@com_google_glog//:glog",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        ":ai",
    ],
)
