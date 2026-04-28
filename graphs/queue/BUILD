load("@rules_cc//cc:defs.bzl", "cc_test", "cc_library")

cc_library(
    name = "queue",
    hdrs = ["queue.h"],
    deps = [
        "//tl2:tl2",   # if queue.h uses tl2
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "bench",
    size = "small",
    srcs = ["bench.cc"],
    deps = [
        ":queue",   # 👈 use your library
    ],
)

cc_test(
    name = "batch_bench",
    size = "small",
    srcs = ["batch_bench.cc"],
    deps = [
        ":queue",   # 👈 same here
    ],
)