add_rules("mode.debug", "mode.release")

add_requires("glew")
add_requires("glfw")
add_requires("stb")
-- add_requires("sdl2")

add_defines("DEMO_USE_CJK")

target("nanovg")
    set_kind("static")
    add_files(
        "src/nanovg.c"
    )
    add_packages("stb")

target("nanovg_awtk")
    set_kind("static")
    add_files(
        "src/awtk/base/nanovg.c"
    )
    add_packages("stb")
    add_defines("WITH_NANOVG_GPU")

target("nanovg_inniyah")
    set_kind("static")
    add_files(
        "src/inniyah/nanovg.c",
        "src/inniyah/android.c"
    )
    add_packages("stb")

target("nanovg_inniyah_gl")
    add_includedirs("src/inniyah")
    set_kind("static")
    add_defines("NANOVG_USE_GLEW", "NANOVG_GL3")
    add_files(
        "src/inniyah/nanovg_gl.c",
        "src/inniyah/nanovg_gl_utils.c"
    )
    add_packages("stb", "glew")


target("nanovg_metal")
    -- add_includedirs("src/awtk/base")
    add_includedirs("src")
    set_kind("static")
    add_files("src/metal/nanovg_mtl.m")

target("example")
    add_includedirs("src")
    add_files(
        "example/example_gl3.c",
        "example/demo.c",
        "example/perf.c"
    )
    add_defines("NANOVG_USE_GLEW", "NANOVG_GL3", "NANOVG_GLEW")
    add_packages("glew", "glfw", "stb")
    add_deps("nanovg")
    if is_plat("windows", "mingw") then
        add_files("src/resource.rc")
        if is_plat("mingw") then
            add_ldflags("-static-libgcc", "-static-libstdc++")
        end
    elseif is_plat("macosx") then
        add_frameworks("OpenGL")
    end

target("example_inniyah")
    add_includedirs("src/inniyah")
    add_files(
        "example/example_gl3.c",
        "example/demo.c",
        "example/perf.c"
    )
    add_packages("glew", "glfw", "stb")
    add_frameworks("OpenGL")
    add_defines("NANOVG_USE_GLEW", "NANOVG_GL3", "NANOVG_GLEW")
    add_deps("nanovg_inniyah", "nanovg_inniyah_gl")
    if is_plat("windows", "mingw") then
        add_files("src/resource.rc")
    end

target("example_awtk")
    add_includedirs("src/awtk/base", "src/awtk/gl")
    add_files(
        "example/example_gl3.c",
        "example/demo.c",
        "example/perf.c"
    )
    add_packages("glew", "glfw", "stb")
    add_frameworks("OpenGL")
    add_deps("nanovg_awtk")
    add_defines("NANOVG_GLEW")
    if is_plat("windows", "mingw") then
        add_files("src/resource.rc")
    end

target("example_metal")
    add_includedirs("src")
    -- add_includedirs("src/awtk/base")
    add_includedirs("src/metal")
    add_files("example/example_metal.mm")
    add_files(
        "example/demo.c",
        "example/perf.c"
    )
    add_deps("nanovg")
    add_deps("nanovg_metal")
    -- add_deps("nanovg_awtk")
    add_frameworks("Metal", "MetalKit", "QuartzCore")
    add_packages("glfw", "stb")
    add_defines("NANOVG_DISABLE_GL")

target("example_sdl2")
    add_includedirs("src")
    -- add_includedirs("src/awtk/base", "src/awtk/gl")
    add_files(
        "example/example_sdl2.c"
    )
    add_files(
        "example/demo.c",
        "example/perf.c"
    )
    add_packages("sdl2", "stb", "glew", "glfw")
    add_defines("NANOVG_GL3", "NANOVG_GL3_IMPLEMENTATION", "NANOVG_GLEW")
    add_deps("nanovg")
    -- add_deps("nanovg_awtk")
    add_frameworks("OpenGL")
    if is_plat("windows", "mingw") then
        add_files("src/resource.rc")
    end

target("example_d3d11")
    add_includedirs("src")
    add_includedirs("src/d3d11")
    -- add_includedirs("src/awtk/base", "src/awtk/gl")
    add_files(
        "example/example_d3d11.c"
    )
    add_files(
        "example/demo.c",
        "example/perf.c"
    )
    add_packages("stb")
    add_deps("nanovg")
    -- add_deps("nanovg_awtk")
    if is_plat("windows", "mingw") then
        add_files("src/resource.rc")
        if is_plat("mingw") then
            add_ldflags("-static-libgcc", "-static-libstdc++")
        end
    end
    add_defines("NANOVG_DISABLE_GL")
    add_syslinks("user32", "d3d11")
