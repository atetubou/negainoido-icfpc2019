cc_library(
  name = "union_find",
  hdrs = ["union_find.h"]
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
        "@com_google_glog//:glog",
    ],
)

cc_binary(
    name = "puzzle_solver",
    srcs = ["puzzle_solver.cc"],
    deps = [
        "//base:graph",
        ":common",
        "@com_google_glog//:glog",
    ],              
)

