const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});
    const target = b.standardTargetOptions(.{});

    const lib_module = b.addModule("malib", .{
        .target = target,
        .optimize = optimize,
        .link_libcpp = true,
    });

    const lib_cppflags = &[_][]const u8{"-std=c++23"};

    lib_module.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &[_][]const u8{"StaticStringBuffer.cpp"},
        .flags = lib_cppflags,
    });

    const lib = b.addStaticLibrary(.{
        .name = "malib",
        .root_module = lib_module,
    });

    // Unity
    const unity = b.addModule("unity", .{
        .target = target,
        .optimize = optimize,
    });

    unity.addCSourceFiles(.{
        .root = b.path("unity/src"),
        .files = &[_][]const u8{"unity.c"},
        .flags = &[_][]const u8{"-std=c11"},
    });

    const unity_lib = b.addStaticLibrary(.{
        .name = "unity",
        .root_module = unity,
    });

    b.installArtifact(lib);
    b.installArtifact(unity_lib);
}
