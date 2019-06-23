package(default_visibility = ["//visibility:public"])

cc_library(
    name = "union_find",
    hdrs = ["union_find.h"],
)

cc_library(
  name = "common",
  hdrs = ["common.h"]
)


cc_binary(
    name = "flowlight_solver",
    srcs = ["clone.cc"],
    deps = [
        ":union_find",
        ":common",
        "//base",
        "//base:ai",
        "//base:flags",
        "//base:graph",
        "//tailed:LKH3",
        "@com_google_absl//absl/flags:flag",
        "@com_google_glog//:glog",
    ],
)


cc_binary(
    name = "flowlight_solver2",
    srcs = ["clone2.cc"],
    deps = [
        ":union_find",
        ":common",
        "//base",
        "//base:ai",
        "//base:flags",
        "//base:graph",
        "//tailed:LKH3",
        "@com_google_absl//absl/flags:flag",
        "@com_google_glog//:glog",
    ],
)
