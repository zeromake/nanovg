add_defines("DEMO_USE_CJK")

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
