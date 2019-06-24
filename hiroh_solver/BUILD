cc_binary(
    name = "duskin_ai",
    srcs = ["duskin_ai.cc"],
    deps = [
        "@com_google_absl//absl/types:optional",
    ],
)

cc_binary(
    name = "konmari_ai",
    srcs = ["konmari_ai.cc",
            "konmari_ai_core.cc",
            "konmari_ai_solver.h",
    ],
    copts = ["-std=c++14"],
    deps = [
        "//base:ai",
        "//base:graph",
        "@com_google_glog//:glog",
    ],
)

cc_binary(
    name = "konmari_ai_clone",
    srcs = ["konmari_ai_clone.cc",
            "konmari_ai_core.cc",
            "konmari_ai_solver.h",
    ],
    copts = ["-std=c++14"],
    deps = [
        "//base:ai",
        "//base:graph",
        "@com_google_glog//:glog",
        "@com_google_absl//absl/algorithm:container",
        "//tailed:LKH3",
    ],
)
