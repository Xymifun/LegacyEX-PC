#include "game/item/WrittenBookItem.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/network/CGameNetworkManager.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/StringTag.hpp"
#include "game/core/Slot.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/ClientboundContainerSetSlotPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/item/WritableBookItem.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

const std::wstring WrittenBookItem::TAG_TITLE = L"title";
const std::wstring WrittenBookItem::TAG_AUTHOR = L"author";
const std::wstring WrittenBookItem::TAG_PAGES = L"pages";
const std::wstring WrittenBookItem::TAG_GENERATION = L"generation";
const std::wstring WrittenBookItem::TAG_RESOLVED = L"resolved";
const std::wstring WrittenBookItem::TAG_OWNER_UUID = L"ownerUUID";
const std::wstring WrittenBookItem::TAG_CREATED_BY_RESTRICTED_PLAYER = L"createdByRestrictedPlayer";
const std::wstring WrittenBookItem::TAG_CREATED_BY_HOST = L"createdByHost";

WrittenBookItem::WrittenBookItem() : Item()
{
    setMaxStackSize(1);
}

bool WrittenBookItem::makeSureTagIsValid(CompoundTag* tag)
{
    if (!WritableBookItem::makeSureTagIsValid(tag))
    {
        return false;
    }

    if (!tag->contains(TAG_TITLE, Tag::TAG_STRING))
    {
        return false;
    }

    std::wstring title = tag->getString(TAG_TITLE);
    if (title.empty() || title.size() > 32)
    {
        return false;
    }

    return tag->contains(TAG_AUTHOR, Tag::TAG_STRING);
}

int WrittenBookItem::getGeneration(not_null_ptr<ItemInstance> itemInstance)
{
    return itemInstance->getTag()->getInt(TAG_GENERATION);
}

std::wstring WrittenBookItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->hasTag())
    {
        CompoundTag* tag = itemInstance->getTag();

        bool ownedByLocalPlayer = false;
        if (tag->getString(TAG_OWNER_UUID) != L"")
        {
            for (int i = 0; i < 4; ++i)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[i];
                if (localPlayer != NULL && localPlayer->getUUID() == tag->getString(TAG_OWNER_UUID))
                {
                    ownedByLocalPlayer = true;
                    break;
                }
            }
        }

        bool chatCensored = gApp.IsConsoleChatCensored();
        bool createdByRestrictedPlayer = false;
        if (tag->contains(TAG_CREATED_BY_RESTRICTED_PLAYER))
        {
            if (tag->getByte(TAG_CREATED_BY_RESTRICTED_PLAYER) != 0)
                createdByRestrictedPlayer = true;
        }
#ifdef MINECRAFT_PC
        createdByRestrictedPlayer = false;
#endif
        bool createdByHost = true;
        if (tag->contains(TAG_CREATED_BY_HOST) && tag->getByte(TAG_CREATED_BY_HOST) == 0)
            createdByHost = false;

        if ((chatCensored || createdByRestrictedPlayer) && !ownedByLocalPlayer
            && (!CGameNetworkManager::sInstance.IsHost() || !createdByHost))
        {
            return Item::getName(itemInstance);
        }

        std::wstring title = tag->getString(TAG_TITLE);
        if (!title.empty())
            return title;
    }
    return Item::getName(itemInstance);
}

void WrittenBookItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                      std::vector<HtmlString>* lines, bool advanced)
{
    if (itemInstance->hasTag())
    {
        CompoundTag* tag = itemInstance->getTag();
        std::wstring author = tag->getString(TAG_AUTHOR);
        if (!author.empty())
        {
            std::wstring line = L10N::GetString(IDS_BOOK_BY_AUTHOR);
            line.replace(line.find(L"%s"), 2, author);
            lines->push_back(HtmlString(line, eMinecraftColour_NOTSET, false, false, 0));
        }

        unsigned int generationIds[4];
        generationIds[0] = IDS_BOOK_GENERATION_0;
        generationIds[1] = IDS_BOOK_GENERATION_1;
        generationIds[2] = IDS_BOOK_GENERATION_2;
        generationIds[3] = IDS_BOOK_GENERATION_3;
        lines->push_back(HtmlString(L10N::GetString(generationIds[tag->getInt(TAG_GENERATION)]),
                                    eMinecraftColour_NOTSET, false, false, 0));
    }
}

void WrittenBookItem::resolveBookComponents(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player)
{
    if (itemInstance->getTag() == NULL)
    {
        return;
    }

    CompoundTag* tag = itemInstance->getTag();
    if (tag->getBoolean(TAG_RESOLVED))
    {
        return;
    }

    tag->putBoolean(TAG_RESOLVED, true);
    if (!makeSureTagIsValid(tag))
    {
        return;
    }

    ListTag* pages = tag->getList(TAG_PAGES, Tag::TAG_STRING);
    for (int i = 0; i < pages->size(); ++i)
    {
        pages->set(i, new StringTag(pages->getString(i)));
    }
    tag->put(TAG_PAGES, pages);

    if (!eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, player->GetType()) || player->getMainHandItem() != itemInstance)
    {
        return;
    }

    boost::shared_ptr<ServerPlayer> serverPlayer = boost::static_pointer_cast<ServerPlayer>(player);
    Slot* slot = player->mContainerMenu->getSlotFor(player->mInventory, player->mInventory->m_selected);
    serverPlayer->mConnection->send(boost::shared_ptr<ClientboundContainerSetSlotPacket>(
        new ClientboundContainerSetSlotPacket(0, slot->mIndex, itemInstance)));
}

InteractionResultHolder WrittenBookItem::use(Level* level, boost::shared_ptr<Player> player,
                                             InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!level->mIsClientSide)
    {
        resolveBookComponents(itemInstance, player);
    }

    player->openItemInstanceGui(itemInstance, hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool WrittenBookItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    return true;
}

unsigned int WrittenBookItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return IDS_TOOLTIPS_READ;
}

bool WrittenBookItem::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}

bool WrittenBookItem::HasBookTags(not_null_ptr<ItemInstance> itemInstance)
{
    return itemInstance->hasTag()
           && (itemInstance->getTag()->contains(TAG_PAGES) || itemInstance->getTag()->contains(TAG_TITLE));
}
