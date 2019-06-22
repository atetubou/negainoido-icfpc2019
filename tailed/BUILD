package(default_visibility = ["//visibility:public"])

cc_library(
    name = "LKH3",
    srcs = ["LKH3_wrapper.cpp"],
    hdrs = ["LKH3_wrapper.h"],
)

cc_binary(
    name = "tailed",
    srcs = ["main.cc"],
    deps = [
        ":LKH3",
        "@com_google_absl//absl/flags:flag",
        "@com_google_glog//:glog",
    ],
)
