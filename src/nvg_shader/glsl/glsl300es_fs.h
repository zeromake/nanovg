const char __shader_fs[] = 
"precision mediump float;\n"
"precision highp int;\n"
"\n"
"uniform highp vec4 frag[11];\n"
"uniform highp sampler2D tex;\n"
"\n"
"in highp vec2 fpos;\n"
"in highp vec2 ftcoord;\n"
"layout(location = 0) out highp vec4 outColor;\n"
"\n"
"highp float scissorMask(highp vec2 p)\n"
"{\n"
"    highp vec2 _108 = (-(abs((mat3(vec3(frag[0].xyz), vec3(frag[1].xyz), vec3(frag[2].xyz)) * vec3(p, 1.0)).xy) - frag[8].xy)) * frag[8].zw + vec2(0.5);\n"
"    return clamp(_108.x, 0.0, 1.0) * clamp(_108.y, 0.0, 1.0);\n"
"}\n"
"\n"
"highp float sdroundrect(highp vec2 pt, highp vec2 ext, highp float rad)\n"
"{\n"
"    highp vec2 _32 = abs(pt) - (ext - vec2(rad, rad));\n"
"    return (min(max(_32.x, _32.y), 0.0) + length(max(_32, vec2(0.0)))) - rad;\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    highp vec2 param = fpos;\n"
"    highp float _124 = scissorMask(param);\n"
"    if (_124 == 0.0)\n"
"    {\n"
"        return;\n"
"    }\n"
"    int _136 = int(frag[10].w);\n"
"    highp vec4 result;\n"
"    if (_136 == 0)\n"
"    {\n"
"        highp vec2 param_1 = (mat3(vec3(frag[3].xyz), vec3(frag[4].xyz), vec3(frag[5].xyz)) * vec3(fpos, 1.0)).xy;\n"
"        highp vec2 param_2 = frag[9].xy;\n"
"        highp float param_3 = frag[9].z;\n"
"        result = mix(frag[6], frag[7], vec4(clamp((frag[9].w * 0.5 + sdroundrect(param_1, param_2, param_3)) / frag[9].w, 0.0, 1.0))) * _124;\n"
"    }\n"
"    else\n"
"    {\n"
"        if (_136 == 1)\n"
"        {\n"
"            highp vec4 color = texture(tex, (mat3(vec3(frag[3].xyz), vec3(frag[4].xyz), vec3(frag[5].xyz)) * vec3(fpos, 1.0)).xy / frag[9].xy);\n"
"            int _261 = int(frag[10].z);\n"
"            if (_261 == 1)\n"
"            {\n"
"                color = vec4(color.xyz * color.w, color.w);\n"
"            }\n"
"            if (_261 == 2)\n"
"            {\n"
"                color = vec4(color.x);\n"
"            }\n"
"            highp vec4 _287 = color;\n"
"            highp vec4 _293 = (_287 * frag[6]) * _124;\n"
"            color = _293;\n"
"            result = _293;\n"
"        }\n"
"        else\n"
"        {\n"
"            if (_136 == 2)\n"
"            {\n"
"                result = vec4(1.0);\n"
"            }\n"
"            else\n"
"            {\n"
"                if (_136 == 3)\n"
"                {\n"
"                    highp vec4 color_1 = texture(tex, ftcoord);\n"
"                    int _317 = int(frag[10].z);\n"
"                    if (_317 == 1)\n"
"                    {\n"
"                        color_1 = vec4(color_1.xyz * color_1.w, color_1.w);\n"
"                    }\n"
"                    if (_317 == 2)\n"
"                    {\n"
"                        color_1 = vec4(color_1.x);\n"
"                    }\n"
"                    result = (color_1 * _124) * frag[6];\n"
"                }\n"
"            }\n"
"        }\n"
"    }\n"
"    outColor = result;\n"
"}\n"
"\n";