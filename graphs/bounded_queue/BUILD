load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "bounded_queue",
    hdrs = ["bounded_queue.h"],
    deps = ["//tl2:tl2"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "bench",
    srcs = ["bench.cc"],
    deps = [
        ":bounded_queue",
    ],
)