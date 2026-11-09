#ifndef GAME_CLIENT_RENDER_THEENDPORTALRENDERER_HPP
#define GAME_CLIENT_RENDER_THEENDPORTALRENDERER_HPP

#include "game/client/render/BlockEntityRenderer.hpp"

#include <boost/shared_ptr.hpp>

class BlockEntity;
class FloatBuffer;

class TheEndPortalRenderer : public BlockEntityRenderer
{
public:
    TheEndPortalRenderer();
    virtual ~TheEndPortalRenderer() {}

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);

protected:
    virtual int getPasses(double distanceSqr);
    virtual double getOffset();

    FloatBuffer* getBuffer(float x, float y, float z, float w);

    FloatBuffer* mBuffer;
};

#endif
