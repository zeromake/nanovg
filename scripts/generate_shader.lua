import("net.http")
import("core.base.option")

local languages = {
    "glsl330",
    "glsl100",
    "glsl300es",
    "hlsl5",
    "metal_macos",
    "metal_ios",
    "metal_sim"
}

local suffixs = {
    "glsl",
    "glsl",
    "glsl",
    "hlsl",
    "metal",
    "metal",
    "metal"
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
    local fragGsub = nil
    for line in inputFile:lines() do
        local skip = false
        if not line:startswith("#version") then
            if line:startswith("struct frag") then
                line = "uniform frag";
            elseif line:startswith("uniform frag ") then
                fragGsub = string.sub(line, 14, #line - 1).."%."
                skip = true
            end
            if not skip then
                if fragGsub ~= nil then
                    line = line:gsub(fragGsub, '')
                end
                outFile:write("\n\"")
                outFile:write(line)
                outFile:write("\\n\"")
            end
        end
    end
    outFile:write(";")
    outFile:close()
    inputFile:close()
    -- if fragGsub ~= nil then
    --     local text = io.readfile(out);
    --     text = text:gsub(fragGsub, '')
    --     io.writefile(out, text);
    -- end
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
    for i, language in ipairs(languages) do
        os.vexecv(shdc_path, {"-i", shdc_shader_path, "-l", language, "-f", "bare", "--defines=EDGE_AA", "-o", "."})
        if language:startswith("glsl") then
            generate_glsl_headers(language, "fs", true)
        end
        os.rm("._nanovg_"..language.."_vs."..suffixs[i])
    end
    os.cd("-")
    if os.host() == "macosx" then
        local ps = {
            {
                "iphoneos",
                "ios",
                "ios",
                "-mios-version-min=9.0",
            },
            {
                "iphonesimulator",
                "simulator",
                "sim",
            },
            {
                "macosx",
                "macos",
                "macos",
                "-mmacosx-version-min=10.0",
            },
            {
                "appletvos",
                "tvos",
                "macos",
            }
        }
        for _, f in ipairs(ps) do
            for _, n in ipairs({"fs", "fs_aa", "vs"}) do
                local file = "build/shader/._nanovg_metal_"..f[3].."_"..n..".metal"
                if n == "fs_aa" then
                    file = "build/shader_aa/._nanovg_metal_"..f[3].."_fs.metal"
                end
                if n == "vs" then
                    local text = io.readfile(file);
                    text = text:gsub('%[%[buffer%(0%)%]%]%)', '[[buffer(1)]])')
                    io.writefile(file, text);
                end
                local argv = {"-sdk", f[1], "metal", "-c", file}
                if #f > 3 then
                    table.insert(argv, f[4])
                end
                local VARIABLE_NAME = path.join("src/metal/mnvg_bitcode", f[2].."_"..n)
                local AIR_FILE_NAME = VARIABLE_NAME..".air"
                local HEADER_NAME = VARIABLE_NAME..".h"
                table.insert(argv, "-o")
                table.insert(argv, AIR_FILE_NAME)
                os.vexecv("xcrun", argv)
                os.vexecv("xcrun", {"-sdk", f[1], "metallib", AIR_FILE_NAME, "-o", VARIABLE_NAME})
                local outdata, errdata = os.iorunv("xxd", {"-i", "-n", "mnvg_bitcode_"..f[2].."_"..n, VARIABLE_NAME})
                io.writefile(HEADER_NAME, outdata)
                os.rm(AIR_FILE_NAME)
                os.rm(VARIABLE_NAME)
            end
        end
    end
end
