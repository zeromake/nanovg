import("net.http")
import("core.base.option")

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

function generate_glsl_headers(language, suffix, edgeAA)
    local include_path = path.join("../../src/nvg_shader")
    os.mkdir(include_path)
    local file = "._nanovg_"..language.."_"..suffix..".glsl"
    local inputFile = io.open(file, "r")
    local out = path.join(include_path, language.."_"..suffix..(edgeAA and "_aa" or "")..".h")
    local outFile = io.open(out, "w")
    outFile:writef("const char %s[] = ", "__shader_"..suffix..(edgeAA and "_aa" or ""));
    for line in inputFile:lines() do
        if not line:startswith("#version") then
            local index = 1
            outFile:write("\n\"")
            outFile:write(line)
            outFile:write("\\n\"")
            -- while(index <= #line-1) do
            --     local char = string.byte(line, index)
            --     outFile:writef("%#x,", char)
            --     index = index+1
            -- end
        end
    end
    outFile:write(";")
    outFile:close()
    inputFile:close()
    print("generate %s", out)
end

function main()
    option.save("main")
    option.set("verbose", true)
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
    local curr = path.join(os.scriptdir(), "../build/shader")
    os.cd(curr)
    for _, language in ipairs(languages) do
        os.vexecv(shdc_path, {"-i", shdc_shader_path, "-l", language, "-f", "bare", "-o", "."})
        if language:startswith("glsl") then
            generate_glsl_headers(language, "fs", false)
            generate_glsl_headers(language, "vs", false)
        end
    end
    os.cd("-")
    curr = path.join(os.scriptdir(), "../build/shader_aa")
    os.cd(curr)
    for _, language in ipairs(languages) do
        os.vexecv(shdc_path, {"-i", shdc_shader_path, "-l", language, "-f", "bare", "--defines=EDGE_AA", "-o", "."})
        if language:startswith("glsl") then
            generate_glsl_headers(language, "fs", true)
        end
    end
    os.cd("-")
end
