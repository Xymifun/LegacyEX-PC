#ifndef GAME_CLIENT_RENDER_SIGNRENDERER_HPP
#define GAME_CLIENT_RENDER_SIGNRENDERER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockEntityRenderer.hpp"
#include "game/client/render/SignModel.hpp"

class BlockEntity;

class SignRenderer : public BlockEntityRenderer
{
public:
    SignRenderer();
    virtual ~SignRenderer();

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float partialTicks,
                        int breakProgress, bool translucent, float alpha, bool useDisplayList);

    void CensorString(std::wstring& text);
    void tick();

private:
    struct TextCacheEntry
    {
        int uniqueId;
        int age;
    };

    TextCacheEntry mTextCache[200];
    int mTextDisplayLists;
    int mTextCacheCount;
    SignModel* mModel;
    bool mCensorText;
    int mLanguage;
};

#endif
