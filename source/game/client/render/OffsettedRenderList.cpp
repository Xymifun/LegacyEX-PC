#include "game/client/render/OffsettedRenderList.hpp"

#include "game/core/Buffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/IntBuffer.hpp"

OffsettedRenderList::OffsettedRenderList()
    : x(0),
      y(0),
      z(0),
      cameraX(0.0f),
      cameraY(0.0f),
      cameraZ(0.0f),
      glListBuffer(new IntBuffer(65536)),
      valid(false),
      bufferUploaded(false)
{
}

void OffsettedRenderList::init(int x_, int y_, int z_, double cameraX_, double cameraY_, double cameraZ_)
{
    valid = true;
    x = x_;
    y = y_;
    z = z_;
    cameraX = (float)cameraX_;
    cameraY = (float)cameraY_;
    cameraZ = (float)cameraZ_;
    glListBuffer->clear();
    bufferUploaded = false;
}

bool OffsettedRenderList::isAt(int x_, int y_, int z_)
{
    return x == x_ && y == y_ && z == z_;
}

void OffsettedRenderList::add(int list)
{
    ((IntBuffer*)glListBuffer)->put(list);
}

void OffsettedRenderList::render()
{
    if (!valid)
    {
        return;
    }

    if (!bufferUploaded)
    {
        ((IntBuffer*)glListBuffer)->flip();
        bufferUploaded = true;
    }

    if (glListBuffer->remaining() > 0u)
    {
        GlStateManager::pushMatrix();
        GlStateManager::translatef((float)x - cameraX, (float)y - cameraY, (float)z - cameraZ);
        GlStateManager::callLists((IntBuffer*)glListBuffer);
        GlStateManager::popMatrix();
    }
}

void OffsettedRenderList::clear()
{
    valid = false;
    glListBuffer->clear();
    bufferUploaded = false;
}
