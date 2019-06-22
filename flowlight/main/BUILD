cc_library(
  name = "union_find",
  hdrs = ["union_find.h"]
)

cc_binary(
    name = "flowlight_solver",
    srcs = ["clone.cc"],
    deps = [
        ":union_find",
        "//base",
        "//base:ai",
        "//base:flags",
        "//base:graph",
        "@com_google_glog//:glog",
    ],
)
