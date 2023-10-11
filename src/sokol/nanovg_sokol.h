#ifndef NANOVG_SOKOL_H
#define NANOVG_SOKOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <nanovg.h>
#include <sokol_gfx.h>

// struct sg_image;

// Create flags

enum NVGcreateFlags {
    // Flag indicating if geometry based anti-aliasing is used (may not be needed when using MSAA).
    NVG_ANTIALIAS 		= 1<<0,
    // Flag indicating if strokes should be drawn using stencil buffer. The rendering will be a little
    // slower, but path overlaps (i.e. self-intersecting or sharp turns) will be drawn just once.
    NVG_STENCIL_STROKES	= 1<<1,
    // Flag indicating that additional debug checks are done.
    NVG_DEBUG 			= 1<<2,
};

NVGcontext* nvgCreateSokol(int flags);
void nvgDeleteSokol(NVGcontext* ctx);

int nvsgCreateImageFromHandleSokol(NVGcontext* ctx, sg_image imageSokol, sg_sampler samplerSokol, int type, int w, int h, int flags);
struct sg_image nvsgImageHandleSokol(NVGcontext* ctx, int image);

// These are additional flags on top of NVGimageFlags.
enum NVGimageFlagsGL {
    NVG_IMAGE_NODELETE			= 1<<16,	// Do not delete Sokol image.
};

#ifdef __cplusplus
}
#endif

#endif /* NANOVG_SOKOL_H */

#ifdef NANOVG_SOKOL_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "shd.glsl.h"
#include "shd.aa.glsl.h"


enum SGNVGshaderType {
    NSVG_SHADER_FILLGRAD,
    NSVG_SHADER_FILLIMG,
    NSVG_SHADER_SIMPLE,
    NSVG_SHADER_IMG
};

struct SGNVGtexture {
    int id;
    sg_image img;
    sg_sampler smp;
    int width, height;
    int type;
    int flags;
};
typedef struct SGNVGtexture SGNVGtexture;

struct SGNVGblend
{
    sg_blend_factor srcRGB;
    sg_blend_factor dstRGB;
    sg_blend_factor srcAlpha;
    sg_blend_factor dstAlpha;
};
typedef struct SGNVGblend SGNVGblend;

enum SGNVGcallType {
    SGNVG_NONE = 0,
    SGNVG_FILL,
    SGNVG_CONVEXFILL,
    SGNVG_STROKE,
    SGNVG_TRIANGLES,
};

struct SGNVGcall {
    int type;
    int image;
    int pathOffset;
    int pathCount;
    int triangleOffset;
    int triangleCount;
    int uniformOffset;
    SGNVGblend blendFunc;
};
typedef struct SGNVGcall SGNVGcall;

struct SGNVGpath {
    int fillOffset;
    int fillCount;
    int strokeOffset;
    int strokeCount;
};
typedef struct SGNVGpath SGNVGpath;

struct SGNVGattribute {
    float vertex[2];
    float tcoord[2];
};
typedef struct SGNVGattribute SGNVGattribute;

struct SGNVGvertUniforms {
    float viewSize[4];
};
typedef struct SGNVGvertUniforms SGNVGvertUniforms;

struct SGNVGfragUniforms {
    #define NANOVG_SG_UNIFORMARRAY_SIZE 11
    union {
        struct {
            float scissorMat[12]; // matrices are actually 3 vec4s
            float paintMat[12];
            struct NVGcolor innerCol;
            struct NVGcolor outerCol;
            float scissorExt[2];
            float scissorScale[2];
            float extent[2];
            float radius;
            float feather;
            float strokeMult;
            float strokeThr;
            float texType;
            float type;
        };
        float uniformArray[NANOVG_SG_UNIFORMARRAY_SIZE][4];
    };
};
typedef struct SGNVGfragUniforms SGNVGfragUniforms;

// LRU cache; keep its size relatively small, as items are accessed via a linear search
#define NANOVG_SG_PIPELINE_CACHE_SIZE 32

struct SGNVGpipelineCacheKey {
    uint16_t blend;         // cached as `src_factor_rgb | (dst_factor_rgb << 4) | (src_factor_alpha << 8) | (dst_factor_alpha << 12)`
    uint16_t lastUse;      // updated on each read
};
typedef struct SGNVGpipelineCacheKey SGNVGpipelineCacheKey;

enum SGNVGpipelineType
{
    // used by sgnvg__convexFill, sgnvg__stroke, sgnvg__triangles
    SGNVG_PIP_BASE = 0,

    // used by sgnvg__fill
    SGNVG_PIP_FILL_STENCIL,
    SGNVG_PIP_FILL_ANTIALIAS,   // only used if sg->flags & NVG_ANTIALIAS
    SGNVG_PIP_FILL_DRAW,

    // used by sgnvg__stroke
    SGNVG_PIP_STROKE_STENCIL_DRAW,      // only used if sg->flags & NVG_STENCIL_STROKES
    SGNVG_PIP_STROKE_STENCIL_ANTIALIAS, // only used if sg->flags & NVG_STENCIL_STROKES
    SGNVG_PIP_STROKE_STENCIL_CLEAR,     // only used if sg->flags & NVG_STENCIL_STROKES

    SGNVG_PIP_NUM_
};
typedef enum SGNVGpipelineType SGNVGpipelineType;

struct SGNVGpipelineCache {
    // keys are stored as a separate array for search performance
    SGNVGpipelineCacheKey keys[NANOVG_SG_PIPELINE_CACHE_SIZE];
    sg_pipeline pipelines[NANOVG_SG_PIPELINE_CACHE_SIZE][SGNVG_PIP_NUM_];
    uint8_t pipelinesActive[NANOVG_SG_PIPELINE_CACHE_SIZE];
    uint16_t currentUse;   // incremented on each overwrite
};
typedef struct SGNVGpipelineCache SGNVGpipelineCache;

struct SGNVGcontext {
    sg_shader shader;
    SGNVGtexture* textures;
    SGNVGvertUniforms view;
    int ntextures;
    int ctextures;
    int textureId;
    sg_buffer vertBuf;
    sg_buffer indexBuf;
    SGNVGpipelineCache pipelineCache;
    int fragSize;
    int flags;

    // Per frame buffers
    SGNVGcall* calls;
    int ccalls;
    int ncalls;
    SGNVGpath* paths;
    int cpaths;
    int npaths;
    SGNVGattribute* verts;
    int cverts;
    int nverts;
    int cverts_gpu;
    uint32_t* indexes;
    int cindexes;
    int nindexes;
    int cindexes_gpu;
    unsigned char* uniforms;
    int cuniforms;
    int nuniforms;

    // state
    int pipelineCacheIndex;
    sg_blend_state blend;

    int dummyTex;
};
typedef struct SGNVGcontext SGNVGcontext;

static int sgnvg__maxi(int a, int b) { return a > b ? a : b; }

#ifdef SOKOL_GLES2
static unsigned int sgnvg__nearestPow2(unsigned int num)
{
    unsigned n = num > 0 ? num - 1 : 0;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}
#endif

static SGNVGtexture* sgnvg__allocTexture(SGNVGcontext* sg)
{
    SGNVGtexture* tex = NULL;
    int i;

    for (i = 0; i < sg->ntextures; i++) {
        if (sg->textures[i].id == 0) {
            tex = &sg->textures[i];
            break;
        }
    }
    if (tex == NULL) {
        if (sg->ntextures+1 > sg->ctextures) {
            SGNVGtexture* textures;
            int ctextures = sgnvg__maxi(sg->ntextures+1, 4) +  sg->ctextures/2; // 1.5x Overallocate
            textures = (SGNVGtexture*)realloc(sg->textures, sizeof(SGNVGtexture)*ctextures);
            if (textures == NULL) return NULL;
            sg->textures = textures;
            sg->ctextures = ctextures;
        }
        tex = &sg->textures[sg->ntextures++];
    }

    memset(tex, 0, sizeof(*tex));
    tex->id = ++sg->textureId;

    return tex;
}

static SGNVGtexture* sgnvg__findTexture(SGNVGcontext* sg, int id)
{
    int i;
    for (i = 0; i < sg->ntextures; i++)
        if (sg->textures[i].id == id)
            return &sg->textures[i];
    return NULL;
}

static int sgnvg__deleteTexture(SGNVGcontext* sg, int id)
{
    int i;
    for (i = 0; i < sg->ntextures; i++) {
        if (sg->textures[i].id == id) {
            if (sg->textures[i].img.id != 0 && (sg->textures[i].flags & NVG_IMAGE_NODELETE) == 0)
                sg_destroy_image(sg->textures[i].img);
            memset(&sg->textures[i], 0, sizeof(sg->textures[i]));
            return 1;
        }
    }
    return 0;
}

static uint16_t sgnvg__getCombinedBlendNumber(sg_blend_state blend)
{
#if __STDC_VERSION__ >= 201112L
    _Static_assert(_SG_BLENDFACTOR_NUM <= 17, "too many blend factors for 16-bit blend number");
#else
    assert(_SG_BLENDFACTOR_NUM <= 17);  // can be a _Static_assert
#endif
    return blend.src_factor_rgb | (blend.dst_factor_rgb << 4) | (blend.src_factor_alpha << 8) | (blend.dst_factor_alpha << 12);
}

static void sgnvg__initPipeline(SGNVGcontext* sg, sg_pipeline pip, const sg_stencil_state* stencil, sg_color_mask write_mask, sg_cull_mode cull_mode)
{
    sg_init_pipeline(pip, &(sg_pipeline_desc){
        .shader = sg->shader,
        .layout = {
            // .buffers[0] = {.stride = sizeof(SGNVGattribute)},
            .attrs = {
                [ATTR_nanovg_vs_vertex].format = SG_VERTEXFORMAT_FLOAT2,
                [ATTR_nanovg_vs_tcoord].format = SG_VERTEXFORMAT_FLOAT2,
            },
        },
        .stencil = *stencil,
        .colors[0] = {
            .write_mask = write_mask,
            .blend = sg->blend,
        },
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .index_type = SG_INDEXTYPE_UINT32,
        .cull_mode = cull_mode,
        .face_winding = SG_FACEWINDING_CCW,
        .label = "nanovg.pipeline",
    });
}

static bool sgnvg__pipelineTypeIsInUse(SGNVGcontext* sg, SGNVGpipelineType type)
{
    switch(type)
    {
    case SGNVG_PIP_BASE:
    case SGNVG_PIP_FILL_STENCIL:
    case SGNVG_PIP_FILL_DRAW:
        return true;
    case SGNVG_PIP_FILL_ANTIALIAS:
        return !!(sg->flags & NVG_ANTIALIAS);
    case SGNVG_PIP_STROKE_STENCIL_DRAW:
    case SGNVG_PIP_STROKE_STENCIL_ANTIALIAS:
    case SGNVG_PIP_STROKE_STENCIL_CLEAR:
        return !!(sg->flags & NVG_STENCIL_STROKES);

    case SGNVG_PIP_NUM_:    // to avoid warnings
        break; /* fall through to assert */
    }
    assert(0);
    return false;
}

static int sgnvg__getIndexFromCache(SGNVGcontext* sg, uint16_t blendNumber)
{
    uint16_t currentUse = sg->pipelineCache.currentUse;

    int maxAge = 0;
    int maxAgeIndex = 0;

    // find the correct cache entry for `blend_number`
    for(unsigned int i = 0; i < NANOVG_SG_PIPELINE_CACHE_SIZE; i++)
    {
        if(sg->pipelineCache.keys[i].blend == blendNumber)
        {
            sg->pipelineCache.keys[i].lastUse = sg->pipelineCache.currentUse;
            return i;
        }
        int age = (uint16_t)(currentUse - sg->pipelineCache.keys[i].lastUse);
        if(age > maxAge)
        {
            maxAge = age;
            maxAgeIndex = i;
        }
    }

    // not found; reuse an old one
    sg->pipelineCache.currentUse = ++currentUse;
    sg->pipelineCache.keys[maxAgeIndex].blend = blendNumber;
    sg->pipelineCache.keys[maxAgeIndex].lastUse = currentUse;

    sg_pipeline* pipelines = sg->pipelineCache.pipelines[maxAgeIndex];
    uint8_t pipelinesActive = sg->pipelineCache.pipelinesActive[maxAgeIndex];
    // we may have had data already initialized; deinit those
    for(uint32_t type = SGNVG_PIP_BASE; type < SGNVG_PIP_NUM_; type++)
        if(pipelinesActive & (1 << type))
            sg_uninit_pipeline(pipelines[type]);
    // mark all as inactive
    sg->pipelineCache.pipelinesActive[maxAgeIndex] = 0;
    return maxAgeIndex;
}

static sg_pipeline sgnvg__getPipelineFromCache(SGNVGcontext* sg, SGNVGpipelineType type)
{
    assert(sgnvg__pipelineTypeIsInUse(sg, type));

    int pipelineCacheIndex = sg->pipelineCacheIndex;
    sg_pipeline pipeline = sg->pipelineCache.pipelines[pipelineCacheIndex][type];
    uint8_t typeMask = 1 << type;
    if(!(sg->pipelineCache.pipelinesActive[pipelineCacheIndex] & typeMask))
    {
        sg->pipelineCache.pipelinesActive[pipelineCacheIndex] |= typeMask;
        switch(type)
        {
        case SGNVG_PIP_BASE:
            sgnvg__initPipeline(sg, pipeline, &(sg_stencil_state){
                .enabled = false,
            }, SG_COLORMASK_RGBA, SG_CULLMODE_BACK);
            break;

        case SGNVG_PIP_FILL_STENCIL:
            sgnvg__initPipeline(sg, pipeline, &(sg_stencil_state){
                .enabled = true,
                .front = {.compare = SG_COMPAREFUNC_ALWAYS, .fail_op = SG_STENCILOP_KEEP, .depth_fail_op = SG_STENCILOP_KEEP, .pass_op = SG_STENCILOP_INCR_WRAP},
                .back = {.compare = SG_COMPAREFUNC_ALWAYS, .fail_op = SG_STENCILOP_KEEP, .depth_fail_op = SG_STENCILOP_KEEP, .pass_op = SG_STENCILOP_DECR_WRAP},
                .read_mask = 0xFF,
                .write_mask = 0xFF,
                .ref = 0,
            }, SG_COLORMASK_NONE, SG_CULLMODE_NONE);
            break;
        case SGNVG_PIP_FILL_ANTIALIAS:
            sgnvg__initPipeline(sg, pipeline, &(sg_stencil_state){
                .enabled = true,
                .front = {.compare = SG_COMPAREFUNC_EQUAL, .fail_op = SG_STENCILOP_KEEP, .depth_fail_op = SG_STENCILOP_KEEP, .pass_op = SG_STENCILOP_KEEP},
                .back = {.compare = SG_COMPAREFUNC_EQUAL, .fail_op = SG_STENCILOP_KEEP, .depth_fail_op = SG_STENCILOP_KEEP, .pass_op = SG_STENCILOP_KEEP},
                .read_mask = 0xFF,
                .write_mask = 0xFF,
                .ref = 0,
            }, SG_COLORMASK_RGBA, SG_CULLMODE_BACK);
            break;
        case SGNVG_PIP_FILL_DRAW:
            sgnvg__initPipeline(sg, pipeline, &(sg_stencil_state){
                .enabled = true,
                .front = {.compare = SG_COMPAREFUNC_NOT_EQUAL, .fail_op = SG_STENCILOP_ZERO, .depth_fail_op = SG_STENCILOP_ZERO, .pass_op = SG_STENCILOP_ZERO},
                .back = {.compare = SG_COMPAREFUNC_NOT_EQUAL, .fail_op = SG_STENCILOP_ZERO, .depth_fail_op = SG_STENCILOP_ZERO, .pass_op = SG_STENCILOP_ZERO},
                .read_mask = 0xFF,
                .write_mask = 0xFF,
                .ref = 0,
            }, SG_COLORMASK_RGBA, SG_CULLMODE_BACK);
            break;

        case SGNVG_PIP_STROKE_STENCIL_DRAW:
            sgnvg__initPipeline(sg, pipeline, &(sg_stencil_state){
                .enabled = true,
                .front = {.compare = SG_COMPAREFUNC_EQUAL, .fail_op = SG_STENCILOP_KEEP, .depth_fail_op = SG_STENCILOP_KEEP, .pass_op = SG_STENCILOP_INCR_CLAMP},
                .back = {.compare = SG_COMPAREFUNC_EQUAL, .fail_op = SG_STENCILOP_KEEP, .depth_fail_op = SG_STENCILOP_KEEP, .pass_op = SG_STENCILOP_INCR_CLAMP},
                .read_mask = 0xFF,
                .write_mask = 0xFF,
                .ref = 0,
            }, SG_COLORMASK_RGBA, SG_CULLMODE_BACK);
            break;
        case SGNVG_PIP_STROKE_STENCIL_ANTIALIAS:
            sgnvg__initPipeline(sg, pipeline, &(sg_stencil_state){
                .enabled = true,
                .front = {.compare = SG_COMPAREFUNC_EQUAL, .fail_op = SG_STENCILOP_KEEP, .depth_fail_op = SG_STENCILOP_KEEP, .pass_op = SG_STENCILOP_KEEP},
                .back = {.compare = SG_COMPAREFUNC_EQUAL, .fail_op = SG_STENCILOP_KEEP, .depth_fail_op = SG_STENCILOP_KEEP, .pass_op = SG_STENCILOP_KEEP},
                .read_mask = 0xFF,
                .write_mask = 0xFF,
                .ref = 0,
            }, SG_COLORMASK_RGBA, SG_CULLMODE_BACK);
            break;
        case SGNVG_PIP_STROKE_STENCIL_CLEAR:
            sgnvg__initPipeline(sg, pipeline, &(sg_stencil_state){
                .enabled = true,
                .front = {.compare = SG_COMPAREFUNC_ALWAYS, .fail_op = SG_STENCILOP_ZERO, .depth_fail_op = SG_STENCILOP_ZERO, .pass_op = SG_STENCILOP_ZERO},
                .back = {.compare = SG_COMPAREFUNC_ALWAYS, .fail_op = SG_STENCILOP_ZERO, .depth_fail_op = SG_STENCILOP_ZERO, .pass_op = SG_STENCILOP_ZERO},
                .read_mask = 0xFF,
                .write_mask = 0xFF,
                .ref = 0,
            }, SG_COLORMASK_NONE, SG_CULLMODE_BACK);
            break;

        default:
            assert(0);
        }
    }
    return pipeline;
}

static SGNVGfragUniforms* nvg__fragUniformPtr(SGNVGcontext* gl, int i);

static void sgnvg__setUniforms(SGNVGcontext* sg, int uniformOffset, int image)
{
    SGNVGtexture* tex = NULL;
    SGNVGfragUniforms* frag = nvg__fragUniformPtr(sg, uniformOffset);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_nanovg_viewSize, &(sg_range){ &sg->view, sizeof(sg->view) });
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_nanovg_frag, &(sg_range){ frag, sizeof(*frag) });

    if (image != 0) {
        tex = sgnvg__findTexture(sg, image);
    }
    // If no image is set, use empty texture
    if (tex == NULL) {
        tex = sgnvg__findTexture(sg, sg->dummyTex);
    }
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = sg->vertBuf,
        .index_buffer = sg->indexBuf,
        .fs.images[SLOT_nanovg_tex] = tex ? tex->img : (sg_image){0},
        .fs.samplers[SLOT_nanovg_smp] = tex ? tex->smp : (sg_sampler){0},
    });
}

static void sgnvg__preparePipelineUniforms(SGNVGcontext* sg, SGNVGpipelineType pipelineType, int uniformOffset, int image)
{
    sg_pipeline pip = sgnvg__getPipelineFromCache(sg, pipelineType);
    sg_apply_pipeline(pip);
    sgnvg__setUniforms(sg, uniformOffset, image);
}

#define NANOVG_SG_TOSTRING_(X)  #X
#define NANOVG_SG_TOSTRING(X)   NANOVG_SG_TOSTRING_(X)

static int sgnvg__renderCreateTexture(void* uptr, int type, int w, int h, int imageFlags, const unsigned char* data);

static int sgnvg__renderCreate(void* uptr)
{
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    int align = 4;

    if(sg->flags & NVG_ANTIALIAS)  // insert NANOVG_SG_EDGE_AA_HEADER between header & body
        sg->shader = sg_make_shader(nanovg_sg_aa_shader_desc(sg_query_backend()));
    else
        sg->shader = sg_make_shader(nanovg_sg_shader_desc(sg_query_backend()));
    for(int i = 0; i < NANOVG_SG_PIPELINE_CACHE_SIZE; i++)
    {
        for(uint32_t t = 0; t < SGNVG_PIP_NUM_; t++)
        {
            // only allocate pipelines if correct flags are set
            if(!sgnvg__pipelineTypeIsInUse(sg, (SGNVGpipelineType)t))
                continue;
            sg->pipelineCache.pipelines[i][t] = sg_alloc_pipeline();
        }
    }

    sg->blend = (sg_blend_state){
        .enabled = true,
        .src_factor_rgb = SG_BLENDFACTOR_ZERO,
        .dst_factor_rgb = SG_BLENDFACTOR_ZERO,
        .op_rgb = SG_BLENDOP_ADD,
        .src_factor_alpha = SG_BLENDFACTOR_ZERO,
        .dst_factor_alpha = SG_BLENDFACTOR_ZERO,
        .op_alpha = SG_BLENDOP_ADD,
    };

    sg->vertBuf = sg_alloc_buffer();
    sg->indexBuf = sg_alloc_buffer();

    sg->fragSize = sizeof(SGNVGfragUniforms) + (align - sizeof(SGNVGfragUniforms) % align) % align;

    // Some platforms does not allow to have samples to unset textures.
    // Create empty one which is bound when there's no texture specified.
    sg->dummyTex = sgnvg__renderCreateTexture(sg, NVG_TEXTURE_ALPHA, 1, 1, 0, NULL);

    return 1;
}


static int sgnvg__renderCreateTexture(void* uptr, int type, int w, int h, int imageFlags, const unsigned char* data)
{
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    SGNVGtexture* tex = sgnvg__allocTexture(sg);

    if (tex == NULL) return 0;

#ifdef SOKOL_GLES2
    // Check for non-power of 2.
    if (sgnvg__nearestPow2(w) != (unsigned int)w || sgnvg__nearestPow2(h) != (unsigned int)h) {
        // No repeat
        if ((imageFlags & NVG_IMAGE_REPEATX) != 0 || (imageFlags & NVG_IMAGE_REPEATY) != 0) {
            printf("Repeat X/Y is not supported for non power-of-two textures (%d x %d)\n", w, h);
            imageFlags &= ~(NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
        }
        // No mips.
        if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) {
            printf("Mip-maps is not support for non power-of-two textures (%d x %d)\n", w, h);
            imageFlags &= ~NVG_IMAGE_GENERATE_MIPMAPS;
        }
    }
#endif
    assert(!(imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) &&  "TODO mipmaps");

    // if we have mipmaps, we forbid updating
    bool immutable = !!(imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) && data;

    tex->width = w;
    tex->height = h;
    tex->type = type;
    tex->flags = imageFlags;
    sg_image_data imageData = {
            // TODO mipmaps
            .subimage[0][0] = {data, w * h * (type == NVG_TEXTURE_RGBA ? 4 : 1)},
    };
    tex->img = sg_make_image(&(sg_image_desc){
        .type = SG_IMAGETYPE_2D,
        //.render_target
        .width = w,
        .height = h,
        .num_mipmaps = 1,   // TODO mipmaps
        .usage = immutable ? SG_USAGE_IMMUTABLE : SG_USAGE_DYNAMIC,
        .pixel_format = type == NVG_TEXTURE_RGBA ? SG_PIXELFORMAT_RGBA8 : SG_PIXELFORMAT_R8,
        .data = ((imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) && data) ? imageData : (sg_image_data){.subimage[0][0] = {NULL, 0}},
        .label = "nanovg.image[]",
    });
    tex->smp = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = imageFlags & NVG_IMAGE_GENERATE_MIPMAPS
            ? _SG_FILTER_DEFAULT
            : (imageFlags & NVG_IMAGE_NEAREST ? SG_FILTER_NEAREST : SG_FILTER_LINEAR),
        .mipmap_filter = imageFlags & NVG_IMAGE_GENERATE_MIPMAPS
            ? (imageFlags & NVG_IMAGE_NEAREST ? SG_FILTER_NEAREST : SG_FILTER_LINEAR)
            : _SG_FILTER_DEFAULT,
        .mag_filter = imageFlags & NVG_IMAGE_NEAREST ? SG_FILTER_NEAREST : SG_FILTER_LINEAR,
        .wrap_u = imageFlags & NVG_IMAGE_REPEATX ? SG_WRAP_REPEAT : SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = imageFlags & NVG_IMAGE_REPEATY ? SG_WRAP_REPEAT : SG_WRAP_CLAMP_TO_EDGE,
    });
    if(!immutable && data)
        sg_update_image(tex->img, &imageData);

    return tex->id;
}

static int sgnvg__renderDeleteTexture(void* uptr, int image)
{
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    return sgnvg__deleteTexture(sg, image);
}

static int sgnvg__renderUpdateTexture(void* uptr, int image, int x, int y, int w, int h, const unsigned char* data)
{
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    SGNVGtexture* tex = sgnvg__findTexture(sg, image);

    if (tex == NULL) return 0;

    sg_update_image(tex->img, &(sg_image_data){
        .subimage[0][0] = {data, w * h * (tex->type == NVG_TEXTURE_RGBA ? 4 : 1)},
    });

    return 1;
}

static int sgnvg__renderGetTextureSize(void* uptr, int image, int* w, int* h)
{
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    SGNVGtexture* tex = sgnvg__findTexture(sg, image);
    if (tex == NULL) return 0;
    *w = tex->width;
    *h = tex->height;
    return 1;
}

static void sgnvg__xformToMat3x4(float* m3, float* t)
{
    m3[0] = t[0];
    m3[1] = t[1];
    m3[2] = 0.0f;
    m3[3] = 0.0f;
    m3[4] = t[2];
    m3[5] = t[3];
    m3[6] = 0.0f;
    m3[7] = 0.0f;
    m3[8] = t[4];
    m3[9] = t[5];
    m3[10] = 1.0f;
    m3[11] = 0.0f;
}

static NVGcolor sgnvg__premulColor(NVGcolor c)
{
    c.r *= c.a;
    c.g *= c.a;
    c.b *= c.a;
    return c;
}

static int sgnvg__convertPaint(SGNVGcontext* sg, SGNVGfragUniforms* frag, NVGpaint* paint,
                               NVGscissor* scissor, float width, float fringe, float strokeThr)
{
    SGNVGtexture* tex = NULL;
    float invxform[6];

    memset(frag, 0, sizeof(*frag));

    frag->innerCol = sgnvg__premulColor(paint->innerColor);
    frag->outerCol = sgnvg__premulColor(paint->outerColor);

    if (scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f) {
        memset(frag->scissorMat, 0, sizeof(frag->scissorMat));
        frag->scissorExt[0] = 1.0f;
        frag->scissorExt[1] = 1.0f;
        frag->scissorScale[0] = 1.0f;
        frag->scissorScale[1] = 1.0f;
    } else {
        nvgTransformInverse(invxform, scissor->xform);
        sgnvg__xformToMat3x4(frag->scissorMat, invxform);
        frag->scissorExt[0] = scissor->extent[0];
        frag->scissorExt[1] = scissor->extent[1];
        frag->scissorScale[0] = sqrtf(scissor->xform[0]*scissor->xform[0] + scissor->xform[2]*scissor->xform[2]) / fringe;
        frag->scissorScale[1] = sqrtf(scissor->xform[1]*scissor->xform[1] + scissor->xform[3]*scissor->xform[3]) / fringe;
    }

    memcpy(frag->extent, paint->extent, sizeof(frag->extent));
    frag->strokeMult = (width*0.5f + fringe*0.5f) / fringe;
    frag->strokeThr = strokeThr;

    if (paint->image != 0) {
        tex = sgnvg__findTexture(sg, paint->image);
        if (tex == NULL) return 0;
        if ((tex->flags & NVG_IMAGE_FLIPY) != 0) {
            float m1[6], m2[6];
            nvgTransformTranslate(m1, 0.0f, frag->extent[1] * 0.5f);
            nvgTransformMultiply(m1, paint->xform);
            nvgTransformScale(m2, 1.0f, -1.0f);
            nvgTransformMultiply(m2, m1);
            nvgTransformTranslate(m1, 0.0f, -frag->extent[1] * 0.5f);
            nvgTransformMultiply(m1, m2);
            nvgTransformInverse(invxform, m1);
        } else {
            nvgTransformInverse(invxform, paint->xform);
        }
        frag->type = NSVG_SHADER_FILLIMG;

        #if NANOVG_GL_USE_UNIFORMBUFFER
        if (tex->type == NVG_TEXTURE_RGBA)
            frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0 : 1;
        else
            frag->texType = 2;
        #else
        if (tex->type == NVG_TEXTURE_RGBA)
            frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0.0f : 1.0f;
        else
            frag->texType = 2.0f;
        #endif
//		printf("frag->texType = %d\n", frag->texType);
    } else {
        frag->type = NSVG_SHADER_FILLGRAD;
        frag->radius = paint->radius;
        frag->feather = paint->feather;
        nvgTransformInverse(invxform, paint->xform);
    }

    sgnvg__xformToMat3x4(frag->paintMat, invxform);

    return 1;
}

static void sgnvg__renderViewport(void* uptr, float width, float height, float devicePixelRatio)
{
    NVG_NOTUSED(devicePixelRatio);
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    sg->view.viewSize[0] = width;
    sg->view.viewSize[1] = height;
}

static void sgnvg__fill(SGNVGcontext* sg, SGNVGcall* call)
{
    SGNVGpath* paths = &sg->paths[call->pathOffset];
    int i, npaths = call->pathCount;

    sgnvg__preparePipelineUniforms(sg, SGNVG_PIP_FILL_STENCIL, call->uniformOffset, 0);
    for (i = 0; i < npaths; i++)
        sg_draw(paths[i].fillOffset, paths[i].fillCount, 1);

    if (sg->flags & NVG_ANTIALIAS) {
        sgnvg__preparePipelineUniforms(sg, SGNVG_PIP_FILL_ANTIALIAS, call->uniformOffset + sg->fragSize, call->image);
        // Draw fringes
        for (i = 0; i < npaths; i++)
            sg_draw(paths[i].strokeOffset, paths[i].strokeCount, 1);
    }

    // Draw fill
    sgnvg__preparePipelineUniforms(sg, SGNVG_PIP_FILL_DRAW, call->uniformOffset + sg->fragSize, call->image);
    sg_draw(call->triangleOffset, call->triangleCount, 1);
}

static void sgnvg__convexFill(SGNVGcontext* sg, SGNVGcall* call)
{
    SGNVGpath* paths = &sg->paths[call->pathOffset];
    int i, npaths = call->pathCount;

    sgnvg__preparePipelineUniforms(sg, SGNVG_PIP_BASE, call->uniformOffset, call->image);
    for (i = 0; i < npaths; i++) {
        sg_draw(paths[i].fillOffset, paths[i].fillCount, 1);
        // Draw fringes
        if (paths[i].strokeCount > 0) {
            sg_draw(paths[i].strokeOffset, paths[i].strokeCount, 1);
        }
    }
}

static void sgnvg__stroke(SGNVGcontext* sg, SGNVGcall* call)
{
    SGNVGpath* paths = &sg->paths[call->pathOffset];
    int npaths = call->pathCount, i;

    if (sg->flags & NVG_STENCIL_STROKES) {
        sgnvg__preparePipelineUniforms(sg, SGNVG_PIP_STROKE_STENCIL_DRAW, call->uniformOffset + sg->fragSize, call->image);
        for (i = 0; i < npaths; i++)
            sg_draw(paths[i].strokeOffset, paths[i].strokeCount, 1);

        // Draw anti-aliased pixels.
        sgnvg__preparePipelineUniforms(sg, SGNVG_PIP_STROKE_STENCIL_ANTIALIAS, call->uniformOffset, call->image);
        for (i = 0; i < npaths; i++)
            sg_draw(paths[i].strokeOffset, paths[i].strokeCount, 1);

        // Clear stencil buffer.
        sgnvg__preparePipelineUniforms(sg, SGNVG_PIP_STROKE_STENCIL_CLEAR, call->uniformOffset, 0);
        for (i = 0; i < npaths; i++)
            sg_draw(paths[i].strokeOffset, paths[i].strokeCount, 1);

//		sgnvg__convertPaint(sg, nvg__fragUniformPtr(sg, call->uniformOffset + sg->fragSize), paint, scissor, strokeWidth, fringe, 1.0f - 0.5f/255.0f);

    } else {
        sgnvg__preparePipelineUniforms(sg, SGNVG_PIP_BASE, call->uniformOffset, call->image);
        // Draw Strokes
        for (i = 0; i < npaths; i++)
            sg_draw(paths[i].strokeOffset, paths[i].strokeCount, 1);
    }
}

static void sgnvg__triangles(SGNVGcontext* sg, SGNVGcall* call)
{
    sgnvg__preparePipelineUniforms(sg, SGNVG_PIP_BASE, call->uniformOffset, call->image);
    sg_draw(call->triangleOffset, call->triangleCount, 1);
}

static void sgnvg__renderCancel(void* uptr) {
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    sg->nverts = 0;
    sg->npaths = 0;
    sg->ncalls = 0;
    sg->nuniforms = 0;
}

static sg_blend_factor sgnvg_convertBlendFuncFactor(int factor)
{
    if (factor == NVG_ZERO)
        return SG_BLENDFACTOR_ZERO;
    if (factor == NVG_ONE)
        return SG_BLENDFACTOR_ONE;
    if (factor == NVG_SRC_COLOR)
        return SG_BLENDFACTOR_SRC_COLOR;
    if (factor == NVG_ONE_MINUS_SRC_COLOR)
        return SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
    if (factor == NVG_DST_COLOR)
        return SG_BLENDFACTOR_DST_COLOR;
    if (factor == NVG_ONE_MINUS_DST_COLOR)
        return SG_BLENDFACTOR_ONE_MINUS_DST_COLOR;
    if (factor == NVG_SRC_ALPHA)
        return SG_BLENDFACTOR_SRC_ALPHA;
    if (factor == NVG_ONE_MINUS_SRC_ALPHA)
        return SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    if (factor == NVG_DST_ALPHA)
        return SG_BLENDFACTOR_DST_ALPHA;
    if (factor == NVG_ONE_MINUS_DST_ALPHA)
        return SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA;
    if (factor == NVG_SRC_ALPHA_SATURATE)
        return SG_BLENDFACTOR_SRC_ALPHA_SATURATED;
    return _SG_BLENDFACTOR_DEFAULT;
}

static SGNVGblend sgnvg__blendCompositeOperation(NVGcompositeOperationState op)
{
    SGNVGblend blend;
    blend.srcRGB = sgnvg_convertBlendFuncFactor(op.srcRGB);
    blend.dstRGB = sgnvg_convertBlendFuncFactor(op.dstRGB);
    blend.srcAlpha = sgnvg_convertBlendFuncFactor(op.srcAlpha);
    blend.dstAlpha = sgnvg_convertBlendFuncFactor(op.dstAlpha);
    if (blend.srcRGB == _SG_BLENDFACTOR_DEFAULT || blend.dstRGB == _SG_BLENDFACTOR_DEFAULT || blend.srcAlpha == _SG_BLENDFACTOR_DEFAULT || blend.dstAlpha == _SG_BLENDFACTOR_DEFAULT)
    {
        blend.srcRGB = SG_BLENDFACTOR_ONE;
        blend.dstRGB = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        blend.srcAlpha = SG_BLENDFACTOR_ONE;
        blend.dstAlpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    }
    return blend;
}

static void sgnvg__renderFlush(void* uptr)
{
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    int i;

    if (sg->ncalls > 0 && sg->nverts && sg->nindexes) {
        if(sg->cverts_gpu < sg->nverts) // resize GPU vertex buffer
        {
            if(sg->cverts_gpu)      // delete old buffer if necessary
                sg_uninit_buffer(sg->vertBuf);
            sg->cverts_gpu = sg->cverts;
            sg_init_buffer(sg->vertBuf, &(sg_buffer_desc){
                .size = sg->cverts_gpu * sizeof(*sg->verts),
                .type = SG_BUFFERTYPE_VERTEXBUFFER,
                .usage = SG_USAGE_STREAM,
                .label = "nanovg.vertBuf",
            });
        }
        // upload vertex data
        sg_update_buffer(sg->vertBuf, &(sg_range){ sg->verts, sg->nverts * sizeof(*sg->verts) });

        if(sg->cindexes_gpu < sg->nindexes) // resize GPU index buffer
        {
            if(sg->cindexes_gpu)    // delete old buffer if necessary
                sg_uninit_buffer(sg->indexBuf);
            sg->cindexes_gpu = sg->cindexes;
            sg_init_buffer(sg->indexBuf, &(sg_buffer_desc){
                .size = sg->cindexes_gpu * sizeof(*sg->indexes),
                .type = SG_BUFFERTYPE_INDEXBUFFER,
                .usage = SG_USAGE_STREAM,
                .label = "nanovg.indexBuf",
            });
        }
        // upload index data
        sg_update_buffer(sg->indexBuf, &(sg_range){ sg->indexes, sg->nindexes * sizeof(*sg->indexes) });

        for (i = 0; i < sg->ncalls; i++) {
            SGNVGcall* call = &sg->calls[i];
            sg->blend.src_factor_rgb = call->blendFunc.srcRGB;
            sg->blend.dst_factor_rgb = call->blendFunc.dstRGB;
            sg->blend.src_factor_alpha = call->blendFunc.srcAlpha;
            sg->blend.dst_factor_alpha = call->blendFunc.dstAlpha;
            sg->pipelineCacheIndex = sgnvg__getIndexFromCache(sg, sgnvg__getCombinedBlendNumber(sg->blend));
            if (call->type == SGNVG_FILL)
                sgnvg__fill(sg, call);
            else if (call->type == SGNVG_CONVEXFILL)
                sgnvg__convexFill(sg, call);
            else if (call->type == SGNVG_STROKE)
                sgnvg__stroke(sg, call);
            else if (call->type == SGNVG_TRIANGLES)
                sgnvg__triangles(sg, call);
        }

        //sg_uninit_pipeline(sg->pipeline);
        //sgnvg__initPipeline(sg, &(sg_stencil_state){0}, SG_COLORMASK_RGBA, SG_CULLMODE_NONE);
    }

    // Reset calls
    sg->nverts = 0;
    sg->nindexes = 0;
    sg->npaths = 0;
    sg->ncalls = 0;
    sg->nuniforms = 0;
}

static int sgnvg__maxVertCount(const NVGpath* paths, int npaths)
{
    int i, count = 0;
    for (i = 0; i < npaths; i++) {
        count += paths[i].nfill;
        count += paths[i].nstroke;
    }
    return count;
}

static int sgnvg__maxIndexCount(const NVGpath* paths, int npaths)
{
    int i, count = 0;
    for (i = 0; i < npaths; i++) {
        count += sgnvg__maxi(paths[i].nfill - 2, 0) * 3;      // triangle fan
        count += sgnvg__maxi(paths[i].nstroke - 2, 0) * 3;    // triangle strip
    }
    return count;
}

static SGNVGcall* sgnvg__allocCall(SGNVGcontext* sg)
{
    SGNVGcall* ret = NULL;
    if (sg->ncalls+1 > sg->ccalls) {
        SGNVGcall* calls;
        int ccalls = sgnvg__maxi(sg->ncalls+1, 128) + sg->ccalls/2; // 1.5x Overallocate
        calls = (SGNVGcall*)realloc(sg->calls, sizeof(SGNVGcall) * ccalls);
        if (calls == NULL) return NULL;
        sg->calls = calls;
        sg->ccalls = ccalls;
    }
    ret = &sg->calls[sg->ncalls++];
    memset(ret, 0, sizeof(SGNVGcall));
    return ret;
}

static int sgnvg__allocPaths(SGNVGcontext* sg, int n)
{
    int ret = 0;
    if (sg->npaths+n > sg->cpaths) {
        SGNVGpath* paths;
        int cpaths = sgnvg__maxi(sg->npaths + n, 128) + sg->cpaths/2; // 1.5x Overallocate
        paths = (SGNVGpath*)realloc(sg->paths, sizeof(SGNVGpath) * cpaths);
        if (paths == NULL) return -1;
        sg->paths = paths;
        sg->cpaths = cpaths;
    }
    ret = sg->npaths;
    sg->npaths += n;
    return ret;
}

static int sgnvg__allocVerts(SGNVGcontext* sg, int n)
{
    int ret = 0;
    if (sg->nverts+n > sg->cverts) {
        SGNVGattribute* verts;
        int cverts = sgnvg__maxi(sg->nverts + n, 4096) + sg->cverts/2; // 1.5x Overallocate
        verts = (SGNVGattribute*)realloc(sg->verts, sizeof(SGNVGattribute) * cverts);
        if (verts == NULL) return -1;
        sg->verts = verts;
        sg->cverts = cverts;
    }
    ret = sg->nverts;
    sg->nverts += n;
    return ret;
}

static int sgnvg__allocIndexes(SGNVGcontext* sg, int n)
{
    int ret = 0;
    if (sg->nindexes+n > sg->cindexes) {
        uint32_t* indexes;
        int cindexes = sgnvg__maxi(sg->nindexes + n, 4096) + sg->cindexes/2; // 1.5x Overallocate
        indexes = (uint32_t*)realloc(sg->indexes, sizeof(uint32_t) * cindexes);
        if (indexes == NULL) return -1;
        sg->indexes = indexes;
        sg->cindexes = cindexes;
    }
    ret = sg->nindexes;
    sg->nindexes += n;
    return ret;
}

static int sgnvg__allocFragUniforms(SGNVGcontext* sg, int n)
{
    int ret = 0, structSize = sg->fragSize;
    if (sg->nuniforms+n > sg->cuniforms) {
        unsigned char* uniforms;
        int cuniforms = sgnvg__maxi(sg->nuniforms+n, 128) + sg->cuniforms/2; // 1.5x Overallocate
        uniforms = (unsigned char*)realloc(sg->uniforms, structSize * cuniforms);
        if (uniforms == NULL) return -1;
        sg->uniforms = uniforms;
        sg->cuniforms = cuniforms;
    }
    ret = sg->nuniforms * structSize;
    sg->nuniforms += n;
    return ret;
}

static SGNVGfragUniforms* nvg__fragUniformPtr(SGNVGcontext* gl, int i)
{
    return (SGNVGfragUniforms*)&gl->uniforms[i];
}

static void sgnvg__vset(SGNVGattribute* vtx, float x, float y, float u, float v)
{
    vtx->vertex[0] = x;
    vtx->vertex[1] = y;
    vtx->tcoord[0] = u;
    vtx->tcoord[1] = v;
}

static void sgnvg__generateTriangleFanIndexes(uint32_t* indexes, int offset, int nverts)
{
    // following triangles all use starting vertex, previous vertex, and current vertex
    for(int i = 2; i < nverts; i++)
    {
        indexes[3*(i-2)+0] = offset+0;
        indexes[3*(i-2)+1] = offset+i-1;
        indexes[3*(i-2)+2] = offset+i;
    }
}

static void sgnvg__generateTriangleStripIndexes(uint32_t* indexes, int offset, int nverts)
{
    // following triangles all use previous 2 vertices, and current vertex
    // we use bit-shifts to get the sequence:
    // i  idx i(bits)   i-1(bits)   i-2(bits)
    // 2: 012 0010      0001        0000
    // 3: 213 0011      0010        0001
    // 4: 234 0100      0011        0010
    // 5: 435 0101      0100        0011
    // 6: 456 0110      0101        0100
    // 7: 657 0111      0110        0101
    //                  first index = above & ~1 = floor_to_even(i-1)
    //                              second index = above | 1 = ceil_to_even(i-2)
    // all this trickery ensures that we maintain correct (CCW) vertex order
    for(int i = 2; i < nverts; i++)
    {
        indexes[3*(i-2)+0] = offset+((i-1)&~1);
        indexes[3*(i-2)+1] = offset+((i-2)|1);
        indexes[3*(i-2)+2] = offset+i;
    }
}

static void sgnvg__renderFill(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe,
                              const float* bounds, const NVGpath* paths, int npaths)
{
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    SGNVGcall* call = sgnvg__allocCall(sg);
    SGNVGattribute* quad;
    SGNVGfragUniforms* frag;
    int i, maxverts, offset, maxindexes, ioffset;

    if (call == NULL) return;

    call->type = SGNVG_FILL;
    call->triangleCount = 4;
    call->pathOffset = sgnvg__allocPaths(sg, npaths);
    if (call->pathOffset == -1) goto error;
    call->pathCount = npaths;
    call->image = paint->image;
    call->blendFunc = sgnvg__blendCompositeOperation(compositeOperation);

    if (npaths == 1 && paths[0].convex)
    {
        call->type = SGNVG_CONVEXFILL;
        call->triangleCount = 0;	// Bounding box fill quad not needed for convex fill
    }

    // Allocate vertices for all the paths.
    maxverts = sgnvg__maxVertCount(paths, npaths) + call->triangleCount;
    offset = sgnvg__allocVerts(sg, maxverts);
    if (offset == -1) goto error;
    maxindexes = sgnvg__maxIndexCount(paths, npaths) + sgnvg__maxi(call->triangleCount - 2, 0) * 3;
    ioffset = sgnvg__allocIndexes(sg, maxindexes);
    if (ioffset == -1) goto error;

    for (i = 0; i < npaths; i++) {
        SGNVGpath* copy = &sg->paths[call->pathOffset + i];
        const NVGpath* path = &paths[i];
        memset(copy, 0, sizeof(SGNVGpath));
        if (path->nfill > 0) {
            // fill: triangle fan
            copy->fillOffset = ioffset;
            copy->fillCount = (path->nfill - 2) * 3;
            memcpy(&sg->verts[offset], path->fill, sizeof(NVGvertex) * path->nfill);
            sgnvg__generateTriangleFanIndexes(&sg->indexes[ioffset], offset, path->nfill);
            offset += path->nfill;
            ioffset += copy->fillCount;
        }
        if (path->nstroke > 0) {
            // stroke: triangle strip
            copy->strokeOffset = ioffset;
            copy->strokeCount = (path->nstroke - 2) * 3;
            memcpy(&sg->verts[offset], path->stroke, sizeof(NVGvertex) * path->nstroke);
            sgnvg__generateTriangleStripIndexes(&sg->indexes[ioffset], offset, path->nstroke);
            offset += path->nstroke;
            ioffset += copy->strokeCount;
        }
    }

    // Setup uniforms for draw calls
    if (call->type == SGNVG_FILL) {
        // Quad
        call->triangleOffset = ioffset;
        call->triangleCount = (call->triangleCount - 2) * 3;    // convert vertex count into index
        quad = &sg->verts[offset];
        sgnvg__vset(&quad[0], bounds[2], bounds[3], 0.5f, 1.0f);
        sgnvg__vset(&quad[1], bounds[2], bounds[1], 0.5f, 1.0f);
        sgnvg__vset(&quad[2], bounds[0], bounds[3], 0.5f, 1.0f);
        sgnvg__vset(&quad[3], bounds[0], bounds[1], 0.5f, 1.0f);
        sgnvg__generateTriangleStripIndexes(&sg->indexes[ioffset], offset, 4);
        call->uniformOffset = sgnvg__allocFragUniforms(sg, 2);
        if (call->uniformOffset == -1) goto error;
        // Simple shader for stencil
        frag = nvg__fragUniformPtr(sg, call->uniformOffset);
        memset(frag, 0, sizeof(*frag));
        frag->strokeThr = -1.0f;
        frag->type = NSVG_SHADER_SIMPLE;
        // Fill shader
        sgnvg__convertPaint(sg, nvg__fragUniformPtr(sg, call->uniformOffset + sg->fragSize), paint, scissor, fringe, fringe, -1.0f);
    } else {
        call->uniformOffset = sgnvg__allocFragUniforms(sg, 1);
        if (call->uniformOffset == -1) goto error;
        // Fill shader
        sgnvg__convertPaint(sg, nvg__fragUniformPtr(sg, call->uniformOffset), paint, scissor, fringe, fringe, -1.0f);
    }

    return;

error:
    // We get here if call alloc was ok, but something else is not.
    // Roll back the last call to prevent drawing it.
    if (sg->ncalls > 0) sg->ncalls--;
}

static void sgnvg__renderStroke(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe,
                                float strokeWidth, const NVGpath* paths, int npaths)
{
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    SGNVGcall* call = sgnvg__allocCall(sg);
    int i, maxverts, offset, maxindexes, ioffset;

    if (call == NULL) return;

    call->type = SGNVG_STROKE;
    call->pathOffset = sgnvg__allocPaths(sg, npaths);
    if (call->pathOffset == -1) goto error;
    call->pathCount = npaths;
    call->image = paint->image;
    call->blendFunc = sgnvg__blendCompositeOperation(compositeOperation);

    // Allocate vertices for all the paths.
    maxverts = sgnvg__maxVertCount(paths, npaths);
    offset = sgnvg__allocVerts(sg, maxverts);
    if (offset == -1) goto error;
    maxindexes = sgnvg__maxIndexCount(paths, npaths);
    ioffset = sgnvg__allocIndexes(sg, maxindexes);
    if (ioffset == -1) goto error;

    for (i = 0; i < npaths; i++) {
        SGNVGpath* copy = &sg->paths[call->pathOffset + i];
        const NVGpath* path = &paths[i];
        memset(copy, 0, sizeof(SGNVGpath));
        if (path->nstroke) {
            // stroke: triangle strip
            copy->strokeOffset = ioffset;
            copy->strokeCount = (path->nstroke - 2) * 3;
            memcpy(&sg->verts[offset], path->stroke, sizeof(NVGvertex) * path->nstroke);
            sgnvg__generateTriangleStripIndexes(&sg->indexes[ioffset], offset, path->nstroke);
            offset += path->nstroke;
            ioffset += copy->strokeCount;
        }
    }

    if (sg->flags & NVG_STENCIL_STROKES) {
        // Fill shader
        call->uniformOffset = sgnvg__allocFragUniforms(sg, 2);
        if (call->uniformOffset == -1) goto error;

        sgnvg__convertPaint(sg, nvg__fragUniformPtr(sg, call->uniformOffset), paint, scissor, strokeWidth, fringe, -1.0f);
        sgnvg__convertPaint(sg, nvg__fragUniformPtr(sg, call->uniformOffset + sg->fragSize), paint, scissor, strokeWidth, fringe, 1.0f - 0.5f/255.0f);

    } else {
        // Fill shader
        call->uniformOffset = sgnvg__allocFragUniforms(sg, 1);
        if (call->uniformOffset == -1) goto error;
        sgnvg__convertPaint(sg, nvg__fragUniformPtr(sg, call->uniformOffset), paint, scissor, strokeWidth, fringe, -1.0f);
    }

    return;

error:
    // We get here if call alloc was ok, but something else is not.
    // Roll back the last call to prevent drawing it.
    if (sg->ncalls > 0) sg->ncalls--;
}

static void sgnvg__renderTriangles(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor,
                                   const NVGvertex* verts, int nverts, float fringe)
{
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    SGNVGcall* call = sgnvg__allocCall(sg);
    SGNVGfragUniforms* frag;

    if (call == NULL) return;

    call->type = SGNVG_TRIANGLES;
    call->image = paint->image;
    call->blendFunc = sgnvg__blendCompositeOperation(compositeOperation);

    int offset, ioffset;
    offset = sgnvg__allocVerts(sg, nverts);
    if(offset == -1) goto error;
    ioffset = sgnvg__allocIndexes(sg, nverts);
    if(ioffset == -1) goto error;

    // Allocate vertices for all the paths.
    call->triangleOffset = ioffset;
    call->triangleCount = nverts;
    memcpy(&sg->verts[offset], verts, sizeof(NVGvertex) * nverts);
    for(int i = 0; i < nverts; i++)
        sg->indexes[ioffset+i] = offset+i;

    // Fill shader
    call->uniformOffset = sgnvg__allocFragUniforms(sg, 1);
    if (call->uniformOffset == -1) goto error;
    frag = nvg__fragUniformPtr(sg, call->uniformOffset);
    sgnvg__convertPaint(sg, frag, paint, scissor, 1.0f, fringe, -1.0f);
    frag->type = NSVG_SHADER_IMG;

    return;

error:
    // We get here if call alloc was ok, but something else is not.
    // Roll back the last call to prevent drawing it.
    if (sg->ncalls > 0) sg->ncalls--;
}

static void sgnvg__renderDelete(void* uptr)
{
    SGNVGcontext* sg = (SGNVGcontext*)uptr;
    int i;
    if (sg == NULL) return;

    sg_destroy_shader(sg->shader);

    for(int i = 0; i < NANOVG_SG_PIPELINE_CACHE_SIZE; i++)
    {
        for(uint32_t t = 0; t < SGNVG_PIP_NUM_; t++)
        {
            // only uninitialize if correct flags are set
            if(!sgnvg__pipelineTypeIsInUse(sg, (SGNVGpipelineType)t))
                continue;
            if(sg->pipelineCache.pipelinesActive[i] & (1 << t))
                sg_uninit_pipeline(sg->pipelineCache.pipelines[i][t]);
            sg_dealloc_pipeline(sg->pipelineCache.pipelines[i][t]);
        }
    }

    if(sg->cverts_gpu)
        sg_uninit_buffer(sg->vertBuf);
    sg_dealloc_buffer(sg->vertBuf);

    if(sg->cindexes_gpu)
        sg_uninit_buffer(sg->indexBuf);
    sg_dealloc_buffer(sg->indexBuf);

    for (i = 0; i < sg->ntextures; i++) {
        if (sg->textures[i].img.id != 0 && (sg->textures[i].flags & NVG_IMAGE_NODELETE) == 0)
            sg_destroy_image(sg->textures[i].img);
    }
    free(sg->textures);

    free(sg->paths);
    free(sg->verts);
    free(sg->indexes);
    free(sg->uniforms);
    free(sg->calls);

    free(sg);
}

NVGcontext* nvgCreateSokol(int flags)
{
    NVGparams params;
    NVGcontext* ctx = NULL;
    SGNVGcontext* sg = (SGNVGcontext*)malloc(sizeof(SGNVGcontext));
    if(sg == NULL) goto error;
    memset(sg, 0, sizeof(SGNVGcontext));

    memset(&params, 0, sizeof(params));
    params.renderCreate = sgnvg__renderCreate;
    params.renderCreateTexture = sgnvg__renderCreateTexture;
    params.renderDeleteTexture = sgnvg__renderDeleteTexture;
    params.renderUpdateTexture = sgnvg__renderUpdateTexture;
    params.renderGetTextureSize = sgnvg__renderGetTextureSize;
    params.renderViewport = sgnvg__renderViewport;
    params.renderCancel = sgnvg__renderCancel;
    params.renderFlush = sgnvg__renderFlush;
    params.renderFill = sgnvg__renderFill;
    params.renderStroke = sgnvg__renderStroke;
    params.renderTriangles = sgnvg__renderTriangles;
    params.renderDelete = sgnvg__renderDelete;
    params.userPtr = sg;
    params.edgeAntiAlias = flags & NVG_ANTIALIAS ? 1 : 0;

    sg->flags = flags;

    ctx = nvgCreateInternal(&params);
    if(ctx == NULL) goto error;

    return ctx;

error:
    if(ctx != NULL) nvgDeleteInternal(ctx);
    return NULL;
}

void nvgDeleteSokol(NVGcontext* ctx)
{
    nvgDeleteInternal(ctx);
}

int nvsgCreateImageFromHandleSokol(NVGcontext* ctx, sg_image imageSokol, sg_sampler samplerSokol, int type, int w, int h, int flags)
{
    SGNVGcontext* sg = (SGNVGcontext*)nvgInternalParams(ctx)->userPtr;
    SGNVGtexture* tex = sgnvg__allocTexture(sg);

    if (tex == NULL) return 0;

    tex->type = type;
    tex->img = imageSokol;
    tex->smp = samplerSokol;
    tex->flags = flags;
    tex->width = w;
    tex->height = h;

    return tex->id;
}

sg_image nvsgImageHandleSokol(NVGcontext* ctx, int image)
{
    SGNVGcontext* sg = (SGNVGcontext*)nvgInternalParams(ctx)->userPtr;
    SGNVGtexture* tex = sgnvg__findTexture(sg, image);
    return tex->img;
}

#endif /* NANOVG_SOKOL_IMPLEMENTATION */
