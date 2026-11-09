#ifndef GAME_CORE_GLSTATEMANAGER_HPP
#define GAME_CORE_GLSTATEMANAGER_HPP

class ByteBuffer;
class FloatBuffer;
class IntBuffer;
class Matrix4f;

extern const int GL_OBJECT_PLANE;
extern const int GL_EYE_PLANE;
extern const int GL_EYE_LINEAR;
extern const int GL_TEXTURE;
extern const int GL_GREATER;
extern const int GL_MODELVIEW;
extern const int GL_MODELVIEW_MATRIX;
extern const int GL_PROJECTION_MATRIX;

class GlStateManager
{
public:
    enum TexGen
    {
        S,
        T,
        R,
        Q
    };

    class BooleanState
    {
    public:
        BooleanState(int capability);

        void setEnabled(bool enabled);
        void enable();
        void disable();

        int capability;
        bool state;
    };

    class Color
    {
    public:
        Color();
        Color(float red, float green, float blue, float alpha);

        float red;
        float green;
        float blue;
        float alpha;
    };

    class AlphaState
    {
    public:
        AlphaState();
        ~AlphaState();

        BooleanState* alphaTest;
        int func;
        float ref;
    };

    class BlendState
    {
    public:
        BlendState();
        ~BlendState();

        BooleanState* blend;
        int srcFactor;
        int dstFactor;
        int srcFactorAlpha;
        int dstFactorAlpha;
    };

    class ClearState
    {
    public:
        ClearState();
        ~ClearState();

        double depth;
        Color* color;
        int stencil;
    };

    class ColorLogicState
    {
    public:
        ColorLogicState();
        ~ColorLogicState();

        BooleanState* colorLogicOp;
        int opcode;
    };

    class ColorMask
    {
    public:
        ColorMask();

        bool red;
        bool green;
        bool blue;
        bool alpha;
    };

    class ColorMaterialState
    {
    public:
        ColorMaterialState();
        ~ColorMaterialState();

        BooleanState* colorMaterial;
        int face;
        int mode;
    };

    class CullState
    {
    public:
        CullState();
        ~CullState();

        BooleanState* cullFace;
        int mode;
    };

    class DepthState
    {
    public:
        DepthState();
        ~DepthState();

        BooleanState* depthTest;
        bool maskEnabled;
        int depthFunc;
    };

    class FogState
    {
    public:
        FogState();
        ~FogState();

        BooleanState* fog;
        int mode;
        float density;
        float start;
        float end;
    };

    class PolygonOffsetState
    {
    public:
        PolygonOffsetState();
        ~PolygonOffsetState();

        BooleanState* polyOffsetFill;
        BooleanState* polyOffsetLine;
        float factor;
        float units;
    };

    class StencilFunc
    {
    public:
        StencilFunc();

        int func;
        int ref;
        int mask;
    };

    class StencilState
    {
    public:
        StencilState();
        ~StencilState();

        StencilFunc* func;
        int mask;
        int fail;
        int zfail;
        int zpass;
    };

    class TexGenCoord
    {
    public:
        TexGenCoord(int coord, int capability);
        ~TexGenCoord();

        BooleanState* textureGen;
        int coord;
        int mode;
    };

    class TexGenState
    {
    public:
        TexGenState();
        ~TexGenState();

        TexGenCoord* s;
        TexGenCoord* t;
        TexGenCoord* r;
        TexGenCoord* q;
    };

    class TextureState
    {
    public:
        TextureState();
        ~TextureState();

        BooleanState* texture2DState;
        int textureName;
    };

    static void staticCtor();

    static void enableState(int capability);
    static void disableState(int capability);

    static void pushMatrix();
    static void popMatrix();
    static void loadIdentity();
    static void matrixMode(int mode);
    static void translatef(float x, float y, float z);
    static void translated(double x, double y, double z);
    static void scalef(float x, float y, float z);
    static void scaled(double x, double y, double z);
    static void rotatef(float angle, float x, float y, float z);
    static void rotated(double angle, double x, double y, double z);
    static void perspective(float fieldOfView, float aspect, float nearDistance, float farDistance);
    static void ortho(double left, double right, double bottom, double top, double nearDistance, double farDistance);
    static void viewport(int x, int y, int width, int height);
    static void getMatrix(int mode, FloatBuffer* buffer);
    static void multMatrix(FloatBuffer* buffer);
    static void multMatrix(Matrix4f* matrix);
    static void glMultMatrixf(float* matrix);

    static void enableLighting();
    static void disableLighting();
    static void enableLight(int light);
    static void disableLight(int light);
    static void light(int light, int parameter, FloatBuffer* buffer);
    static void lightModel(int parameter, FloatBuffer* buffer);
    static void pushLightingAttributes();
    static void popAttributes();

    static void enableColorMaterial();
    static void disableColorMaterial();
    static void colorMaterial(int face, int mode);

    static void enableDepthTest();
    static void disableDepthTest();
    static void depthFunc(int func);
    static void depthMask(bool flag);

    static void enableBlend();
    static void disableBlend();
    static void blendFunc(int srcFactor, int dstFactor);
    static void blendFuncSeparate(int srcFactor, int dstFactor, int srcFactorAlpha, int dstFactorAlpha);

    static void enableFog();
    static void disableFog();
    static void fogMode(int mode);
    static void fogDensity(float density);
    static void fogStart(float start);
    static void fogEnd(float end);
    static float getFogStart();
    static float getFogEnd();
    static void glFog(int parameter, FloatBuffer* buffer);

    static void enableCull();
    static void disableCull();
    static void cullFace(int mode);

    static void enablePolygonOffset();
    static void disablePolygonOffset();
    static void enableLineOffset();
    static void disableLineOffset();
    static void polygonOffset(float factor, float units);

    static void enableColorLogicOp();
    static void disableColorLogicOp();

    static void enableTexGen(TexGen coord);
    static void disableTexGen(TexGen coord);
    static void texGenMode(TexGen coord, int mode);
    static void texGenParam(TexGen coord, int parameter, FloatBuffer* buffer);

    static void enableNormalize();
    static void disableNormalize();
    static void enableRescaleNormal();
    static void disableRescaleNormal();

    static void enableAlphaTest();
    static void disableAlphaTest();
    static void alphaFunc(int func, float ref);

    static void colorMask(bool red, bool green, bool blue, bool alpha);
    static void clearDepth(double depth);
    static void clearColor(float red, float green, float blue, float alpha);
    static void clear(int flags);
    static void clearCurrentColor();

    static void activeTexture(int texture);
    static void glActiveTexture(int texture);
    static void enableTexture();
    static void disableTexture();
    static void bindTexture(int texture);
    static int genTexture();
    static void deleteTexture(int texture);
    static void genTextures(IntBuffer* buffer);
    static void deleteTextures(IntBuffer* buffer);
    static void texImage2D(int target, int level, int internalFormat, int width, int height, int border, int format,
                           int type, ByteBuffer* pixels);
    static void texParameteri(int target, int parameter, int value);
    static void texParameteri(int parameter, int value);
    static void texParameterf(int target, int parameter, float value);
    static void texLayerColor3f(int stage, float red, float green, float blue);
    static void multiTexCoord2f(float u, float v);
    static void setupSolidRenderingTextureCombine(int stage);
    static void tearDownSolidRenderingTextureCombine();

    static void color3f(float red, float green, float blue);
    static void color4f(float red, float green, float blue, float alpha);
    static void glColor3f(float red, float green, float blue);
    static void normal3f(float x, float y, float z);
    static void lineWidth(float width);
    static void shadeModel(int mode);
    static void glShadeModel(int mode);

    static int genLists(int count);
    static void newList(int list, int mode);
    static void endList(int list);
    static void callList(int list);
    static void callLists(IntBuffer* buffer);
    static void deleteLists(int list, int range);

    static void drawArrays(int mode, int first, int count);
    static void glReadPixels(int x, int y, int width, int height, int format, int type, ByteBuffer* pixels);
    static void glVertexPointer(int size, int type, int stride, int pointer);
    static void glVertexPointer(int size, int type, FloatBuffer* buffer);
    static void glTexCoordPointer(int size, int type, int stride, int pointer);
    static void glTexCoordPointer(int size, int type, FloatBuffer* buffer);
    static void glNormalPointer(int type, int stride, int pointer);
    static void glNormalPointer(int type, ByteBuffer* buffer);
    static void glColorPointer(int size, bool normalized, int stride, ByteBuffer* buffer);
    static void glColorPointer(int size, int type, int stride, int pointer);
    static void glEnableClientState(int capability);
    static void disableClientState(int capability);
    static void glClientActiveTexture(int texture);

    static void glGenQueriesARB(IntBuffer* buffer);
    static void glBeginQueryARB(int target, int query);
    static void glEndQueryARB(int target);
    static void glGetQueryObjectuARB(int query, int parameter, IntBuffer* buffer);

    static void flush();
    static void setProfile();
    static void unsetProfile();

private:
    static TexGenCoord* getTexGen(TexGen coord);
};

#endif
