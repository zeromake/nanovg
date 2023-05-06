import("net.http")

local languages = {
    "glsl330",
    "glsl300es",
    "hlsl5",
    "metal_macos",
    "metal_ios",
    "metal_sim"
}

local shdc = "sokol-shdc"

if os.host() == "windows" then
    shdc = shdc .. ".exe"
end

local downloads = {
    windows="https://raw.githubusercontent.com/floooh/sokol-tools-bin/master/bin/win32/sokol-shdc.exe",
    osx="https://raw.githubusercontent.com/floooh/sokol-tools-bin/master/bin/osx/sokol-shdc",
    linux="https://raw.githubusercontent.com/floooh/sokol-tools-bin/master/bin/linux/sokol-shdc",
}

function main()
    local shdc_path = path.absolute(path.join(os.scriptdir(), "../build/bin", shdc))
    local shdc_shader_path = path.absolute(path.join(os.scriptdir(), "../src/shd.glsl"))
    if not os.exists(shdc_path) then
        local url = downloads[os.host()]
        print("download: "..url.." -> "..shdc_path)
        http.download(url, shdc_path)
    end
    os.mkdir(path.join(os.scriptdir(), "../build/shader"))
    os.mkdir(path.join(os.scriptdir(), "../build/shader_aa"))
    os.cd(path.join(os.scriptdir(), "../build/shader"))
    for _, language in ipairs(languages) do
        os.vexecv(shdc_path, {"-i", shdc_shader_path, "-l", language, "-f", "bare", "-o", "._"})
    end
    os.cd("-")
    os.cd(path.join(os.scriptdir(), "../build/shader_aa"))
    for _, language in ipairs(languages) do
        os.vexecv(shdc_path, {"-i", shdc_shader_path, "-l", language, "-f", "bare", "--defines=EDGE_AA", "-o", "._"})
    end
    os.cd("-")
end
