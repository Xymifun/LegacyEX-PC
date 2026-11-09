#ifndef GAME_UI_FUIRENDERNODE_HPP
#define GAME_UI_FUIRENDERNODE_HPP

#include "game/ui/fuiMatrix.hpp"
#include "game/ui/fuiObject.hpp"
#include "game/ui/fuiRGBA.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiSymbol.hpp"
class FJ_FuiNode;
class FJ_Factory;
class fui;
struct fuiFile;
class fuiRenderNodeEditText;
class fuiRenderNodeStage;

class fuiRenderNode
{
    friend class fui;
    friend class fuiRenderNodeTimeline;
    friend class fuiRenderNodeCodeGenRect;
    friend class FJ_Factory;
    friend class FJ_FuiNode;

protected:
    unsigned int m_flags;
    unsigned char m_renderFlags;
    unsigned char m_filterFlags;
    unsigned int m_symbolIndex;
    unsigned int m_frame;
    fuiObject* m_object;
    eFuiObjectType m_type;
    fuiMatrix m_localMatrix;
    fuiMatrix m_globalMatrix;
    fuiFile* m_file;
    float m_redAdd;
    float m_greenAdd;
    float m_blueAdd;
    float m_alphaAdd;
    float m_redMultiply;
    float m_greenMultiply;
    float m_blueMultiply;
    float m_alpha;
    fuiRect m_bounds;
    char m_name[64];
    fuiRect m_scrollRect;
    unsigned int m_color;
    unsigned char m_visible;
    fuiRenderNodeStage* m_stage;
    fuiRenderNode* m_parent;
    FJ_FuiNode* m_node;

public:
    fuiRenderNode(fuiRenderNode*, fuiRenderNode*, fuiObject*, unsigned int, eFuiObjectType, unsigned char, fuiRGBA*,
                  unsigned char, bool, fuiFile*);
    virtual ~fuiRenderNode();
    virtual void shutdown();
    virtual void gather(bool) = 0;
    virtual void preRender() = 0;
    virtual void render(fuiRect*, fuiMatrix*) = 0;
    virtual void handleAddedToStage();
    virtual fuiRenderNode* findNode(const char*);
    virtual fuiRenderNode* getNodeFromPath(const char*);
    virtual void setVisibility(bool);
    virtual void setX(float);
    virtual void setY(float);
    virtual void setWidth(float);
    virtual void setHeight(float);
    virtual float getWidth();
    virtual float getHeight();
    virtual void getParentSpaceBounds(fuiRect*);
    virtual void generateGlobalMatrix();
    void disableCtor();
    void pushMatrix(fuiMatrix&);
    void setScrollRectViewport();
    void popMatrix();
    void clearScrollRectViewport();
    bool isVisible();
    void setAlpha(float);
    float getAlpha();
    fuiRenderNode* asTimeline();
    void progogateBounds();
    const char* getName() { return m_name; }
    fuiRect getBounds() const { return m_bounds; }
    void setScrollRect(const fuiRect& rect) { m_scrollRect = rect; }
    float getX();
    float getY();
    float getScaleX();
    void setScaleX(float);
    float getScaleY();
    void setScaleY(float);
    fuiRenderNodeStage* getStage();
    fuiRenderNode* getRoot();
    fuiRenderNode* getParent() { return m_parent; }
    FJ_FuiNode* getFuiNode() { return m_node; }
    void setFuiNode(FJ_FuiNode* node) { m_node = node; }
    void pushMatrix();
    void pushColor();
    void popColor();
    void pushFilterFlags();
    void popFilterFlags();
    fuiRenderNodeEditText* asEditText();
    fuiRect getGlobalBounds() const;
    void setGlobalBounds(const fuiRect&);
    fuiRenderNode* asShape();
    fuiRenderNode* asBitmap();
};

#endif
