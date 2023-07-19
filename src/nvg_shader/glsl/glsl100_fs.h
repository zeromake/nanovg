const char __shader_fs[] = 
"#extension GL_OES_standard_derivatives : require\n"
"precision mediump float;\n"
"precision highp int;\n"
"\n"
"uniform highp vec4 frag[11];\n"
"uniform highp sampler2D tex_smp;\n"
"\n"
"varying highp vec2 fpos;\n"
"varying highp vec2 ftcoord;\n"
"\n"
"highp float scissorMask(highp vec2 p)\n"
"{\n"
"    highp vec2 _106 = (-(abs((mat3(vec3(frag[0].xyz), vec3(frag[1].xyz), vec3(frag[2].xyz)) * vec3(p, 1.0)).xy) - frag[8].xy)) * frag[8].zw + vec2(0.5);\n"
"    return clamp(_106.x, 0.0, 1.0) * clamp(_106.y, 0.0, 1.0);\n"
"}\n"
"\n"
"highp float sdroundrect(highp vec2 pt, highp vec2 ext, highp float rad)\n"
"{\n"
"    highp vec2 _30 = abs(pt) - (ext - vec2(rad, rad));\n"
"    return (min(max(_30.x, _30.y), 0.0) + length(max(_30, vec2(0.0)))) - rad;\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    highp vec2 param = fpos;\n"
"    highp float _122 = scissorMask(param);\n"
"    if (_122 == 0.0)\n"
"    {\n"
"        return;\n"
"    }\n"
"    int _134 = int(frag[10].w);\n"
"    highp vec4 result;\n"
"    if (_134 == 0)\n"
"    {\n"
"        highp vec2 param_1 = (mat3(vec3(frag[3].xyz), vec3(frag[4].xyz), vec3(frag[5].xyz)) * vec3(fpos, 1.0)).xy;\n"
"        highp vec2 param_2 = frag[9].xy;\n"
"        highp float param_3 = frag[9].z;\n"
"        result = mix(frag[6], frag[7], vec4(clamp((frag[9].w * 0.5 + sdroundrect(param_1, param_2, param_3)) / frag[9].w, 0.0, 1.0))) * _122;\n"
"    }\n"
"    else\n"
"    {\n"
"        if (_134 == 1)\n"
"        {\n"
"            highp vec4 color = texture2D(tex_smp, (mat3(vec3(frag[3].xyz), vec3(frag[4].xyz), vec3(frag[5].xyz)) * vec3(fpos, 1.0)).xy / frag[9].xy);\n"
"            int _264 = int(frag[10].z);\n"
"            if (_264 == 1)\n"
"            {\n"
"                color = vec4(color.xyz * color.w, color.w);\n"
"            }\n"
"            if (_264 == 2)\n"
"            {\n"
"                color = vec4(color.x);\n"
"            }\n"
"            highp vec4 _290 = color;\n"
"            highp vec4 _296 = (_290 * frag[6]) * _122;\n"
"            color = _296;\n"
"            result = _296;\n"
"        }\n"
"        else\n"
"        {\n"
"            if (_134 == 2)\n"
"            {\n"
"                result = vec4(1.0);\n"
"            }\n"
"            else\n"
"            {\n"
"                if (_134 == 3)\n"
"                {\n"
"                    highp vec4 color_1 = texture2D(tex_smp, ftcoord);\n"
"                    int _322 = int(frag[10].z);\n"
"                    if (_322 == 1)\n"
"                    {\n"
"                        color_1 = vec4(color_1.xyz * color_1.w, color_1.w);\n"
"                    }\n"
"                    if (_322 == 2)\n"
"                    {\n"
"                        color_1 = vec4(color_1.x);\n"
"                    }\n"
"                    result = (color_1 * _122) * frag[6];\n"
"                }\n"
"                else\n"
"                {\n"
"                    if (_134 == 4)\n"
"                    {\n"
"                        highp vec4 color_2 = texture2D(tex_smp, ftcoord);\n"
"                        highp float _369 = dFdx(ftcoord.x);\n"
"                        if (_369 < 0.0)\n"
"                        {\n"
"                            highp vec4 _374 = color_2;\n"
"                            highp vec4 _447 = _374;\n"
"                            _447.x = _374.z;\n"
"                            _447.z = _374.x;\n"
"                            color_2 = _447;\n"
"                        }\n"
"                        else\n"
"                        {\n"
"                            if (_369 == 0.0)\n"
"                            {\n"
"                                color_2 = vec4(((color_2.x + color_2.y) + color_2.z) * 0.3333333432674407958984375);\n"
"                            }\n"
"                        }\n"
"                        if (((color_2.w * _122) * frag[6].w) < 0.00390625)\n"
"                        {\n"
"                            discard;\n"
"                        }\n"
"                        result = vec4(color_2.xyz * frag[6].xyz + ((vec3(frag[6].w) - color_2.xyz) * frag[7].xyz), frag[6].w) * _122;\n"
"                    }\n"
"                }\n"
"            }\n"
"        }\n"
"    }\n"
"    gl_FragData[0] = result;\n"
"}\n"
"\n";