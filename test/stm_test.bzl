load("@rules_cc//cc:cc_test.bzl", "cc_test")
def _stm_test_impl(name, visibility, srcs):
    cc_test(
        name = name + "_asan",
        size = "small",
        srcs = srcs,
        deps = [
            "@googletest//:gtest",
            "@googletest//:gtest_main",
            "//tl2:tl2",
        ],
        cxxopts = [
            "-fsanitize=address",
            "-fno-omit-frame-pointer",
            "-g",
            "-O2",
        ],
        linkopts = [
            "-fsanitize=address",
        ],
        visibility = visibility
    )

        # TSan version
    cc_test(
        name = name + "_tsan",
        size = "small",
        srcs = srcs,
        deps = [
            "@googletest//:gtest",
            "@googletest//:gtest_main",
            "//tl2:tl2",
        ],
        cxxopts = [
            "-fsanitize=thread",
            "-fno-omit-frame-pointer",
            "-g",
            "-O1",  # better for TSan
        ],
        linkopts = [
            "-fsanitize=thread",
        ],
        visibility = visibility
    )

stm_test = macro(
    attrs = {
        "srcs": attr.label_list(mandatory = True, doc = "Source files"),
    },
    implementation = _stm_test_impl,
)
