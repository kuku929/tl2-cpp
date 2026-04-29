load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:defs.bzl", "cc_test", "cc_binary")

cc_binary(
    name = "bench",
    srcs = ["bounded_queue_bench.cc","bounded_queue.h"],
    deps = [
        ":bounded_queue",
    ],
)