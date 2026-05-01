load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "transaction",
    srcs = ["transaction.cc", "account.h"],
    hdrs = ["transaction.h"],
    deps = [
        "//tl2:tl2",
        "//zoo:hash_map"
    ]
)

cc_library(
    name = "server",
    srcs = ["server.cc"],
    hdrs = ["server.h", "account.h"],
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