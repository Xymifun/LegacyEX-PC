#include "game/ui/fuiRenderNodeStage.hpp"
#include "game/ui/fuiFile.hpp"

fuiRenderNodeStage::fuiRenderNodeStage(fuiFile* file)
    : fuiRenderNode(0, 0, 0, 0, eFuiObjectType_None, 0, 0, 0, false, file),
      m_child(0)
{
}

fuiRenderNodeStage::~fuiRenderNodeStage()
{
    m_child->shutdown();
    delete m_child;
}

void fuiRenderNodeStage::addChild(fuiRenderNode* child)
{
    m_child = child;
}

fuiRenderNode* fuiRenderNodeStage::getChild()
{
    return m_child;
}

void fuiRenderNodeStage::gather(bool force)
{
    m_child->gather(force);
}

void fuiRenderNodeStage::preRender()
{
    m_child->preRender();
}

void fuiRenderNodeStage::render(fuiRect* rect, fuiMatrix* matrix)
{
    m_child->render(rect, matrix);
}

fuiRenderNode* fuiRenderNodeStage::findNode(const char* name)
{
    return m_child->findNode(name);
}

float fuiRenderNodeStage::getStageWidth()
{
    return m_file->getStageWidth();
}

float fuiRenderNodeStage::getStageHeight()
{
    return m_file->getStageHeight();
}
