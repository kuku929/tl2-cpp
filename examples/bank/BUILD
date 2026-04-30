load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "transaction",
    srcs = ["transaction.cc"],
    hdrs = ["transaction.h"],
    deps = [
        "//tl2:tl2"
    ]
)

cc_library(
    name = "server",
    srcs = ["server.cc"],
    hdrs = ["server.h"],
    deps = [
        ":transaction"
    ]
)

cc_binary(
    name = "transfer",
    srcs = ["//examples/bank:main.cc"],
    deps = [
        ":server"
    ],
)