cc_binary(
    name = "duskin_ai",
    srcs = ["duskin_ai.cc"],
    deps = [
        "@com_google_absl//absl/types:optional",
    ],
)

cc_binary(
    name = "konmari_ai",
    srcs = ["konmari_ai.cc"],
    copts = ["-std=c++14"],
    deps = [
        "//base:ai",
        "//base:graph",
        "@com_google_glog//:glog",
    ],
)
