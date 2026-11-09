#ifndef GAME_UI_UISCENE_HOWTOPLAY_HPP
#define GAME_UI_UISCENE_HOWTOPLAY_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_DynamicLabel.hpp"
#include "game/ui/UIControl_EnchantmentBook.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/item/not_null_ptr.hpp"

class Block;
class Item;
class ItemInstance;
class Potion;
class UILayer;
struct fuiRect;

enum EHowToPlayPage
{
    EHowToPlayPage_WhatsNew,
    EHowToPlayPage_Basics,
    EHowToPlayPage_Multiplayer,
    EHowToPlayPage_HUD,
    EHowToPlayPage_BattleMiniGame,
    EHowToPlayPage_TumbleMiniGame,
    EHowToPlayPage_GlideMiniGame,
    EHowToPlayPage_Creative,
    EHowToPlayPage_Inventory,
    EHowToPlayPage_Chest,
    EHowToPlayPage_LargeChest,
    EHowToPlayPage_EnderChest,
    EHowToPlayPage_Crafting,
    EHowToPlayPage_CraftingTable,
    EHowToPlayPage_Furnace,
    EHowToPlayPage_Dispenser,
    EHowToPlayPage_Brewing,
    EHowToPlayPage_Enchanting,
    EHowToPlayPage_Anvil,
    EHowToPlayPage_FarmingAnimals,
    EHowToPlayPage_BreedingAnimals,
    EHowToPlayPage_Trading,
    EHowToPlayPage_Horses,
    EHowToPlayPage_Beacons,
    EHowToPlayPage_Fireworks,
    EHowToPlayPage_Banners,
    EHowToPlayPage_Hoppers,
    EHowToPlayPage_Droppers,
    EHowToPlayPage_NetherPortal,
    EHowToPlayPage_TheEnd,
    EHowToPlayPage_EndCities,
    EHowToPlayPage_Elytra,
    EHowToPlayPage_Oceans,
    EHowToPlayPage_OceanMobs,
    EHowToPlayPage_Conduit,
    EHowToPlayPage_HostOptions,
    EHowToPlayPage_Count
};

class UIScene_HowToPlay : public UIScene
{
public:
    class sCustomItems
    {
    public:
        sCustomItems(Item* item, int count, int auxValue);
        sCustomItems(Block* block, int count, int auxValue);
        sCustomItems(int blockId, int count, int auxValue);
        sCustomItems(Item* item, Potion* potion, int count, int auxValue);
        not_null_ptr<ItemInstance> GetItemInstance() const;

        Block* m_block;
        Item* m_item;
        Potion* m_potion;
        int m_count;
        int m_auxValue;
    };

    UIScene_HowToPlay(int playerId, void* data, UILayer* layer);

    void StartPage(EHowToPlayPage page);
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    void customDrawEnchantmentFont(fuiRect* rect, int variant);
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual int getSceneType();
    virtual unsigned int getSubSceneType() const;

private:
    int m_page;
    int m_pageCount;
    UIControl m_container;
    UIControl_DynamicLabel m_dynamicHtmlText;
    UIControl_Label m_labels[30];
    UIControl_EnchantmentBook m_enchantmentBook;
    UIControl_EnchantmentBook m_enchantmentBooks[3];
};

#endif
