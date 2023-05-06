add_rules("mode.debug", "mode.release")

option("example")
    set_default(false)
    set_showmenu(true)
option_end()

add_requires("stb")

if is_plat("windows") then
    add_cxflags("/utf-8")
end

target("nanovg")
    set_kind("$(kind)")
    add_files(
        "src/nanovg.c"
    )
    add_packages("stb")
    add_headerfiles("src/*.h")

if is_plat("macosx") then
target("nanovg_metal")
    set_kind("$(kind)")
    add_includedirs("src")
    add_headerfiles("src/metal/*.h")
    add_headerfiles("src/metal/mnvg_bitcode/*.h", {prefixdir="mnvg_bitcode"})
    add_files("src/metal/nanovg_mtl.m")
end

target("nanovg_d3d11")
    set_kind("headeronly")
    add_headerfiles("src/d3d11/*.h")
    add_headerfiles("src/d3d11/nvg_shader/*.h", {prefixdir="nvg_shader"})
target_end()

if get_config("example") then
    add_defines("DEMO_USE_CJK")
    add_defines(format('EXAMPLE_PATH="%s"', path.absolute(path.join(os.scriptdir(), "example")):gsub('\\', '/')..'/'))
    if is_plat("android") then
        add_requires("sdl2", {configs={shared=true}})
    else
        add_requires("glew", "sdl2", "glfw")
    end

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
    target_end()

    target("example_gles")
        if is_plat("android") then
            add_defines("ANDROID")
            set_kind("shared")
        else
            set_kind("binary")
        end
        add_includedirs("src")
        add_files(
            "example/example_sdl_gles3.c"
        )
        add_packages("sdl2", "stb")
        add_deps("nanovg")
        if is_plat("windows", "mingw") then
            add_files("src/resource.rc")
            if is_plat("mingw") then
                add_ldflags("-static-libgcc", "-static-libstdc++")
            end
        elseif is_plat("macosx") then
            add_frameworks("OpenGL")
        end
        after_build(function (target)
            if target:is_plat("android") then
                local outDir = "project/android/app/libs/"..target:arch().."/"
                for _, pkg in pairs(target:pkgs()) do
                    if pkg:has_shared() then
                        for _, f in ipairs(pkg:libraryfiles()) do
                            if f ~= nil and f:endswith(".so") then
                                os.cp(f, outDir)
                                print("cp "..f.." "..outDir)
                            end
                        end
                    end
                end
                os.cp(target:targetfile(), outDir)
                return
            end
        end)
    target_end()

    if is_plat("macosx") then
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
        target_end()
    end

    target("example_d3d11")
        add_includedirs("src")
        add_includedirs("src/d3d11")
        add_files(
            "example/example_d3d11.c"
        )
        add_files(
            "example/demo.c",
            "example/perf.c"
        )
        add_packages("stb")
        add_deps("nanovg")
        if is_plat("windows", "mingw") then
            add_files("src/resource.manifest")
            if is_plat("mingw") then
                add_ldflags("-static-libgcc", "-static-libstdc++")
            end
        end
        add_defines("NANOVG_DISABLE_GL")
        add_syslinks("user32", "d3d11")
    target_end()

    package("mpv")
        if is_plat("windows", "mingw") then
            set_urls("https://github.com/zhongfly/mpv-winbuild/releases/download/2023-03-01/mpv-dev-x86_64-20230301-git-779d4f9.7z")
            add_versions("latest", "5b49e682548f40169dc52f8844344ad779d06f6656934753521e21b1e8e3e2d7")
        end
        add_links("mpv")
        on_install("windows", "mingw", function (package)
            import("detect.sdks.find_vstudio")
            os.cp("include/*", package:installdir("include").."/")
            os.cp("*.a", package:installdir("lib").."/")
            os.cp("*.dll", package:installdir("bin").."/")

            -- 从 dll 里导出函数为 lib 文件，预编译自带 def 文件格式不正确，没法导出 lib
            if os.isfile("mpv.def") then
                local def_context = io.readfile("mpv.def")
                if not def_context:startswith("EXPORTS") then
                    io.writefile("mpv.def", format("EXPORTS\n%s", def_context))
                end
            end
            local vs = find_vstudio()["2022"]["vcvarsall"]["x64"]
            local libExec = path.join(
                vs["VSInstallDir"],
                "VC",
                "Tools",
                "MSVC",
                vs["VCToolsVersion"],
                "bin",
                "HostX64",
                "x64",
                "lib.exe"
            )
            os.execv(libExec, {"/name:libmpv-2.dll", "/def:mpv.def", "/out:mpv.lib", "/MACHINE:X64"})
            os.cp("*.lib", package:installdir("lib").."/")
            os.cp("*.exp", package:installdir("lib").."/")
        end)
    package_end()

    target("demo/sw")
        add_includedirs("src")
        add_includedirs("src/d3d11")
        add_packages(
            "mpv",
            "sdl2"
        )
        add_defines("__SDL2__")
        if is_plat("windows", "mingw") then
            add_files("src/resource.rc")
            if is_plat("mingw") then
                add_ldflags("-static-libgcc", "-static-libstdc++")
            end
        end
        add_files("example/sw.cpp")
        add_files("example/d3d11.cpp")
        add_deps("nanovg")
        add_syslinks("d3d11")
    target_end()
end