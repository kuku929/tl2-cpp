load("@rules_cc//cc:cc_test.bzl", "cc_test")
cc_test(
    name = "two_threads_counter",
    size = "small",
    srcs = ["two_threads_counter.cc"],
    deps = [
        "@googletest//:gtest",
        "@googletest//:gtest_main",
        "//tl2:tl2"
    ],
)