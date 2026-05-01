load("@rules_cc//cc:defs.bzl", "cc_test", "cc_library")

cc_library(
    name = "coarse_linked_list",
    hdrs = ["coarse_linked_list.h"],
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
        ":coarse_linked_list",   # 👈 use your library
    ],
)