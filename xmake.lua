add_rules("mode.debug", "mode.release")

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
