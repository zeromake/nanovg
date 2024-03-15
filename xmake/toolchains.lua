toolchain("devkit")
    set_description("devkit is Nintendo Switch build sdk")
    set_homepage("https://devkitpro.org")
    set_kind("cross")

    on_check(function (toolchain)
        local sdkdir = toolchain:sdkdir()
        local bindir = toolchain:bindir()
        if not sdkdir or not bindir then
            if not sdkdir then
                sdkdir = is_host("windows") and "C:\\devkitpro" or "/opt/devkitpro"
            end
            bindir = path.join(sdkdir, 'devkitA64/bin')
        end
        if not os.exists(path.join(bindir, 'aarch64-none-elf-gcc')) then
            raise("devkit toolchain not found!")
        end
        toolchain:config_set("cross", 'aarch64-none-elf-')
        toolchain:config_set("bindir", bindir)
        toolchain:config_set("sdkdir", sdkdir)
        toolchain:configs_save()
    end)
    on_load(function (toolchain)
        import("core.project.config")
        -- get cross
        local cross = toolchain:cross() or "aarch64-none-elf-"
        local bindir = toolchain:bindir()
        local sdkdir = toolchain:sdkdir()
        if bindir and is_host("windows") then
            toolchain:add("runenvs", "PATH", bindir)
        end
        toolchain:add("toolset", "cc", cross .. "gcc")
        toolchain:add("toolset", "cxx", cross .. "g++", cross .. "gcc")
        toolchain:add("toolset", "cpp", cross .. "gcc -E")
        toolchain:add("toolset", "as", cross .. "gcc")
        toolchain:add("toolset", "ld", cross .. "g++", cross .. "gcc")
        toolchain:add("toolset", "sh", cross .. "g++", cross .. "gcc")
        toolchain:add("toolset", "ar", cross .. "ar")
        toolchain:add("toolset", "strip", cross .. "strip")
        toolchain:add("toolset", "ranlib", cross .. "ranlib")
        local cross_prefix = cross:sub(1, -2)
        -- toolchain:add("sysincludedirs", path.join(sdkdir, cross_prefix, "include"))
        -- toolchain:add("sysincludedirs", path.join(sdkdir, "libnx/include"))
        toolchain:add("includedirs", path.join(sdkdir, "portlibs/switch/include"))
        toolchain:add("linkdirs", path.join(sdkdir, cross_prefix, "lib"))
        toolchain:add("linkdirs", path.join(sdkdir, "libnx/lib"))
        toolchain:add("linkdirs", path.join(sdkdir, "portlibs/switch/lib"))
        toolchain:add("cxflags", '-D__SWITCH__=1 -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE')
        toolchain:add("ldflags", '-specs='..sdkdir..'/libnx/switch.specs -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE')
        toolchain:add("syslinks", 'nx')
    end)
