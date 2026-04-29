load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
cc_binary(
    name = "counter",
    srcs = ["//perf:profile_counter.cc"],
    deps = [
        "//tl2:tl2"
    ],
)

cc_binary(
    name = "queue",
    srcs = ["//perf:profile_queue.cc"],
    deps = [
        "//tl2:tl2"
    ],
)

cc_binary(
    name = "swap",
    srcs = ["//perf:profile_swap.cc"],
    deps = [
        "//tl2:tl2"
    ],
    # cxxopts = ["-O2"]
)

cc_binary(
    name = "array",
    srcs = ["//perf:profile_array.cc"],
    deps = [
        "//tl2:tl2",
        "@googletest//:gtest"
    ],
)