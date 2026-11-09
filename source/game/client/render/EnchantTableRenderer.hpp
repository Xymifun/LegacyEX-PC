#ifndef GAME_CLIENT_RENDER_ENCHANTTABLERENDERER_HPP
#define GAME_CLIENT_RENDER_ENCHANTTABLERENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockEntityRenderer.hpp"

class BlockEntity;
class BookModel;

class EnchantTableRenderer : public BlockEntityRenderer
{
public:
    EnchantTableRenderer();
    virtual ~EnchantTableRenderer();

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);

private:
    friend class UIControl_EnchantmentBook;

    BookModel* mBookModel;
};

#endif
