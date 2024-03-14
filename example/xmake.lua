if is_plat("android") then
    add_defines("ANDROID")
end
local pkgs = (get_config('pkg') or ''):split(';')
for _, pkg in ipairs(pkgs) do
    if pkg then
        if is_plat("android") and pkg == 'sdl2' then
            add_requires("sdl2", {configs={shared=true}})
        else
            add_requires(pkg)
        end
    end
end
add_defines("DEMO_USE_CJK")
add_defines(format('EXAMPLE_PATH="%s"', path.absolute(os.scriptdir()):gsub('\\', '/')..'/'))

target("example.sdl_flags")
    set_group("00.sdl_wrapper")
    set_kind("object")
    add_defines("NANOVG_DISABLE_GLFW", "DEMO_USE_CJK", {public = true})
    if is_plat("android") then
        add_defines("ANDROID", {public = true})
        set_kind("shared", {public = true})
    elseif is_plat("macosx") then
        add_deps("nanovg_metal", {public = true})
        add_includedirs("../src/metal", {public = true})
        add_frameworks("QuartzCore", {public = true})
    elseif is_plat("windows", "mingw") then
        add_deps("nanovg_d3d11", {public = true})
        add_files("../src/resource.rc")
        add_includedirs("../src/d3d11", {public = true})
        add_syslinks("d3d11", "dxguid", {public = true})
        if is_plat("mingw") then
            add_ldflags("-static-libgcc", "-static-libstdc++")
        end
    else
        add_defines("NANOVG_GLEW", {public = true})
        add_packages("glew", {public = true})
    end
    add_packages("sdl2", "stb", {public = true})
    add_deps("nanovg", "nanovg_wrapper", {public = true})
    add_includedirs("../src", {public = true})

target("example.sdl")
    add_files(
        "example_sdl_auto.c",
        "demo.c"
    )
    add_deps("example.sdl_flags")

target("example.gl")
    add_includedirs("../src")
    add_files(
        "example_gl3.c",
        "demo.c",
        "perf.c"
    )
    add_defines("NANOVG_USE_GLEW", "NANOVG_GL3", "NANOVG_GLEW")
    add_packages("glew", "glfw", "stb")
    add_deps("nanovg")
    if is_plat("windows", "mingw") then
        add_files("../src/resource.rc")
        if is_plat("mingw") then
            add_ldflags("-static-libgcc", "-static-libstdc++")
        end
    elseif is_plat("macosx") then
        add_frameworks("OpenGL")
    end

target("example.gles")
    if is_plat("android") then
        add_defines("ANDROID")
        set_kind("shared")
    elseif is_plat("macosx") then
        add_deps("nanovg_metal")
        add_includedirs("../src/metal")
    elseif is_plat("windows", "mingw") then
        add_deps("nanovg_d3d11")
        add_includedirs("../src/d3d11")
        add_syslinks("d3d11", "dxguid")
        -- add_defines("NANOVG_GLEW")
        -- add_packages("glew")
    else
        add_defines("NANOVG_GLEW")
        add_packages("glew")
    end
    add_defines("NANOVG_DISABLE_GLFW")
    add_includedirs("../src")
    add_files(
        "example_sdl_gles3.c"
    )
    add_packages("sdl2", "stb")
    add_deps("nanovg", "nanovg_wrapper")
    if is_plat("android") then
        if is_arch("arm64-v8a") then
            add_syslinks("GLESv3")
        else
            add_syslinks("GLESv2")
        end
    end
    if is_plat("windows", "mingw") then
        add_files("../src/resource.rc")
    elseif is_plat("macosx") then
        add_frameworks("QuartzCore", "Metal")
        -- add_frameworks("OpenGL", "QuartzCore")
    end
    if is_plat("mingw") then
        add_ldflags("-static")
    end
    after_build(function (target)
        if target:is_plat("android") then
            local outDir = path.join(os.scriptdir(), "../project/android/app/libs", target:arch()).."/"
            for _, pkg in pairs(target:pkgs()) do
                for _, f in ipairs(pkg:libraryfiles()) do
                    if f ~= nil and f:endswith(".so") then
                        os.cp(f, outDir)
                        print("cp "..f.." "..outDir)
                    end
                end
            end
            os.cp(target:targetfile(), outDir)
            return
        end
    end)

target("example.vulkan")
    add_deps("nanovg")
    add_includedirs("../src", "../src/vulkan")
    add_defines("NANOVG_DISABLE_GLFW")
    if is_plat("windows", "mingw") then
        add_includedirs(path.join(get_config("vulkan"), "include"))
        add_linkdirs(path.join(get_config("vulkan"), "Lib"))
        add_links("vulkan-1")
    elseif is_plat("macosx") then
        add_includedirs("/usr/local/include")
        add_linkdirs("/usr/local/lib")
        add_links("MoltenVK")
    end
    add_packages("glfw", "stb", "vulkan")
    add_files(
        "example_vulkan.c",
        "demo.c",
        "perf.c"
    )

if is_plat("macosx") then
target("example.metal")
    add_includedirs("../src")
    add_includedirs("../src/metal")
    add_files(
        "example_metal.mm",
        "demo.c",
        "perf.c"
    )
    add_deps("nanovg")
    add_deps("nanovg_metal")
    add_frameworks("Metal", "MetalKit", "QuartzCore")
    add_packages("glfw", "stb")
    add_defines("NANOVG_DISABLE_GLFW")
end

target("example.d3d11")
    add_includedirs("../src")
    add_includedirs("../src/d3d11")
    add_files(
        "example_d3d11.c",
        "demo.c",
        "perf.c"
    )
    add_packages("stb")
    add_deps("nanovg")
    if is_plat("windows", "mingw") then
        add_files("../src/resource.rc")
        if is_plat("mingw") then
            add_ldflags("-static-libgcc", "-static-libstdc++")
        end
    end
    add_defines("NANOVG_DISABLE_GLFW")
    add_syslinks("user32", "d3d11")


target("example.sokol")
    add_includedirs("../src")
    add_includedirs("../src/sokol")
    add_includedirs("$(buildir)/sokol_shader")
    add_defines("NANOVG_DISABLE_GLFW")
    add_files(
        "example_sokol.c",
        "demo.c",
        "frequency.c"
    )
    add_packages("stb", "sokol")
    add_deps("nanovg")
    if is_plat("mingw") then
        add_ldflags("-static-libgcc", "-static-libstdc++")
    end
    if not is_plat("macosx") then
        add_files("sokol.c")
    end
    if is_plat("windows", "mingw") then
        add_files("../src/resource.rc")
        -- add_defines("SOKOL_D3D11")
        add_defines("SOKOL_GLCORE33")
    elseif is_plat("macosx") then
        add_files("sokol.m")
        add_frameworks(
            "Appkit",
            "CoreGraphics",
            "QuartzCore"
        )
        -- add_defines("SOKOL_GLCORE33")
        -- add_frameworks("OpenGL")
        add_defines("SOKOL_METAL")
        add_frameworks(
            "Metal",
            "MetalKit"
        )
    else
        add_defines("SOKOL_GLCORE33")
    end
