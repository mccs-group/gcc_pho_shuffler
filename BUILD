load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@rules_python//python:defs.bzl", "py_library")

cc_library(
        name = "file_parsing",
        srcs = [
                "src/file_parsing.cc",
        ],
        hdrs = [
                "include/utilities.hh",
                "include/file_parsing.hh",
        ],
        strip_include_prefix = "include",
        copts = [
                "-O2",
                "-Wall",
                "-std=c++2a",
                "-fPIC",
                "-I./include"
        ],
)

cc_library(
        name = "state_machine",
        srcs = [
                "src/state_machine.cc",
        ],
        hdrs = [
                "include/utilities.hh",
                "include/state_machine.hh",
                "include/file_parsing.hh",
        ],
        strip_include_prefix = "include",
        copts = [
                "-O2",
                "-Wall",
                "-std=c++2a",
                "-fPIC",
                "-I./include"
        ],
)

cc_library(
        name = "actions",
        srcs = [
                "src/actions.cc",
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
)

cc_shared_library(
        name = "actions_lib",
        shared_lib_name = "actions.so",
        roots = [
                ":file_parsing",
                ":state_machine",
                ":actions",
        ],
)

py_library(
        name = "actions_py",
        srcs = [
                "__init__.py",
        ],
        visibility = ["//visibility:public"],
        data = [
                ":actions_lib",
                ":shuffler_deps",
        ],
)
filegroup(
        name = "shuffler_deps",
        srcs = [
                "unique_passes.txt",
                ":lists",
        ],
)
