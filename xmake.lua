add_rules("mode.debug", "mode.release")

option("example")
    set_default(false)
    set_showmenu(true)
option_end()

option("freetype")
    set_default(false)
    set_showmenu(true)
option_end()

option("vulkan")
    set_default("")
    set_showmenu(true)
option_end()

option("pkg")
    set_default("sdl2;glew;glfw;sokol")
    set_showmenu(true)
    set_description("require pkg")
option_end()


add_repositories("zeromake https://github.com/zeromake/xrepo.git")

add_requires("stb")
add_defines("NVG_USE_SHD_SHADER")

rule("sokol_shader")
    set_extensions(".glsl")
    on_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        local _targetfile = path.relative(sourcefile, "src")
        batchcmds:mkdir("$(buildir)/sokol_shader")
        local targetfile = vformat(path.join("$(buildir)/sokol_shader", _targetfile..".h"))
        -- USE_UNIFORMBUFFER
        local defines = "--defines=USE_SOKOL"
        if string.find(sourcefile, ".aa.glsl") then
            defines = defines..":EDGE_AA"
        end
        -- local shdc = "/Users/zero/Downloads/fips-deploy/sokol-tools/osx-ninja-release/sokol-shdc"
        local shdc = "sokol-shdc"
        batchcmds:vrunv(shdc, {
            "--ifdef",
            "-l",
            "hlsl5:glsl330:glsl300es:metal_macos:metal_ios",
            defines,
            "--input",
            sourcefile,
            "--output",
            targetfile,
        })
        batchcmds:show_progress(opt.progress, "${color.build.object}glsl %s", sourcefile)
        batchcmds:add_depfiles(sourcefile)
    end)
rule_end()

if get_config("freetype") then
    add_requires("freetype", {system=false,configs={
        zlib=true,
        bzip2=true,
        brotli=true,
        png=true
    }})
end

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
    if get_config("freetype") then
        add_defines("FONS_USE_FREETYPE")
        add_packages("freetype")
    end
target_end()

if is_plat("macosx") then
target("nanovg_metal")
    set_kind("$(kind)")
    add_includedirs("src")
    add_headerfiles("src/metal/*.h")
    add_files("src/metal/nanovg_mtl.m")
    add_files("src/metal/metal_helper.mm")
target_end()
end

target("nanovg_d3d11")
    set_kind("$(kind)")
    add_includedirs("src")
    add_headerfiles("src/d3d11/*.h")
    add_headerfiles("src/d3d11/nvg_shader/*.h", {prefixdir="nvg_shader"})
    add_files("src/d3d11/d3d11_helper.c")
target_end()

target("nanovg_wrapper")
    set_kind("object")
    if is_plat("macosx") then
        add_files("src/nanovg_wrapper.mm")
    end
target_end()

add_rules("sokol_shader")
target("sokol_shader")
    set_kind("object")
    add_files("src/shd.glsl", "src/shd.aa.glsl")
    add_files()
target_end()

if get_config("example") then
    includes("example/xmake.lua")
end
