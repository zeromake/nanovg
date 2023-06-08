import("net.http")

local languages = {
    "glsl330",
    "glsl100",
    "glsl300es",
    "hlsl5",
    "metal_macos",
}

local shdc = "sokol-shdc"

if os.host() == "windows" then
    shdc = shdc .. ".exe"
end

local downloads = {
    windows="https://raw.githubusercontent.com/floooh/sokol-tools-bin/master/bin/win32/sokol-shdc.exe",
    macosx="https://raw.githubusercontent.com/floooh/sokol-tools-bin/master/bin/osx/sokol-shdc",
    linux="https://raw.githubusercontent.com/floooh/sokol-tools-bin/master/bin/linux/sokol-shdc",
}

function generate_glsl_headers(dir, language, suffix, edgeAA) {
    local include_path = path.absolute(path.join(os.scriptdir(), "../build/include"))
    os.mkdir(include_path)
    local file = path.join(dir, "._nanovg_"..language.."_"..suffix..".glsl")
    local inputFile = io.open(file, "rb")
    local outFile = io.open(path.join(include_path, language.."_"..suffix..(edgeAA and "_aa" or "")..".h"))
    for line in inputFile:lines() do
    end
    outFile:close()
    inputFile:close()
}

function main()
    local shdc_path = path.absolute(path.join(os.scriptdir(), "../build/bin", shdc))
    local shdc_shader_path = path.absolute(path.join(os.scriptdir(), "../src/shd.glsl"))
    if not os.exists(shdc_path) then
        local url = downloads[os.host()]
        print("download: "..url.." -> "..shdc_path)
        http.download(url, shdc_path)
        if os.host() ~= "windows" then
            os.vexecv("chmod", {"+x", shdc_path})
        end
    end
    os.mkdir(path.join(os.scriptdir(), "../build/shader"))
    os.mkdir(path.join(os.scriptdir(), "../build/shader_aa"))
    os.cd(path.join(os.scriptdir(), "../build/shader"))
    for _, language in ipairs(languages) do
        os.vexecv(shdc_path, {"-i", shdc_shader_path, "-l", language, "-f", "bare", "-o", "."})
        if language:startswith("glsl") then

        end
    end
    os.cd("-")
    os.cd(path.join(os.scriptdir(), "../build/shader_aa"))
    for _, language in ipairs(languages) do
        os.vexecv(shdc_path, {"-i", shdc_shader_path, "-l", language, "-f", "bare", "--defines=EDGE_AA", "-o", "."})
    end
    os.cd("-")
end
