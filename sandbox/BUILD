cc_binary(
    name = "hello_world",
    srcs = ["hello_world.cc"],
    deps = [
        "@com_google_absl//absl/flags:flag",
        "@com_google_glog//:glog",
    ],
)

cc_library(
    name = "fib",
    srcs = ["fib.cc"],
    hdrs = ["fib.h"],
)

cc_test(
    name = "fib_test",
    size = "small",
    srcs = ["fib_test.cc"],
    deps = [
        ":fib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "json_test",
    size = "small",
    srcs = ["json_test.cc"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
        "@jsoncpp_git//:jsoncpp",
    ],
)
