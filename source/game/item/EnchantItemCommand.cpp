#include "game/item/EnchantItemCommand.hpp"

#include <string>

#include "game/command/CommandSender.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/server/ServerPlayer.hpp"

EGameCommand EnchantItemCommand::getId()
{
    return eGameCommand_EnchantItem;
}

int EnchantItemCommand::getPermissionLevel()
{
    return 2;
}

void EnchantItemCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);

    PlayerUID uid = stream.readPlayerUID();
    int enchantmentId = stream.readInt();
    int level = stream.readInt();
    byteStream.reset();

    boost::shared_ptr<ServerPlayer> player = getPlayer(uid);

    sender->setCommandStats(CommandStats::AFFECTED_ITEMS, 0);

    if (player.get() == NULL)
    {
        return;
    }

    not_null_ptr<ItemInstance> item = player->getMainHandItem();
    if (item->isEmpty())
    {
        return;
    }

    Enchantment* enchantment = Enchantment::byId(enchantmentId);
    if (enchantment == NULL)
    {
        return;
    }

    if (!enchantment->canEnchant(item))
    {
        return;
    }

    if (level < enchantment->getMinLevel())
    {
        level = enchantment->getMinLevel();
    }
    if (level > enchantment->getMaxLevel())
    {
        level = enchantment->getMaxLevel();
    }

    if (item->hasTag())
    {
        ListTag* enchantmentTags = item->getEnchantmentTags();
        if (enchantmentTags != NULL)
        {
            for (int i = 0; i < enchantmentTags->size(); ++i)
            {
                int existingId = enchantmentTags->getCompound(i)->getShort(ItemInstance::TAG_ENCH_ID);
                if (Enchantment::byId(existingId) != NULL
                    && !Enchantment::byId(existingId)->isCompatibleWith(enchantment))
                {
                    return;
                }
            }
        }
    }

    item->enchant(enchantment, level);

    std::wstring stringParams[1];
    stringParams[0] = L"commands.enchant.success";
    logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_Text, NULL, 0, stringParams, 1);
    sender->setCommandStats(CommandStats::AFFECTED_ITEMS, 1);
}

boost::shared_ptr<GameCommandPacket> EnchantItemCommand::preparePacket(boost::shared_ptr<Player> player,
                                                                       int enchantmentId, int level)
{
    if (player.get() == NULL)
    {
        return boost::shared_ptr<GameCommandPacket>();
    }

    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    PlayerUID uid = player->mXuid;
    stream.writePlayerUID(uid);
    stream.writeInt(enchantmentId);
    stream.writeInt(level);

    return boost::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_EnchantItem, byteStream.toByteArray()));
}
