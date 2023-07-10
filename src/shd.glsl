// use https://github.com/floooh/sokol-tools demo

@vs vs
layout (binding = 0) uniform viewSize {
#ifdef _HLSL5_
    mat4 dummy;
#endif
    vec4 _viewSize;
};
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 tcoord;
layout (location = 0) out vec2 ftcoord;
layout (location = 1) out vec2 fpos;

void main(void) {
	ftcoord = tcoord;
	fpos = vertex;
    float x = 2.0 * vertex.x / _viewSize.x - 1.0;
    float y = 1.0 - 2.0 * vertex.y / _viewSize.y;
	gl_Position = vec4(
        x,
        y,
        0,
        1
    );
}
@end

@fs fs
precision highp float;
#ifdef _HLSL5_
    uniform frag {
        mat4 _scissorMat;
        vec4 _scissorExt;
        vec4 _scissorScale;
        mat4 _paintMat;
        vec4 _extent;
        vec4 _radius;
        vec4 _feather;
        vec4 innerCol;
        vec4 outerCol;
        vec4 _strokeMult;
        int texType;
        int type;
    };
    #define scissorMat mat3(_scissorMat)
    #define scissorExt _scissorExt.xy
    #define scissorScale _scissorScale.xy
    #define paintMat mat3(_paintMat)
    #define extent _extent.xy
    #define radius _radius.x
    #define feather _feather.x
    #define strokeMult _strokeMult.x
    #define strokeThr _strokeMult.y
#else
    #ifdef USE_UNIFORMBUFFER
        layout(std140,binding=1) uniform frag {
            mat3 scissorMat;
            mat3 paintMat;
            vec4 innerCol;
            vec4 outerCol;
            vec2 scissorExt;
            vec2 scissorScale;
            vec2 extent;
            float radius;
            float feather;
            float strokeMult;
            float strokeThr;
            int texType;
            int type;
        };
    #else
        layout(std140,binding=1) uniform frag {
            vec4 dummy[11];
        };
        #define scissorMat mat3(dummy[0].xyz, dummy[1].xyz, dummy[2].xyz)
        #define paintMat mat3(dummy[3].xyz, dummy[4].xyz, dummy[5].xyz)
        #define innerCol dummy[6]
        #define outerCol dummy[7]
        #define scissorExt dummy[8].xy
        #define scissorScale dummy[8].zw
        #define extent dummy[9].xy
        #define radius dummy[9].z
        #define feather dummy[9].w
        #define strokeMult dummy[10].x
        #define strokeThr dummy[10].y
        #define texType int(dummy[10].z)
        #define type int(dummy[10].w)
    #endif
#endif

layout(binding=2) uniform sampler2D tex;
layout(location = 0) in vec2 ftcoord;
layout(location = 1) in vec2 fpos;
layout(location = 0) out vec4 outColor;

float sdroundrect(vec2 pt, vec2 ext, float rad) {
    vec2 ext2 = ext - vec2(rad,rad);
    vec2 d = abs(pt) - ext2;
    return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rad;
}

// Scissoring
float scissorMask(vec2 p) {
    vec2 sc = (abs((scissorMat * vec3(p,1.0)).xy) - scissorExt);
    sc = vec2(0.5,0.5) - sc * scissorScale;
    return clamp(sc.x,0.0,1.0) * clamp(sc.y,0.0,1.0);
}

#ifdef EDGE_AA
// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.
float strokeMask() {
    return min(1.0, (1.0-abs(ftcoord.x*2.0-1.0))*strokeMult) * min(1.0, ftcoord.y);
}
#endif

void main(void) {
    vec4 result;

#ifdef EDGE_AA
    float strokeAlpha = strokeMask();
#ifdef _HLSL5_
#endif
    if (strokeAlpha < strokeThr) discard;
#else
    float strokeAlpha = 1.0;
#endif
    float scissor = scissorMask(fpos);

    if (scissor == 0) {
        return;
    }

    if (type == 0) {    // Gradient
        // Calculate gradient color using box gradient
        vec2 pt = (paintMat * vec3(fpos,1.0)).xy;
        float d = clamp((sdroundrect(pt, extent, radius) + feather*0.5) / feather, 0.0, 1.0);
        vec4 color = mix(innerCol,outerCol,d);
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    } else if (type == 1) {// Image
        // Calculate color fron texture
        vec2 pt = (paintMat * vec3(fpos,1.0)).xy / extent;
        vec4 color = texture(tex, pt);
        if (texType == 1) color = vec4(color.xyz*color.w,color.w);
        if (texType == 2) color = vec4(color.x);
        // Apply color tint and alpha.
        color *= innerCol;
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    } else if (type == 2) {// Stencil fill
        result = vec4(1,1,1,1);
    } else if (type == 3) {// Textured tris
        vec4 color = texture(tex, ftcoord);
        if (texType == 1) color = vec4(color.xyz*color.w,color.w);
        if (texType == 2) color = vec4(color.x);
        result = color * scissor * innerCol;
    } else if (type == 4) {// GLNVGshaderType::NSVG_SHADER_CLEARTYPE
        // https://github.com/Const-me/nanovg
        vec4 color = texture(tex, ftcoord);
        float deriv = dFdx(ftcoord.x);
        if (deriv < 0.0) {
            // The text is horizontally mirrored, or rotated 180 degrees. Flip red and blue subpixels of the texture.
            color.xz = color.zx;
        } else if (deriv == 0.0) {
            // The text is rotated 90 degrees. Average all 3 subpixels, disabling ClearType
            color = vec4((color.x + color.y + color.z) * (1.0 / 3.0));
        }
        if (color.w * scissor * innerCol.w < (1.0 / 256.0)) {
            discard;
        }
        // Do the clear type thing
		result.xyz = color.xyz * innerCol.xyz + ( vec3( innerCol.w ) - color.xyz ) * outerCol.xyz;
		result.w = innerCol.w;
		result *= scissor;
    }
    outColor = result;
}
@end

@program nanovg vs fs
