package(default_visibility = ["//visibility:public"])

cc_library(
    name = "base",
    hdrs = ["base.h"],
    srcs = ["base.cc"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_glog//:glog",
        "@jsoncpp_git//:jsoncpp",
    ]
)

cc_library(
    name = "flags",
    hdrs = ["flags.h"],
    deps = [
        "@com_github_gflags_gflags//:gflags",
    ]
)

cc_library(
    name = "geometry",
    hdrs = ["geometry.h"],
    srcs = ["geometry.cc"],
    deps = [
    ]
)

cc_library(
    name = "ai",
    hdrs = ["ai.h"],
    srcs = ["ai.cc"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_glog//:glog",
        ":base",
        ":geometry",
    ]
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        ":ai",
    ]
)
