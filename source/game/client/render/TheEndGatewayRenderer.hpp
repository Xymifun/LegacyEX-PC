#ifndef GAME_CLIENT_RENDER_THEENDGATEWAYRENDERER_HPP
#define GAME_CLIENT_RENDER_THEENDGATEWAYRENDERER_HPP

#include "game/client/render/TheEndPortalRenderer.hpp"

#include <boost/shared_ptr.hpp>

class BlockEntity;

class TheEndGatewayRenderer : public TheEndPortalRenderer
{
public:
    virtual ~TheEndGatewayRenderer();

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);

protected:
    virtual int getPasses(double distance);
    virtual double getOffset();
};

#endif
