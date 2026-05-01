load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_library(
    name = "ordered_linked_list",
    hdrs = ["ordered_ll.h"],
    deps = ["//tl2:tl2"],
    visibility = ["//visibility:public"],
)
cc_library(
    name = "linked_list",
    hdrs = ["linked_list.h"],
    deps = ["//tl2:tl2"],
    visibility = ["//visibility:public"],
)
cc_library(
    name = "hash_map",
    hdrs = ["hash_map.h"],
    deps = ["//tl2:tl2", ":linked_list"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "linked_list_test",
    size = "small",
    srcs = ["linked_list_test.cc"],
    deps = [
        "@googletest//:gtest",
        "@googletest//:gtest_main",
        "//tl2:tl2",
        ":linked_list",
    ],
)

cc_test(
    name = "ordered_ll_test",
    size = "small",
    srcs = ["ordered_ll_test.cc"],
    deps = [
        "@googletest//:gtest",
        "@googletest//:gtest_main",
        "//tl2:tl2",
        ":ordered_linked_list",
    ],
)

cc_test(
    name = "hash_map_test",
    size = "small",
    srcs = ["hash_map_test.cc"],
    deps = [
        "@googletest//:gtest",
        "@googletest//:gtest_main",
        "//tl2:tl2",
        ":hash_map",
    ],
)