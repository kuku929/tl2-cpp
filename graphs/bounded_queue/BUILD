load("@rules_cc//cc:cc_test.bzl", "cc_test")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:defs.bzl", "cc_test")
cc_library(
    name = "bounded_queue",
    hdrs = ["bounded_queue.h",],
    deps=["//tl2:tl2",],
    includes = ["."],
    visibility = ["//visibility:public"],
)
cc_test(
    name = "bench",
    size = "small",
    srcs = ["bench.cc","bounded_queue.h",],
    deps = [
        "//tl2:tl2"
    ],
)