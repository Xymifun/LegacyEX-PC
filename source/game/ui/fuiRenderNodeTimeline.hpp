#ifndef GAME_UI_FUIRENDERNODETIMELINE_HPP
#define GAME_UI_FUIRENDERNODETIMELINE_HPP

#include <string>
#include <vector>

#include "game/ui/fuiRenderNode.hpp"

struct fuiFile;

struct fuiRenderNodeRemoved
{
    unsigned int depth;
    fuiObject* object;
    fuiRenderNodeRemoved(unsigned int d, fuiObject* o) : depth(d), object(o) {}
};

class fuiRenderNodeTimeline : public fuiRenderNode
{
public:
    enum eFrameActionType
    {
        eFrameActionType_GotoAndStop = 0,
        eFrameActionType_Stop = 1,
        eFrameActionType_GotoAndStopLabel = 2,
        eFrameActionType_GotoAndPlay = 3
    };

    struct frameAction
    {
        int frame;
        eFrameActionType type;
        std::string value;

        frameAction(const frameAction& other);
        frameAction() : frame(0), type(eFrameActionType_GotoAndStop), value() {}
    };

private:
    std::vector<fuiRenderNode*> m_children;
    std::vector<fuiRenderNodeRemoved> m_removedChildren;
    fuiRenderNode* m_mask;
    int m_currentFrame;
    int m_previousFrame;
    std::vector<frameAction> m_frameActions;

    void handleConstruction();
    int getFrame(const char* name);
    void handleFrameTransition(int frame);
    fuiObject* getReferencedObject(fuiObject* object);
    fuiRenderNode* createReferencedNode(fuiObject* object, unsigned int depth, unsigned char renderFlags,
                                        fuiRGBA* color, unsigned char clipDepth, bool visible);
    static bool depthComparison(const fuiRenderNode* first, const fuiRenderNode* second);
    void depthSort();
    void expandBoundsFromChild(fuiRect& bounds);
    void updateBoundsLocal();
    void updateBoundsTree();

public:
    fuiRenderNodeTimeline(fuiRenderNode* stage, fuiRenderNode* parent, fuiObject* object, unsigned int symbolIndex,
                          unsigned char depth, fuiRGBA* color, unsigned char clipDepth, bool visible, fuiFile* file);
    virtual ~fuiRenderNodeTimeline();
    void setPlaying(bool playing);
    void setFrame(unsigned int frame);
    void setFrame(const char* name);
    void addFrameAction(int frame, eFrameActionType type, const std::string& value);
    virtual void shutdown();
    virtual void gather(bool force);
    virtual void preRender();
    virtual void render(fuiRect* bounds, fuiMatrix* matrix);
    virtual void handleAddedToStage();
    virtual fuiRenderNode* findNode(const char* name);
    virtual fuiRenderNode* getNodeFromPath(const char* path);
    int getNumChildren() { return static_cast<int>(m_children.size()); }
    fuiRenderNode* getChildAt(int index) { return m_children[index]; }
    void progogateBoundsFromChild();
    std::string currentLabel();
    int currentFrame();
    void removeChild(fuiRenderNode* child, bool destroy);
    void removeChildAt(int index, bool destroy);
    void addMask(fuiRenderNode* mask);
    void addChild(fuiRenderNode* child);
    void addChildAt(fuiRenderNode* child, int index);
    unsigned int getChildIndex(fuiRenderNode* child);
    virtual void generateGlobalMatrix();
};

#endif
