load("@rules_cc//cc:defs.bzl", "cc_test", "cc_library")

cc_library(
    name = "coarse_stack",
    hdrs = ["coarse_stack.h"],
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
        ":coarse_stack",   # 👈 use your library
    ],
)