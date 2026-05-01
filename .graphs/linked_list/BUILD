load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "linked_list_impls",
    hdrs = [
        "coarse_list.h",
        "fine_list.h",
        "lockfree_list.h",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "bench",
    srcs = ["bench.cc"],
    deps = [
        ":linked_list_impls",
        "//zoo:zoo",
    ],
)
