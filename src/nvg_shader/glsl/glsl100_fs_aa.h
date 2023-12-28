const char __shader_fs_aa[] = 
"#extension GL_OES_standard_derivatives : require\n"
"precision mediump float;\n"
"precision highp int;\n"
"\n"
"uniform highp vec4 frag[11];\n"
"uniform highp sampler2D tex_smp;\n"
"\n"
"varying highp vec2 ftcoord;\n"
"varying highp vec2 fpos;\n"
"\n"
"highp float strokeMask()\n"
"{\n"
"    return min(1.0, (1.0 - abs(ftcoord.x * 2.0 + (-1.0))) * frag[10].x) * min(1.0, ftcoord.y);\n"
"}\n"
"\n"
"highp float scissorMask(highp vec2 p)\n"
"{\n"
"    highp vec2 _109 = (-(abs((mat3(vec3(frag[0].xyz), vec3(frag[1].xyz), vec3(frag[2].xyz)) * vec3(p, 1.0)).xy) - frag[8].xy)) * frag[8].zw + vec2(0.5);\n"
"    return clamp(_109.x, 0.0, 1.0) * clamp(_109.y, 0.0, 1.0);\n"
"}\n"
"\n"
"highp float sdroundrect(highp vec2 pt, highp vec2 ext, highp float rad)\n"
"{\n"
"    highp vec2 _33 = abs(pt) - (ext - vec2(rad, rad));\n"
"    return (min(max(_33.x, _33.y), 0.0) + length(max(_33, vec2(0.0)))) - rad;\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    highp float _142 = strokeMask();\n"
"    if (_142 < frag[10].y)\n"
"    {\n"
"        discard;\n"
"    }\n"
"    highp vec2 param = fpos;\n"
"    highp float _155 = scissorMask(param);\n"
"    if (_155 == 0.0)\n"
"    {\n"
"        return;\n"
"    }\n"
"    int _164 = int(frag[10].w);\n"
"    highp vec4 result;\n"
"    if (_164 == 0)\n"
"    {\n"
"        highp vec2 param_1 = (mat3(vec3(frag[3].xyz), vec3(frag[4].xyz), vec3(frag[5].xyz)) * vec3(fpos, 1.0)).xy;\n"
"        highp vec2 param_2 = frag[9].xy;\n"
"        highp float param_3 = frag[9].z;\n"
"        result = mix(frag[6], frag[7], vec4(clamp((frag[9].w * 0.5 + sdroundrect(param_1, param_2, param_3)) / frag[9].w, 0.0, 1.0))) * (_142 * _155);\n"
"    }\n"
"    else\n"
"    {\n"
"        if (_164 == 1)\n"
"        {\n"
"            highp vec4 color = texture2D(tex_smp, (mat3(vec3(frag[3].xyz), vec3(frag[4].xyz), vec3(frag[5].xyz)) * vec3(fpos, 1.0)).xy / frag[9].xy);\n"
"            int _294 = int(frag[10].z);\n"
"            if (_294 == 1)\n"
"            {\n"
"                color = vec4(color.xyz * color.w, color.w);\n"
"            }\n"
"            if (_294 == 2)\n"
"            {\n"
"                color = vec4(color.x);\n"
"            }\n"
"            bool _321 = _294 == 3;\n"
"            bool _327;\n"
"            if (_321)\n"
"            {\n"
"                _327 = color.w == 0.0;\n"
"            }\n"
"            else\n"
"            {\n"
"                _327 = _321;\n"
"            }\n"
"            if (_327)\n"
"            {\n"
"                discard;\n"
"            }\n"
"            highp vec4 _333 = color;\n"
"            highp vec4 _339 = (_333 * frag[6]) * (_142 * _155);\n"
"            color = _339;\n"
"            result = _339;\n"
"        }\n"
"        else\n"
"        {\n"
"            if (_164 == 2)\n"
"            {\n"
"                result = vec4(1.0);\n"
"            }\n"
"            else\n"
"            {\n"
"                if (_164 == 3)\n"
"                {\n"
"                    highp vec4 color_1 = texture2D(tex_smp, ftcoord);\n"
"                    int _364 = int(frag[10].z);\n"
"                    if (_364 == 1)\n"
"                    {\n"
"                        color_1 = vec4(color_1.xyz * color_1.w, color_1.w);\n"
"                    }\n"
"                    if (_364 == 2)\n"
"                    {\n"
"                        color_1 = vec4(color_1.x);\n"
"                    }\n"
"                    result = (color_1 * _155) * frag[6];\n"
"                }\n"
"                else\n"
"                {\n"
"                    if (_164 == 4)\n"
"                    {\n"
"                        highp vec4 color_2 = texture2D(tex_smp, ftcoord);\n"
"                        highp float _410 = dFdx(ftcoord.x);\n"
"                        if (_410 < 0.0)\n"
"                        {\n"
"                            highp vec4 _415 = color_2;\n"
"                            highp vec4 _489 = _415;\n"
"                            _489.x = _415.z;\n"
"                            _489.z = _415.x;\n"
"                            color_2 = _489;\n"
"                        }\n"
"                        else\n"
"                        {\n"
"                            if (_410 == 0.0)\n"
"                            {\n"
"                                color_2 = vec4(((color_2.x + color_2.y) + color_2.z) * 0.3333333432674407958984375);\n"
"                            }\n"
"                        }\n"
"                        if (((color_2.w * _155) * frag[6].w) < 0.00390625)\n"
"                        {\n"
"                            discard;\n"
"                        }\n"
"                        result = vec4(color_2.xyz * frag[6].xyz + ((vec3(frag[6].w) - color_2.xyz) * frag[7].xyz), frag[6].w) * _155;\n"
"                    }\n"
"                }\n"
"            }\n"
"        }\n"
"    }\n"
"    gl_FragData[0] = result;\n"
"}\n"
"\n";