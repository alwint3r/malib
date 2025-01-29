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
        .files = &[_][]const u8{ "placeholders.cpp", "Token.cpp" },
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

    const test_module = b.addModule("test", .{
        .target = target,
        .optimize = optimize,
        .link_libcpp = true,
    });

    test_module.addCSourceFiles(.{
        .root = b.path("test"),
        .files = &[_][]const u8{
            "test.cpp",
            "test_FixedStringBuffer.cpp",
            "test_Token.cpp",
            "test_Tokenizer.cpp",
            "test_RingBuffer.cpp",
        },
        .flags = &[_][]const u8{
            "-std=c++23",
            "-I",
            "unity/src",
            "-I",
            "src",
        },
    });

    const test_exe = b.addExecutable(.{
        .name = "test",
        .root_module = test_module,
    });

    test_exe.linkLibrary(lib);
    test_exe.linkLibrary(unity_lib);

    b.installArtifact(test_exe);

    const unity_cmd = b.addRunArtifact(test_exe);
    unity_cmd.step.dependOn(b.getInstallStep());

    const unity_step = b.step("unity_test", "Run Unity test");
    unity_step.dependOn(&unity_cmd.step);
}
