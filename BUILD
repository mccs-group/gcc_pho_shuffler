load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@rules_python//python:defs.bzl", "py_library")

cc_library(
        name = "actions",
        srcs = [
                "src/actions.cc",
                "src/state_machine.cc",
                "src/file_parsing.cc",
        ],
        hdrs = [
                "include/state_machine.hh",
                "include/file_parsing.hh",
                "include/utilities.hh",
                "include/action_space.hh",
        ],
        strip_include_prefix = "include",
        copts = [
                "-O2",
                "-Wall",
                "-std=c++2a",
                "-fPIC",
                "-I./include"
        ],
        linkopts = [
                "-shared",
                "-fPIC",
        ],
        linkstatic = False,
)
py_library(
        name = "actions_py",
        srcs = [
                "__init__.py",
        ],
        visibility = ["//visibility:public"],
        data = [
                ":shuffler_deps",
        ],
)
filegroup(
        name = "shuffler_deps",
        srcs = [
                "unique_passes.txt",
                ":lists",
                ":actions",
        ],
)
