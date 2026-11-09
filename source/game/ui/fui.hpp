#ifndef GAME_UI_FUI_HPP
#define GAME_UI_FUI_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/fuiTextureManager.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiFontSet.hpp"
#include "game/ui/fuiMatrix.hpp"
#include "game/world/biome/arrayWithLength.hpp"

struct fuiBitmap;
class fuiBitmapFont;
class fuiRenderNode;
class fuiRenderNodeTimeline;
class fuiTTFFontSet;

struct FJ_EventListener
{
    eFJEventType m_eType;
    bool m_bCapture;
    FJ_FuiNode* m_pNode;
    void (*m_pfnHandler)(FJ_FuiNode*, FJ_Event*);
};

struct fuiFontIndirection
{
    std::string source;
    std::string target;
};

class fui
{
    friend class fuiRenderNode;
    friend class fuiRenderNodeEditText;
    friend class fuiRenderNodeShape;
    friend class fuiRenderNodeBitmap;
    friend class fuiRenderNodeCodeGenRect;

private:
    struct ColorState
    {
        float redAdd;
        float greenAdd;
        float blueAdd;
        float alphaAdd;
        float redMultiply;
        float greenMultiply;
        float blueMultiply;
        float alphaMultiply;
    };

    static fuiMatrix s_identityMatrix;
    static ColorState s_identityColor;

    float m_rendererMatrix[16];
    fuiMatrix m_matrixStack[10];
    int m_matrixDepth;
    ColorState m_colorStack[10];
    int m_colorDepth;
    unsigned char m_filterStack[12];
    int m_filterDepth;
    float m_originX;
    float m_originY;
    unsigned char m_stencilMask;
    float m_screenWidth;
    float m_screenHeight;
    std::vector<fuiFile*> m_files;
    int m_permanentFileIndices[64];
    unsigned int m_frameCount;
    bool m_enterFrameDispatch;
    int64_t m_lastTime;
    std::vector<FJ_EventListener> m_directEventListeners;
    std::vector<fuiFontSet*> m_fonts;
    std::vector<fuiFontIndirection> m_fontIndirections;
    void* m_customTextureContext;
    int (*m_customTextureFunction)(void*, const std::wstring&, unsigned int*, unsigned int*, bool*);
    void (*m_releaseCustomTextureFunction)(int);
    OSFastMutex m_mutex;
    fuiTextureManager* m_textureManager;
    int m_resolution;

public:
    fui();
    void removeManagedTexture(fuiFile* file, fuiBitmap* bitmap);
    static void EndianSwapImageData(void* data, unsigned int length);
    void manageTexture(fuiFile* file, fuiBitmap* bitmap);
    int allocatePermanentFileIndex(int fileIndex);
    intptr_t load(arrayWithLength<unsigned char> bytes, bool createRoot, int fileIndex);
    void unloadScene(fuiFile* file);
    void setScreenSize(float width, float height);
    void setResolution(int resolution);
    unsigned int getResolution();
    void setOrigin(float x, float y);
    void tick();
    void preRender(fuiFile* file);
    void render(fuiFile* file, float x, float y, float width, float height, float scaleX, float scaleY);
    void render(fuiFile* file, float x, float y, float width, float height);
    void dispatchKeyboardEvent(fuiFile* file, bool keyUp, int keyCode);
    void dispatchMouseMoveEvent(fuiFile* file, float x, float y);
    void dispatchEnterFrameEvent();
    void removeDirectEventListener(const FJ_EventListener& listener);
    void addDirectEventListener(const FJ_EventListener& listener);
    void removeEventListenerForNode(FJ_FuiNode* node);
    void setCustomTextureFn(int (*textureFunction)(void*, const std::wstring&, unsigned int*, unsigned int*, bool*),
                            void (*releaseFunction)(int), void* context);
    uint32_t getCustomTexture(const std::wstring& name, unsigned int* width, unsigned int* height, bool* hasAlpha);
    void releaseCustomTexture(int textureId);
    unsigned char getCurrentStencilMask();
    void incrementStencilMask();
    uintptr_t getFuiFileFromIndex(unsigned int index);
    fuiRenderNode* createNodeFromSymbol(const char* name, fuiRenderNode* parent);
    intptr_t findBitmapFromSymbol(const char* name, fuiFile** file);
    void installTrueTypeFont(int fontIndex, const void** data, const char* name, unsigned short missingGlyph);
    void uninstallTrueTypeFont(const char* name);
    void installBitmapFont(fuiBitmapFont* font);
    void setFontIndirection(const char* source, const char* target);
    uintptr_t getFont(const char* name, bool skipIndirection);
    uintptr_t getDefaultFont(bool skipIndirection);
    unsigned int GetCurrentFrameCount();
    void unloadAll();
    void render(fuiFile* file);
};

extern fui* gFui;

#endif
