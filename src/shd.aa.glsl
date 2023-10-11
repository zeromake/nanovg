#define EDGE_AA 1

@vs vs_aa
@include internal_shd.vs.glsl
@end

@fs fs_aa
@include internal_shd.fs.glsl
@end

@program sg_aa vs_aa fs_aa
