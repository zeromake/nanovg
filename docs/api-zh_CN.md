# Nanovg 函数使用
> 本文由 [简悦 SimpRead](http://ksria.com/simpread/) 转码， 原文地址 [nanovg 函数使用](https://blog.csdn.net/hz__dream/article/details/41648205)

## 1.Color 常用工具：

| 序号 | 说明 | 方法 |
|----|----|---|
|1|返回指定整数 RGB 的颜色，RGB 范围 0-255.|`NVGcolor nvgRGB(unsigned char r, unsigned char g, unsigned char b);`|
|2|返回指定小数 RGB 的颜色，RGB 范围 0-1.0|`NVGcolor nvgRGBf(float r, float g, float b);`|
|3|返回指定整数 RGB 和透明度的颜色|`NVGcolor nvgRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);`|
|4|返回指定浮点数 RGB 和透明度的颜色|`NVGcolor nvgRGBAf(float r,float g, float b,float a);`|
|5|返回颜色 c0 和颜色 c1 之间的中间色，0<=u<=1|`NVGcolor nvgLerpRGBA(NVGcolor c0, NVGcolor c1, float u);`|
|6|返回颜色为 c0 并用 a 指定透明度。0<=a<=255|`NVGcolor nvgTransRGBA(NVGcolor c0, unsigned char a);`|
|7|返回颜色为 c0 并用 a 指定透明度。0<=a<=1.0|`NVGcolor nvgTransRGBAf(NVGcolor c0, float a);`|
|8|返回指定小数 HSL 的颜色，HSL 范围 0-1.0|`NVGcolor nvgHSL(float h, float s, float l);`|
|9|返回指定浮点数 HSL 和透明度的颜色, 0<=a<=255|`NVGcolor nvgHSLA(float h, float s, float l, unsigned char a);`|

## 2.渲染模式

| 序号 | 说明 | 方法 |
|----|----|---|
|1|指定笔刷模式为具体的颜色。|`void nvgStrokeColor(NVGcontext* ctx, NVGcolor color);`|
|2|指定笔刷模式为 paint，paint 可以是渐变也可以是其他模式。|`void nvgStrokePaint(NVGcontext* ctx, NVGpaint paint);`|
|3|指定填充为具体的颜色。|`void nvgFillColor(NVGcontext* ctx, NVGcolor color);`|
|4|指定填充模式为 paint，paint 可以是渐变也可以是其他模式。|`void nvgFillPaint(NVGcontext* ctx, NVGpaint paint);`|
|5|指定笔刷在拐角处是否为斜面。|`void nvgMiterLimit(NVGcontext* ctx,float limit);`|
|6|指定笔刷的宽度。|`void nvgStrokeWidth(NVGcontext* ctx,float size);`|
|7|设置笔画的末端的类型(NVG_BUTT \| NVG_ROUND \| NVG_SQUARE)|`void nvgLineCap(NVGcontext* ctx,int cap);`|
|8|指定连接部分的类型(NVG_MITER \| NVG_ROUND \| NVG_BEVEL)|`void nvgLineJoin(NVGcontext* ctx,int join);`|
|9|指定全局的 Alpha 值。|`void nvgGlobalAlpha(NVGcontext* ctx,float alpha);`|

## 3.变换

变换有 3 种：缩放（scal）、倾斜（skew）、平移（translate）。在 nanovg 中使用了一个变换矩阵（第二个矩阵只是说明在程序中对应的 a、b、c…… 等参数，第三个矩阵标注内部 `float[6] xform` 的对应下标）：

```
[sx kx tx]       [a  c  e]       [0  2  4]
[ky sy ty]  ==>  [b  d  f]  ==>  [1  3  5]
[0   0  1]       [0  0  1]       [x  x  x]
```

其中 sx、sy 为缩放，kx、ky 为歪斜，tx、ty 为平移。后面的 `[0  0  1]` 不做存储。

| 序号 | 说明 | 方法 |
|----|----|---|
|1|重置转换|`void nvgResetTransform(NVGcontext* ctx);`|
|2|设置变换矩阵的 6 个值|`void nvgTransform(NVGcontext* ctx,float a, float b,float c, float d,float e, float f);`|
|3|平移变换|`void nvgTranslate(NVGcontext* ctx,float x, float y);`|
|4|旋转变换|`void nvgRotate(NVGcontext* ctx,float angle);`|
|5|倾斜 X 坐标|`void nvgSkewX(NVGcontext* ctx,float angle);`|
|6|倾斜 Y 坐标|`void nvgSkewY(NVGcontext* ctx,float angle);`|
|7|缩放变换|`void nvgScale(NVGcontext* ctx,float x, float y);`|
|8|返回变换矩阵|`void nvgCurrentTransform(NVGcontext* ctx,float* xform);`|
|9|指定矩阵为变换矩阵|`void nvgTransformIdentity(float* dst);`|
|10|指定变换矩阵中的 tx，ty|`void nvgTransformTranslate(float* dst,float tx, float ty);`|
|11|指定变换矩阵中的 sx，sy|`void nvgTransformScale(float* dst,float sx, float sy);`|
|12|指定变换角度|`void nvgTransformRotate(float* dst,float a);`|
|13|指定变换矩阵中的 kx|`void nvgTransformSkewX(float* dst,float a);`|
|14|指定变换矩阵中的 ky|`void nvgTransformSkewY(float* dst,float a);`|
|15|变换矩阵和指定矩阵相乘|`void nvgTransformMultiply(float* dst,const float* src);`|
|16|指定矩阵和变换矩阵相乘|`void nvgTransformPremultiply(float* dst,const float* src);`|
|17|变换矩阵求逆|`int nvgTransformInverse(float* dst,const float* src);`|
|18|由给定的转换改变一个点|`void nvgTransformPoint(float* dstx,float* dsty, constfloat* xform, float srcx,float srcy);`|
|19|弧度转化为角度|`float nvgDegToRad(float deg);`|
|20|角度转化为弧度|`float nvgRadToDeg(float rad);`|

## 4.创建图像

| 序号 | 说明 | 方法 |
|----|----|---|
|1|根据图片文件名字创建图像，imageFlages 为（NVG_IMAGE_FLIPY \| NVG_IMAGE_GENERATE_MIPMAPS \| NVG_IMAGE_REPEATX \| NVG_IMAGE_REPEATY \| NVG_IMAGE_PREMULTIPLIED     ）和 nvgImagePattern 配合使用|`int nvgCreateImage(NVGcontext* ctx,const char* filename,int imageFlags);`|
|2|根据内存中指定的数据块创建图像|`int nvgCreateImageMem(NVGcontext* ctx,int imageFlags, unsignedchar* data, int ndata);`|
|3|创建 RGB 图像|`int nvgCreateImageRGBA(NVGcontext* ctx,int w, int h,int imageFlags, constunsigned char* data);`|
|4|使用指定的数据块更新指定的图像|`void nvgUpdateImage(NVGcontext* ctx,int image, constunsigned char* data);`|
|5|指定图像的大小|`void nvgImageSize(NVGcontext* ctx,int image, int* w,int* h);`|
|6|删除图像|`void nvgDeleteImage(NVGcontext* ctx,int image);`|

## 5.Paints（绘画）

| 序号 | 说明 | 方法 |
|----|----|---|
|1|线性渐变|`NVGpaint nvgLinearGradient(NVGcontext* ctx, float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol);`|
|2|盒型渐变|`NVGpaint nvgBoxGradient(NVGcontext* ctx, float x, float y, float w, float h, float r,float f, NVGcolor icol, NVGcolor ocol);`|
|3|角度渐变|`NVGpaint nvgRadialGradient(NVGcontext* ctx, float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol);`|
|4|指定的图像填充（和 nvgFillPaint，nvgStrokePaint 配合使用）|`NVGpaint nvgImagePattern(NVGcontext* ctx, float ox, float oy, float ex, float ey, float angle, int image, float alpha);`|

## 6.Scissoring(裁剪)

| 序号 | 说明 | 方法 |
|----|----|---|
|1|使用指定的矩形进行裁剪|`void nvgScissor(NVGcontext* ctx,float x, float y,float w, float h);`|
|2|在原来裁剪的基础上继续裁剪|`void nvgIntersectScissor(NVGcontext* ctx,float x, float y,float w, float h);`|
|3|裁剪重置|`void nvgResetScissor(NVGcontext* ctx);`|

## 7.Path(路径)

| 序号 | 说明 | 方法 |
|----|----|---|
|1|销毁以前的路径，准备开始新的路径|`void nvgBeginPath(NVGcontext* ctx);`|
|2|开始一个子路径，以（x,y）为起始点，和后面的 LineTo、BezierTo 等配合使用。|`void nvgMoveTo(NVGcontext* ctx,float x, float y);`|
|3|从起始点画线到指定点|`void nvgLineTo(NVGcontext* ctx,float x, float y);`|
|4|从起始点绘制三次贝塞尔曲线到终点（包含两个控制点）|`void nvgBezierTo(NVGcontext* ctx,float c1x, float c1y,float c2x, float c2y,float x, float y);`|
|5|从起始点绘制二次贝塞尔曲线到终点（一个控制点）|`void nvgQuadTo(NVGcontext* ctx,float cx, float cy,float x, float y);`|
|6|以起始点和指定的两个点绘制弧线段|`void nvgArcTo(NVGcontext* ctx,float x1, float y1,float x2, float y2,float radius);`|
|7|使用一个线段闭合当前路径|`void nvgClosePath(NVGcontext* ctx);`|
|8|设置当前子路径的 winding|`void nvgPathWinding(NVGcontext* ctx,int dir);`|
|9|绘制弧线段，中心点为(cx，cy)，半径为 r，弧度从 a0 到 a1。(dir 为：NVG_CW \| NVG_CCW)|`void nvgArc(NVGcontext* ctx,float cx, float cy,float r, float a0,float a1, int dir);`|
|10|绘制矩形|`void nvgRect(NVGcontext* ctx,float x, float y,float w, float h);`|
|11|绘制圆角矩形|`void nvgRoundedRect(NVGcontext* ctx,float x, float y,float w, float h,float r);`|
|12|绘制椭圆|`void nvgEllipse(NVGcontext* ctx,float cx, float cy,float rx, float ry);`|
|13|绘制圆圈|`void nvgCircle(NVGcontext* ctx,float cx, float cy,float r);`|
|14|填充当前的路径|`void nvgFill(NVGcontext* ctx);`|
|15|用 StrokeColor 或者 StrokePaint 绘制当前的路径|`void nvgStroke(NVGcontext* ctx);`|

## 8.Text（文本）

首先必须创建字体 ==> 然后指定大小等 ==> 显示文字

| 序号 | 说明 | 方法 |
|----|----|---|
|1|从磁盘上加载指定的字体|`int nvgCreateFont(NVGcontext* ctx,const char* name,const char* filename);`|
|2|从内存中加载指定的字体|`int nvgCreateFontMem(NVGcontext* ctx,const char* name,unsigned char* data,int ndata, int freeData);`|
|3|寻找一个加载过的字体|`int nvgFindFont(NVGcontext* ctx,const char* name);`|
|4|设置字体的大小|`void nvgFontSize(NVGcontext* ctx,float size);`|
|5|设置字体的模糊风格|`void nvgFontBlur(NVGcontext* ctx,float blur);`|
|6|设置文本字母间的间隙|`void nvgTextLetterSpacing(NVGcontext* ctx,float spacing);`|
|7|设置文本行高度|`void nvgTextLineHeight(NVGcontext* ctx,float lineHeight);`|
|8|设置文本的对齐方式，参见枚举 NVGaling|`void nvgTextAlign(NVGcontext* ctx,int align);`|
|9|使用指定的字体|`void nvgFontFaceId(NVGcontext* ctx,int font);`|
|10|通过字体名字设置字体|`void nvgFontFace(NVGcontext* ctx,const char* font);`|
|11|在指定位置创建文字|`float nvgText(NVGcontext* ctx,float x, float y,const char* string,const char* end);`|
|12|创建多行文字|`void nvgTextBox(NVGcontext* ctx,float x, float y,float breakRowWidth, const char* string, const char* end);`|
|13|创建文字并指定边界|`float nvgTextBounds(NVGcontext* ctx,float x, float y,const char* string,const char* end,float* bounds);`|
|14|创建多行文字并指定边界|`void nvgTextBoxBounds(NVGcontext* ctx,float x, float y,float breakRowWidth, const char* string, const char* end, float* bounds);`|
|15|计算指定文本的 X 坐标|`int nvgTextGlyphPositions(NVGcontext* ctx,float x, float y,const char* string,const char* end,NVGglyphPosition* positions, int maxPositions);`|
|16|返回文字间的垂直距离|`void nvgTextMetrics(NVGcontext* ctx,float* ascender, float* descender,float* lineh);`|
|17|将指定的文本分成多行|`int nvgTextBreakLines(NVGcontext* ctx,const char* string,const char* end,float breakRowWidth, NVGtextRow* rows, int maxRows);`|
