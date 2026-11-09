#ifndef GAME_CLIENT_RENDER_MAPRENDERER_HPP
#define GAME_CLIENT_RENDER_MAPRENDERER_HPP

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/world/biome/arrayWithLength.hpp"

class Font;
class MapItemSavedData;
class Options;
class Player;
class Textures;

class MapRenderer
{
public:
    class MapInstance
    {
    public:
        MapInstance(MapRenderer* renderer, const boost::shared_ptr<MapItemSavedData>& data);

        void fjUpdateWithRenderContext();
        void draw(const boost::shared_ptr<Player>& player);
        void updateTexture();

    public:
        MapRenderer* mRenderer;
        boost::shared_ptr<MapItemSavedData> mData;
        int mTexture;
        arrayWithLength<int> mPixels;
        bool mTextureDirty;
    };

    MapRenderer(Font* font, Options* options, Textures* textures);

    static void reloadColours();
    MapInstance* getMapInstance(const boost::shared_ptr<MapItemSavedData>& data);
    void render(const boost::shared_ptr<MapItemSavedData>& data, const boost::shared_ptr<Player>& player);
    void update(const boost::shared_ptr<MapItemSavedData>& data);
    MapInstance* getMapInstanceIfExists(const std::wstring& id);
    void resetData();
    boost::shared_ptr<MapItemSavedData> getData(MapInstance* instance);
    static arrayWithLength<int> DebugGeneratePixelsFromColours(arrayWithLength<unsigned char> colours);

private:
    Textures* mTextures;
    boost::unordered_map<std::wstring, MapInstance*> mMapInstances;
    Options* mOptions;
    Font* mFont;
};

#endif
