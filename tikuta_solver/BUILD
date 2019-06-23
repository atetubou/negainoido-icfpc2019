cc_binary(
    name = "tikuta_solver",
    srcs = ["main.cc"],
    deps = [
        "@com_google_absl//absl/flags:flag",
        "@com_google_glog//:glog",
    ],
)

cc_binary(
    name = "tikuta_solver2",
    srcs = ["main2.cc"],
    deps = [
        "//base:ai",
        "//base:graph",
        "//tailed:LKH3",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_google_glog//:glog",
    ],
)


cc_binary(
    name = "tikuta_solver3",
    srcs = ["main3.cc"],
    deps = [
        "//base:ai",
        "//base:graph",
        "//tailed:LKH3",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_glog//:glog",
    ],
)
