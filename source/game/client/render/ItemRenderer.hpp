#ifndef GAME_CLIENT_RENDER_ITEMRENDERER_HPP
#define GAME_CLIENT_RENDER_ITEMRENDERER_HPP
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/client/render/EntityRenderer.hpp"
#include "game/item/ItemTransforms.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"
class Entity;
class EntityRenderDispatcher;
class Font;
class ItemColors;
class ItemEntity;
class ItemInstance;
class LivingEntity;
class Random;
class ResourceLocation;
class Tesselator;
class TextureAtlasSprite;
class Textures;
class ItemInHandRenderer;
class ItemRenderer : public EntityRenderer
{
public:
    static ResourceLocation ENCHANT_GLINT_LOCATION;

    ItemRenderer(EntityRenderDispatcher* dispatcher, Textures* textures, ItemColors* itemColors);
    virtual ~ItemRenderer();
    ItemColors* getItemColors();
    void renderFoilLayer(Tesselator* tesselator);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float rot, float partialTicks);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual const ResourceLocation* getTextureLocation(int iconType);
    virtual void renderItemBillboard(boost::shared_ptr<ItemEntity>& entity, TextureAtlasSprite* sprite, int count,
                                     float partialTicks, float red, float green, float blue);
    void setupGuiItem(float x, float y, float scaleX, float scaleY, bool lighting);
    bool isGui3d(not_null_ptr<ItemInstance> item);
    void renderStatic(const not_null_ptr<ItemInstance>& item, boost::shared_ptr<LivingEntity> entity,
                      ItemTransforms::TransformType transformType, float alpha, bool leftHand,
                      TextureAtlasSprite* sprite);
    void renderGuiItem(not_null_ptr<ItemInstance> item, float x, float y, float scaleX, float scaleY, float alpha, bool,
                       TextureAtlasSprite* sprite);
    void renderGuiItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float alpha);
    void renderGuiItem(not_null_ptr<ItemInstance> item, int x, int y);
    void renderAndDecorateItem(not_null_ptr<ItemInstance> item, float x, float y, float scaleX, float scaleY,
                               float alpha, bool, bool, bool lighting, TextureAtlasSprite* sprite);
    void renderAndDecorateItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float alpha, bool foil);
    void renderAndDecorateItem(not_null_ptr<ItemInstance> item, int x, int y);
    void fillRect(Tesselator* tesselator, int x, int y, int width, int height, int rgb, int alpha);
    void renderGuiItemDecorations(Font* font, not_null_ptr<ItemInstance> item, int x, int y,
                                  const std::wstring& countText, int colour, float alpha);
    void renderGuiItemDecorations(Font* font, not_null_ptr<ItemInstance> item, int x, int y, float alpha);
    void blit(float x, float y, TextureAtlasSprite* sprite, float width, float height);
    void blitGlint(int id, float x, float y, float width, float height);
    void blit(float x, float y, int sx, int sy, float width, float height);
    void SetItemFrame(bool itemFrame);

private:
    friend class ItemInHandRenderer;
    ItemColors* mItemColors;
    Random* mRandom;
    bool mItemFrameEnabled;
    Textures* mTextures;
    bool mRenderWithColor;
    float mDepth;
};
#endif
