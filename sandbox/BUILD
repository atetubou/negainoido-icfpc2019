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
    srcs = ["fib_test.cc"],
    deps = [
        ":fib",
        "@com_google_googletest//:gtest_main",
    ],
)
