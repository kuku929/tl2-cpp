load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "tl2",
    hdrs = ["tl2.h", 
            "hash_table.h",
            "read_set.h",
            "write_set.h",
            "stm.h",
            "t_var.h",
            "types.h",
            "version_lock.h",
            "log.h",
            "state.h",
            "memory.h"
            ],
    includes = ["."],
    deps = ["@unordered_dense//:unordered_dense"],
    visibility = ["//visibility:public"],
)
