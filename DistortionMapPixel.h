#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer cbParams
// {
//
//   float distortscale;                // Offset:    0 Size:     4 [unused]
//   float k0;                          // Offset:    4 Size:     4
//   float k1;                          // Offset:    8 Size:     4
//   float k2;                          // Offset:   12 Size:     4
//   float k3;                          // Offset:   16 Size:     4
//   float k4;                          // Offset:   20 Size:     4
//   float k5;                          // Offset:   24 Size:     4
//   float k6;                          // Offset:   28 Size:     4
//   float rk0;                         // Offset:   32 Size:     4
//   float rk1;                         // Offset:   36 Size:     4
//   float rk2;                         // Offset:   40 Size:     4
//   float rk3;                         // Offset:   44 Size:     4
//   float rk4;                         // Offset:   48 Size:     4
//   float rk5;                         // Offset:   52 Size:     4
//   float rk6;                         // Offset:   56 Size:     4
//   float gk0;                         // Offset:   60 Size:     4
//   float gk1;                         // Offset:   64 Size:     4
//   float gk2;                         // Offset:   68 Size:     4
//   float gk3;                         // Offset:   72 Size:     4
//   float gk4;                         // Offset:   76 Size:     4
//   float gk5;                         // Offset:   80 Size:     4
//   float gk6;                         // Offset:   84 Size:     4
//   float bk0;                         // Offset:   88 Size:     4
//   float bk1;                         // Offset:   92 Size:     4
//   float bk2;                         // Offset:   96 Size:     4
//   float bk3;                         // Offset:  100 Size:     4
//   float bk4;                         // Offset:  104 Size:     4
//   float bk5;                         // Offset:  108 Size:     4
//   float bk6;                         // Offset:  112 Size:     4
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// cbParams                          cbuffer      NA          NA            cb1      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float       
// TEXCOORD                 0   xy          1     NONE   float   xy  
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
dcl_constantbuffer CB1[8], immediateIndexed
dcl_input_ps linear v1.xy
dcl_output o0.xyzw
dcl_temps 3
mad r0.xy, v1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
dp2 r0.x, r0.xyxx, r0.xyxx
sqrt r0.y, r0.x
mul r0.z, r0.y, r0.x
mul r0.w, r0.y, r0.z
mul r1.x, r0.y, r0.w
mul r1.y, r0.y, r1.x
mad r1.z, cb1[0].z, r0.y, cb1[0].y
mad r1.z, cb1[0].w, r0.x, r1.z
mad r1.z, cb1[1].x, r0.z, r1.z
mad r1.z, cb1[1].y, r0.w, r1.z
mad r1.z, cb1[1].z, r1.x, r1.z
mad r1.z, cb1[1].w, r1.y, r1.z
add r2.xy, v1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
mad r2.xy, r2.xyxx, r1.zzzz, l(0.500000, 0.500000, 0.000000, 0.000000)
ge r2.zw, l(0.000000, 0.000000, 0.000000, 0.000000), r2.xxxy
or r1.w, r2.w, r2.z
ge r2.xy, r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
or r1.w, r1.w, r2.x
or r1.w, r2.y, r1.w
if_nz r1.w
  mov o0.xyzw, l(0,0,0,0)
  ret 
endif 
mad r1.w, cb1[2].y, r0.y, cb1[2].x
mad r1.w, cb1[2].z, r0.x, r1.w
mad r1.w, cb1[2].w, r0.z, r1.w
mad r1.w, cb1[3].x, r0.w, r1.w
mad r1.w, cb1[3].y, r1.x, r1.w
mad r2.x, cb1[3].z, r1.y, r1.w
mad r1.w, cb1[4].x, r0.y, cb1[3].w
mad r1.w, cb1[4].y, r0.x, r1.w
mad r1.w, cb1[4].z, r0.z, r1.w
mad r1.w, cb1[4].w, r0.w, r1.w
mad r1.w, cb1[5].x, r1.x, r1.w
mad r2.y, cb1[5].y, r1.y, r1.w
mad r0.y, cb1[5].w, r0.y, cb1[5].z
mad r0.x, cb1[6].x, r0.x, r0.y
mad r0.x, cb1[6].y, r0.z, r0.x
mad r0.x, cb1[6].z, r0.w, r0.x
mad r0.x, cb1[6].w, r1.x, r0.x
mad r2.z, cb1[7].x, r1.y, r0.x
mov r2.w, l(1.000000)
mul o0.xyzw, r1.zzzz, r2.xyzw
ret 
// Approximately 45 instruction slots used
#endif

const BYTE g_PS_DistortionMap[] =
{
     68,  88,  66,  67, 193, 197, 
    250,  82,  24, 230,  96,  32, 
    111,  40, 222,   1, 171, 117, 
     36,  97,   1,   0,   0,   0, 
    140,  13,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
     16,   6,   0,   0, 104,   6, 
      0,   0, 156,   6,   0,   0, 
    240,  12,   0,   0,  82,  68, 
     69,  70, 212,   5,   0,   0, 
      1,   0,   0,   0, 104,   0, 
      0,   0,   1,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
    255, 255,   0,   1,   0,   0, 
    169,   5,   0,   0,  82,  68, 
     49,  49,  60,   0,   0,   0, 
     24,   0,   0,   0,  32,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
     92,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,  99,  98,  80,  97, 
    114,  97, 109, 115,   0, 171, 
    171, 171,  92,   0,   0,   0, 
     29,   0,   0,   0, 128,   0, 
      0,   0, 128,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   8,   5,   0,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     28,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     64,   5,   0,   0,   4,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,  28,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  67,   5, 
      0,   0,   8,   0,   0,   0, 
      4,   0,   0,   0,   2,   0, 
      0,   0,  28,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,  70,   5,   0,   0, 
     12,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     73,   5,   0,   0,  16,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,  28,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  76,   5, 
      0,   0,  20,   0,   0,   0, 
      4,   0,   0,   0,   2,   0, 
      0,   0,  28,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,  79,   5,   0,   0, 
     24,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     82,   5,   0,   0,  28,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,  28,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  85,   5, 
      0,   0,  32,   0,   0,   0, 
      4,   0,   0,   0,   2,   0, 
      0,   0,  28,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,  89,   5,   0,   0, 
     36,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     93,   5,   0,   0,  40,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,  28,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  97,   5, 
      0,   0,  44,   0,   0,   0, 
      4,   0,   0,   0,   2,   0, 
      0,   0,  28,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 101,   5,   0,   0, 
     48,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    105,   5,   0,   0,  52,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,  28,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 109,   5, 
      0,   0,  56,   0,   0,   0, 
      4,   0,   0,   0,   2,   0, 
      0,   0,  28,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 113,   5,   0,   0, 
     60,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    117,   5,   0,   0,  64,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,  28,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 121,   5, 
      0,   0,  68,   0,   0,   0, 
      4,   0,   0,   0,   2,   0, 
      0,   0,  28,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 125,   5,   0,   0, 
     72,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    129,   5,   0,   0,  76,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,  28,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 133,   5, 
      0,   0,  80,   0,   0,   0, 
      4,   0,   0,   0,   2,   0, 
      0,   0,  28,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 137,   5,   0,   0, 
     84,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    141,   5,   0,   0,  88,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,  28,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 145,   5, 
      0,   0,  92,   0,   0,   0, 
      4,   0,   0,   0,   2,   0, 
      0,   0,  28,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 149,   5,   0,   0, 
     96,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    153,   5,   0,   0, 100,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,  28,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 157,   5, 
      0,   0, 104,   0,   0,   0, 
      4,   0,   0,   0,   2,   0, 
      0,   0,  28,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 161,   5,   0,   0, 
    108,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    165,   5,   0,   0, 112,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,  28,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 100, 105, 
    115, 116, 111, 114, 116, 115, 
     99,  97, 108, 101,   0, 102, 
    108, 111,  97, 116,   0, 171, 
      0,   0,   3,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  21,   5,   0,   0, 
    107,  48,   0, 107,  49,   0, 
    107,  50,   0, 107,  51,   0, 
    107,  52,   0, 107,  53,   0, 
    107,  54,   0, 114, 107,  48, 
      0, 114, 107,  49,   0, 114, 
    107,  50,   0, 114, 107,  51, 
      0, 114, 107,  52,   0, 114, 
    107,  53,   0, 114, 107,  54, 
      0, 103, 107,  48,   0, 103, 
    107,  49,   0, 103, 107,  50, 
      0, 103, 107,  51,   0, 103, 
    107,  52,   0, 103, 107,  53, 
      0, 103, 107,  54,   0,  98, 
    107,  48,   0,  98, 107,  49, 
      0,  98, 107,  50,   0,  98, 
    107,  51,   0,  98, 107,  52, 
      0,  98, 107,  53,   0,  98, 
    107,  54,   0,  77, 105,  99, 
    114, 111, 115, 111, 102, 116, 
     32,  40,  82,  41,  32,  72, 
     76,  83,  76,  32,  83, 104, 
     97, 100, 101, 114,  32,  67, 
    111, 109, 112, 105, 108, 101, 
    114,  32,  49,  48,  46,  49, 
      0, 171, 171, 171,  73,  83, 
     71,  78,  80,   0,   0,   0, 
      2,   0,   0,   0,   8,   0, 
      0,   0,  56,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0,  68,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,   3,   3, 
      0,   0,  83,  86,  95,  80, 
     79,  83,  73,  84,  73,  79, 
     78,   0,  84,  69,  88,  67, 
     79,  79,  82,  68,   0, 171, 
    171, 171,  79,  83,  71,  78, 
     44,   0,   0,   0,   1,   0, 
      0,   0,   8,   0,   0,   0, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   0,   0,   0, 
     83,  86,  95,  84,  97, 114, 
    103, 101, 116,   0, 171, 171, 
     83,  72,  69,  88,  76,   6, 
      0,   0,  80,   0,   0,   0, 
    147,   1,   0,   0, 106,   8, 
      0,   1,  89,   0,   0,   4, 
     70, 142,  32,   0,   1,   0, 
      0,   0,   8,   0,   0,   0, 
     98,  16,   0,   3,  50,  16, 
     16,   0,   1,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   3,   0, 
      0,   0,  50,   0,   0,  15, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,  16,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,  64, 
      0,   0,   0,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128, 191,   0,   0, 128, 191, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  15,   0,   0,   7, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
     75,   0,   0,   5,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   7, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     56,   0,   0,   7, 130,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,  56,   0, 
      0,   7,  18,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     58,   0,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   7, 
     34,   0,  16,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  11,  66,   0, 
     16,   0,   1,   0,   0,   0, 
     42, 128,  32,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  26, 128,  32,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,  50,   0,   0,  10, 
     66,   0,  16,   0,   1,   0, 
      0,   0,  58, 128,  32,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  42,   0, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  10,  66,   0, 
     16,   0,   1,   0,   0,   0, 
     10, 128,  32,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10,  66,   0,  16,   0, 
      1,   0,   0,   0,  26, 128, 
     32,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
     66,   0,  16,   0,   1,   0, 
      0,   0,  42, 128,  32,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  10,  66,   0, 
     16,   0,   1,   0,   0,   0, 
     58, 128,  32,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
     26,   0,  16,   0,   1,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   0,   0, 
      0,  10,  50,   0,  16,   0, 
      2,   0,   0,   0,  70,  16, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0, 191,   0,   0,   0, 191, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  50,   0,   0,  12, 
     50,   0,  16,   0,   2,   0, 
      0,   0,  70,   0,  16,   0, 
      2,   0,   0,   0, 166,  10, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,  63,   0,   0,   0,  63, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  29,   0,   0,  10, 
    194,   0,  16,   0,   2,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   6,   4, 
     16,   0,   2,   0,   0,   0, 
     60,   0,   0,   7, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   2,   0, 
      0,   0,  42,   0,  16,   0, 
      2,   0,   0,   0,  29,   0, 
      0,  10,  50,   0,  16,   0, 
      2,   0,   0,   0,  70,   0, 
     16,   0,   2,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  60,   0,   0,   7, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  10,   0, 
     16,   0,   2,   0,   0,   0, 
     60,   0,   0,   7, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     26,   0,  16,   0,   2,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  31,   0, 
      4,   3,  58,   0,  16,   0, 
      1,   0,   0,   0,  54,   0, 
      0,   8, 242,  32,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     62,   0,   0,   1,  21,   0, 
      0,   1,  50,   0,   0,  11, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  26, 128,  32,   0, 
      1,   0,   0,   0,   2,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  10, 128, 
     32,   0,   1,   0,   0,   0, 
      2,   0,   0,   0,  50,   0, 
      0,  10, 130,   0,  16,   0, 
      1,   0,   0,   0,  42, 128, 
     32,   0,   1,   0,   0,   0, 
      2,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  58, 128,  32,   0, 
      1,   0,   0,   0,   2,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  10, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     10, 128,  32,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
     58,   0,  16,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10, 130,   0,  16,   0, 
      1,   0,   0,   0,  26, 128, 
     32,   0,   1,   0,   0,   0, 
      3,   0,   0,   0,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  42, 128,  32,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  11, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     10, 128,  32,   0,   1,   0, 
      0,   0,   4,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  58, 128,  32,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,  50,   0,   0,  10, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  26, 128,  32,   0, 
      1,   0,   0,   0,   4,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  10, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     42, 128,  32,   0,   1,   0, 
      0,   0,   4,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10, 130,   0,  16,   0, 
      1,   0,   0,   0,  58, 128, 
     32,   0,   1,   0,   0,   0, 
      4,   0,   0,   0,  58,   0, 
     16,   0,   0,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  10, 128,  32,   0, 
      1,   0,   0,   0,   5,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  10,  34,   0, 
     16,   0,   2,   0,   0,   0, 
     26, 128,  32,   0,   1,   0, 
      0,   0,   5,   0,   0,   0, 
     26,   0,  16,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  11,  34,   0,  16,   0, 
      0,   0,   0,   0,  58, 128, 
     32,   0,   1,   0,   0,   0, 
      5,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     42, 128,  32,   0,   1,   0, 
      0,   0,   5,   0,   0,   0, 
     50,   0,   0,  10,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10, 128,  32,   0,   1,   0, 
      0,   0,   6,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  50,   0, 
      0,  10,  18,   0,  16,   0, 
      0,   0,   0,   0,  26, 128, 
     32,   0,   1,   0,   0,   0, 
      6,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  50,   0,   0,  10, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  42, 128,  32,   0, 
      1,   0,   0,   0,   6,   0, 
      0,   0,  58,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     50,   0,   0,  10,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     58, 128,  32,   0,   1,   0, 
      0,   0,   6,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  50,   0, 
      0,  10,  66,   0,  16,   0, 
      2,   0,   0,   0,  10, 128, 
     32,   0,   1,   0,   0,   0, 
      7,   0,   0,   0,  26,   0, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
    130,   0,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0, 128,  63,  56,   0, 
      0,   7, 242,  32,  16,   0, 
      0,   0,   0,   0, 166,  10, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   2,   0, 
      0,   0,  62,   0,   0,   1, 
     83,  84,  65,  84, 148,   0, 
      0,   0,  45,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
     36,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      2,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
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
      0,   0,   0,   0,   0,   0
};
