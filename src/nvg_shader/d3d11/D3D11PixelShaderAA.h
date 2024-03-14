#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer frag
// {
//
//   row_major float4x4 _59_scissorMat; // Offset:    0 Size:    64
//   float4 _59_scissorExt;             // Offset:   64 Size:    16
//   float4 _59_scissorScale;           // Offset:   80 Size:    16
//   row_major float4x4 _59_paintMat;   // Offset:   96 Size:    64
//   float4 _59_extent;                 // Offset:  160 Size:    16
//   float4 _59_radius;                 // Offset:  176 Size:    16
//   float4 _59_feather;                // Offset:  192 Size:    16
//   float4 _59_innerCol;               // Offset:  208 Size:    16
//   float4 _59_outerCol;               // Offset:  224 Size:    16
//   float4 _59_strokeMult;             // Offset:  240 Size:    16
//   int _59_texType;                   // Offset:  256 Size:     4
//   int _59_type;                      // Offset:  260 Size:     4
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// smp                               sampler      NA          NA             s0      1 
// tex                               texture  float4          2d             t0      1 
// frag                              cbuffer      NA          NA            cb0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// TEXCOORD                 0   xy          0     NONE   float   xy  
// TEXCOORD                 1     zw        0     NONE   float     zw
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Target                0   xyzw        0   TARGET   float   xyzw
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[17], immediateIndexed
dcl_sampler s0, mode_default
dcl_resource_texture2d (float,float,float,float) t0
dcl_input_ps linear v0.xy
dcl_input_ps linear v0.zw
dcl_output o0.xyzw
dcl_temps 4
mad r0.x, v0.x, l(2.000000), l(-1.000000)
add r0.x, -|r0.x|, l(1.000000)
mul r0.x, r0.x, cb0[15].x
min r0.x, r0.x, l(1.000000)
min r0.y, v0.y, l(1.000000)
mul r0.x, r0.y, r0.x
lt r0.y, r0.x, cb0[15].y
discard_nz r0.y
mul r0.yz, v0.wwww, cb0[1].xxyx
mad r0.yz, v0.zzzz, cb0[0].xxyx, r0.yyzy
add r0.yz, r0.yyzy, cb0[2].xxyx
add r0.yz, |r0.yyzy|, -cb0[4].xxyx
mad_sat r0.yz, -r0.yyzy, cb0[5].xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
mul r0.y, r0.z, r0.y
ne r0.z, r0.y, l(0.000000)
if_nz r0.z
  if_z cb0[16].y
    mul r0.zw, v0.wwww, cb0[7].xxxy
    mad r0.zw, v0.zzzz, cb0[6].xxxy, r0.zzzw
    add r0.zw, r0.zzzw, cb0[8].xxxy
    add r1.xy, cb0[10].xyxx, -cb0[11].xxxx
    add r0.zw, |r0.zzzw|, -r1.xxxy
    max r1.x, r0.w, r0.z
    min r1.x, r1.x, l(0.000000)
    max r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    dp2 r0.z, r0.zwzz, r0.zwzz
    sqrt r0.z, r0.z
    add r0.z, r0.z, r1.x
    add r0.z, r0.z, -cb0[11].x
    mad r0.z, cb0[12].x, l(0.500000), r0.z
    div_sat r0.z, r0.z, cb0[12].x
    add r1.xyzw, -cb0[13].xyzw, cb0[14].xyzw
    mad r1.xyzw, r0.zzzz, r1.xyzw, cb0[13].xyzw
    mul r0.z, r0.y, r0.x
    mul o0.xyzw, r0.zzzz, r1.xyzw
  else 
    ieq r0.z, l(1), cb0[16].y
    if_nz r0.z
      mul r0.zw, v0.wwww, cb0[7].xxxy
      mad r0.zw, v0.zzzz, cb0[6].xxxy, r0.zzzw
      add r0.zw, r0.zzzw, cb0[8].xxxy
      div r0.zw, r0.zzzw, cb0[10].xxxy
      sample_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.zwzz, t0.xyzw, s0
      ieq r2.xyz, l(1, 2, 3, 0), cb0[16].xxxx
      mul r3.xyz, r1.wwww, r1.xyzx
      movc r1.xyz, r2.xxxx, r3.xyzx, r1.xyzx
      movc r1.yzw, r2.yyyy, r1.xxxx, r1.yyzw
      eq r0.z, r1.w, l(1.000000)
      and r0.z, r0.z, r2.z
      discard_nz r0.z
      mul r1.xyzw, r1.xyzw, cb0[13].xyzw
      mul r0.x, r0.y, r0.x
      mul o0.xyzw, r0.xxxx, r1.xyzw
    else 
      ieq r0.x, l(2), cb0[16].y
      if_nz r0.x
        mov o0.xyzw, l(1.000000,1.000000,1.000000,1.000000)
      else 
        ieq r0.x, l(3), cb0[16].y
        if_nz r0.x
          sample_indexable(texture2d)(float,float,float,float) r1.xyzw, v0.xyxx, t0.xyzw, s0
          ieq r0.xz, l(1, 0, 2, 0), cb0[16].xxxx
          mul r2.xyz, r1.wwww, r1.xyzx
          movc r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
          movc r1.yzw, r0.zzzz, r1.xxxx, r1.yyzw
          mul r0.xyzw, r0.yyyy, r1.xyzw
          mul o0.xyzw, r0.xyzw, cb0[13].xyzw
        endif 
      endif 
    endif 
  endif 
else 
  mov o0.xyzw, l(0,0,0,0)
endif 
ret 
// Approximately 75 instruction slots used
#endif

const BYTE g_D3D11PixelShaderAA_Main[] =
{
     68,  88,  66,  67,  24, 149, 
    121, 221,  18,  67, 159, 135, 
    152, 147, 141, 251, 116, 107, 
     71,  51,   1,   0,   0,   0, 
    204,  13,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
     44,   4,   0,   0, 120,   4, 
      0,   0, 172,   4,   0,   0, 
     48,  13,   0,   0,  82,  68, 
     69,  70, 240,   3,   0,   0, 
      1,   0,   0,   0, 172,   0, 
      0,   0,   3,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
    255, 255,   0,   1,   0,   0, 
    197,   3,   0,   0,  82,  68, 
     49,  49,  60,   0,   0,   0, 
     24,   0,   0,   0,  32,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
    156,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 160,   0,   0,   0, 
      2,   0,   0,   0,   5,   0, 
      0,   0,   4,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     13,   0,   0,   0, 164,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
    115, 109, 112,   0, 116, 101, 
    120,   0, 102, 114,  97, 103, 
      0, 171, 171, 171, 164,   0, 
      0,   0,  12,   0,   0,   0, 
    196,   0,   0,   0,  16,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 164,   2, 
      0,   0,   0,   0,   0,   0, 
     64,   0,   0,   0,   2,   0, 
      0,   0, 188,   2,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 224,   2,   0,   0, 
     64,   0,   0,   0,  16,   0, 
      0,   0,   2,   0,   0,   0, 
    248,   2,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     28,   3,   0,   0,  80,   0, 
      0,   0,  16,   0,   0,   0, 
      2,   0,   0,   0, 248,   2, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  45,   3, 
      0,   0,  96,   0,   0,   0, 
     64,   0,   0,   0,   2,   0, 
      0,   0, 188,   2,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,  58,   3,   0,   0, 
    160,   0,   0,   0,  16,   0, 
      0,   0,   2,   0,   0,   0, 
    248,   2,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     69,   3,   0,   0, 176,   0, 
      0,   0,  16,   0,   0,   0, 
      2,   0,   0,   0, 248,   2, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  80,   3, 
      0,   0, 192,   0,   0,   0, 
     16,   0,   0,   0,   2,   0, 
      0,   0, 248,   2,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,  92,   3,   0,   0, 
    208,   0,   0,   0,  16,   0, 
      0,   0,   2,   0,   0,   0, 
    248,   2,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    105,   3,   0,   0, 224,   0, 
      0,   0,  16,   0,   0,   0, 
      2,   0,   0,   0, 248,   2, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 118,   3, 
      0,   0, 240,   0,   0,   0, 
     16,   0,   0,   0,   2,   0, 
      0,   0, 248,   2,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 133,   3,   0,   0, 
      0,   1,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
    152,   3,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    188,   3,   0,   0,   4,   1, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0, 152,   3, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  95,  53, 
     57,  95, 115,  99, 105, 115, 
    115, 111, 114,  77,  97, 116, 
      0, 102, 108, 111,  97, 116, 
     52, 120,  52,   0,   2,   0, 
      3,   0,   4,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    179,   2,   0,   0,  95,  53, 
     57,  95, 115,  99, 105, 115, 
    115, 111, 114,  69, 120, 116, 
      0, 102, 108, 111,  97, 116, 
     52,   0, 171, 171,   1,   0, 
      3,   0,   1,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    239,   2,   0,   0,  95,  53, 
     57,  95, 115,  99, 105, 115, 
    115, 111, 114,  83,  99,  97, 
    108, 101,   0,  95,  53,  57, 
     95, 112,  97, 105, 110, 116, 
     77,  97, 116,   0,  95,  53, 
     57,  95, 101, 120, 116, 101, 
    110, 116,   0,  95,  53,  57, 
     95, 114,  97, 100, 105, 117, 
    115,   0,  95,  53,  57,  95, 
    102, 101,  97, 116, 104, 101, 
    114,   0,  95,  53,  57,  95, 
    105, 110, 110, 101, 114,  67, 
    111, 108,   0,  95,  53,  57, 
     95, 111, 117, 116, 101, 114, 
     67, 111, 108,   0,  95,  53, 
     57,  95, 115, 116, 114, 111, 
    107, 101,  77, 117, 108, 116, 
      0,  95,  53,  57,  95, 116, 
    101, 120,  84, 121, 112, 101, 
      0, 105, 110, 116,   0, 171, 
    171, 171,   0,   0,   2,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 145,   3, 
      0,   0,  95,  53,  57,  95, 
    116, 121, 112, 101,   0,  77, 
    105,  99, 114, 111, 115, 111, 
    102, 116,  32,  40,  82,  41, 
     32,  72,  76,  83,  76,  32, 
     83, 104,  97, 100, 101, 114, 
     32,  67, 111, 109, 112, 105, 
    108, 101, 114,  32,  49,  48, 
     46,  49,   0, 171, 171, 171, 
     73,  83,  71,  78,  68,   0, 
      0,   0,   2,   0,   0,   0, 
      8,   0,   0,   0,  56,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   3,   0,   0,  56,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     12,  12,   0,   0,  84,  69, 
     88,  67,  79,  79,  82,  68, 
      0, 171, 171, 171,  79,  83, 
     71,  78,  44,   0,   0,   0, 
      1,   0,   0,   0,   8,   0, 
      0,   0,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0,  83,  86,  95,  84, 
     97, 114, 103, 101, 116,   0, 
    171, 171,  83,  72,  69,  88, 
    124,   8,   0,   0,  80,   0, 
      0,   0,  31,   2,   0,   0, 
    106,   8,   0,   1,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      0,   0,   0,   0,  17,   0, 
      0,   0,  90,   0,   0,   3, 
      0,  96,  16,   0,   0,   0, 
      0,   0,  88,  24,   0,   4, 
      0, 112,  16,   0,   0,   0, 
      0,   0,  85,  85,   0,   0, 
     98,  16,   0,   3,  50,  16, 
     16,   0,   0,   0,   0,   0, 
     98,  16,   0,   3, 194,  16, 
     16,   0,   0,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   4,   0, 
      0,   0,  50,   0,   0,   9, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,  16,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,  64, 
      1,  64,   0,   0,   0,   0, 
    128, 191,   0,   0,   0,   8, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16, 128, 
    193,   0,   0,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0, 128,  63,  56,   0, 
      0,   8,  18,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     10, 128,  32,   0,   0,   0, 
      0,   0,  15,   0,   0,   0, 
     51,   0,   0,   7,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0, 128,  63,  51,   0, 
      0,   7,  34,   0,  16,   0, 
      0,   0,   0,   0,  26,  16, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
    128,  63,  56,   0,   0,   7, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     49,   0,   0,   8,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  26, 128,  32,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0,  13,   0,   4,   3, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   8, 
     98,   0,  16,   0,   0,   0, 
      0,   0, 246,  31,  16,   0, 
      0,   0,   0,   0,   6, 129, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10,  98,   0,  16,   0, 
      0,   0,   0,   0, 166,  26, 
     16,   0,   0,   0,   0,   0, 
      6, 129,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     86,   6,  16,   0,   0,   0, 
      0,   0,   0,   0,   0,   8, 
     98,   0,  16,   0,   0,   0, 
      0,   0,  86,   6,  16,   0, 
      0,   0,   0,   0,   6, 129, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,  10,  98,   0,  16,   0, 
      0,   0,   0,   0,  86,   6, 
     16, 128, 129,   0,   0,   0, 
      0,   0,   0,   0,   6, 129, 
     32, 128,  65,   0,   0,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,  50,  32,   0,  14, 
     98,   0,  16,   0,   0,   0, 
      0,   0,  86,   6,  16, 128, 
     65,   0,   0,   0,   0,   0, 
      0,   0,   6, 129,  32,   0, 
      0,   0,   0,   0,   5,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,  63,   0,   0,   0,  63, 
      0,   0,   0,   0,  56,   0, 
      0,   7,  34,   0,  16,   0, 
      0,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  57,   0,   0,   7, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     31,   0,   4,   3,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     31,   0,   0,   4,  26, 128, 
     32,   0,   0,   0,   0,   0, 
     16,   0,   0,   0,  56,   0, 
      0,   8, 194,   0,  16,   0, 
      0,   0,   0,   0, 246,  31, 
     16,   0,   0,   0,   0,   0, 
      6, 132,  32,   0,   0,   0, 
      0,   0,   7,   0,   0,   0, 
     50,   0,   0,  10, 194,   0, 
     16,   0,   0,   0,   0,   0, 
    166,  26,  16,   0,   0,   0, 
      0,   0,   6, 132,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0, 166,  14,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   8, 194,   0,  16,   0, 
      0,   0,   0,   0, 166,  14, 
     16,   0,   0,   0,   0,   0, 
      6, 132,  32,   0,   0,   0, 
      0,   0,   8,   0,   0,   0, 
      0,   0,   0,  10,  50,   0, 
     16,   0,   1,   0,   0,   0, 
     70, 128,  32,   0,   0,   0, 
      0,   0,  10,   0,   0,   0, 
      6, 128,  32, 128,  65,   0, 
      0,   0,   0,   0,   0,   0, 
     11,   0,   0,   0,   0,   0, 
      0,   9, 194,   0,  16,   0, 
      0,   0,   0,   0, 166,  14, 
     16, 128, 129,   0,   0,   0, 
      0,   0,   0,   0,   6,   4, 
     16, 128,  65,   0,   0,   0, 
      1,   0,   0,   0,  52,   0, 
      0,   7,  18,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  51,   0,   0,   7, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     52,   0,   0,  10, 194,   0, 
     16,   0,   0,   0,   0,   0, 
    166,  14,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   7,  66,   0,  16,   0, 
      0,   0,   0,   0, 230,  10, 
     16,   0,   0,   0,   0,   0, 
    230,  10,  16,   0,   0,   0, 
      0,   0,  75,   0,   0,   5, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   7,  66,   0,  16,   0, 
      0,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,   0,   0,   0,   9, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,  10, 128, 
     32, 128,  65,   0,   0,   0, 
      0,   0,   0,   0,  11,   0, 
      0,   0,  50,   0,   0,  10, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  10, 128,  32,   0, 
      0,   0,   0,   0,  12,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,  63,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     14,  32,   0,   8,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  10, 128,  32,   0, 
      0,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,  10, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70, 142,  32, 128, 
     65,   0,   0,   0,   0,   0, 
      0,   0,  13,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,  14,   0,   0,   0, 
     50,   0,   0,  10, 242,   0, 
     16,   0,   1,   0,   0,   0, 
    166,  10,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     13,   0,   0,   0,  56,   0, 
      0,   7,  66,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   7, 
    242,  32,  16,   0,   0,   0, 
      0,   0, 166,  10,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     18,   0,   0,   1,  32,   0, 
      0,   8,  66,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     26, 128,  32,   0,   0,   0, 
      0,   0,  16,   0,   0,   0, 
     31,   0,   4,   3,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     56,   0,   0,   8, 194,   0, 
     16,   0,   0,   0,   0,   0, 
    246,  31,  16,   0,   0,   0, 
      0,   0,   6, 132,  32,   0, 
      0,   0,   0,   0,   7,   0, 
      0,   0,  50,   0,   0,  10, 
    194,   0,  16,   0,   0,   0, 
      0,   0, 166,  26,  16,   0, 
      0,   0,   0,   0,   6, 132, 
     32,   0,   0,   0,   0,   0, 
      6,   0,   0,   0, 166,  14, 
     16,   0,   0,   0,   0,   0, 
      0,   0,   0,   8, 194,   0, 
     16,   0,   0,   0,   0,   0, 
    166,  14,  16,   0,   0,   0, 
      0,   0,   6, 132,  32,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,  14,   0,   0,   8, 
    194,   0,  16,   0,   0,   0, 
      0,   0, 166,  14,  16,   0, 
      0,   0,   0,   0,   6, 132, 
     32,   0,   0,   0,   0,   0, 
     10,   0,   0,   0,  69,   0, 
      0, 139, 194,   0,   0, 128, 
     67,  85,  21,   0, 242,   0, 
     16,   0,   1,   0,   0,   0, 
    230,  10,  16,   0,   0,   0, 
      0,   0,  70, 126,  16,   0, 
      0,   0,   0,   0,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     32,   0,   0,  11, 114,   0, 
     16,   0,   2,   0,   0,   0, 
      2,  64,   0,   0,   1,   0, 
      0,   0,   2,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   6, 128,  32,   0, 
      0,   0,   0,   0,  16,   0, 
      0,   0,  56,   0,   0,   7, 
    114,   0,  16,   0,   3,   0, 
      0,   0, 246,  15,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     55,   0,   0,   9, 114,   0, 
     16,   0,   1,   0,   0,   0, 
      6,   0,  16,   0,   2,   0, 
      0,   0,  70,   2,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     55,   0,   0,   9, 226,   0, 
     16,   0,   1,   0,   0,   0, 
     86,   5,  16,   0,   2,   0, 
      0,   0,   6,   0,  16,   0, 
      1,   0,   0,   0,  86,  14, 
     16,   0,   1,   0,   0,   0, 
     24,   0,   0,   7,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0, 128,  63,   1,   0, 
      0,   7,  66,   0,  16,   0, 
      0,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   2,   0, 
      0,   0,  13,   0,   4,   3, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   8, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     13,   0,   0,   0,  56,   0, 
      0,   7,  18,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   7, 
    242,  32,  16,   0,   0,   0, 
      0,   0,   6,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     18,   0,   0,   1,  32,   0, 
      0,   8,  18,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
     26, 128,  32,   0,   0,   0, 
      0,   0,  16,   0,   0,   0, 
     31,   0,   4,   3,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   8, 242,  32, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0, 128,  63,   0,   0, 
    128,  63,  18,   0,   0,   1, 
     32,   0,   0,   8,  18,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   3,   0, 
      0,   0,  26, 128,  32,   0, 
      0,   0,   0,   0,  16,   0, 
      0,   0,  31,   0,   4,   3, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  69,   0,   0, 139, 
    194,   0,   0, 128,  67,  85, 
     21,   0, 242,   0,  16,   0, 
      1,   0,   0,   0,  70,  16, 
     16,   0,   0,   0,   0,   0, 
     70, 126,  16,   0,   0,   0, 
      0,   0,   0,  96,  16,   0, 
      0,   0,   0,   0,  32,   0, 
      0,  11,  82,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      6, 128,  32,   0,   0,   0, 
      0,   0,  16,   0,   0,   0, 
     56,   0,   0,   7, 114,   0, 
     16,   0,   2,   0,   0,   0, 
    246,  15,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,  55,   0, 
      0,   9, 114,   0,  16,   0, 
      1,   0,   0,   0,   6,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   2,  16,   0,   2,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,  55,   0, 
      0,   9, 226,   0,  16,   0, 
      1,   0,   0,   0, 166,  10, 
     16,   0,   0,   0,   0,   0, 
      6,   0,  16,   0,   1,   0, 
      0,   0,  86,  14,  16,   0, 
      1,   0,   0,   0,  56,   0, 
      0,   7, 242,   0,  16,   0, 
      0,   0,   0,   0,  86,   5, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  56,   0,   0,   8, 
    242,  32,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     13,   0,   0,   0,  21,   0, 
      0,   1,  21,   0,   0,   1, 
     21,   0,   0,   1,  21,   0, 
      0,   1,  18,   0,   0,   1, 
     54,   0,   0,   8, 242,  32, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  21,   0,   0,   1, 
     62,   0,   0,   1,  83,  84, 
     65,  84, 148,   0,   0,   0, 
     75,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,  44,   0, 
      0,   0,   5,   0,   0,   0, 
      1,   0,   0,   0,   6,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0
};
