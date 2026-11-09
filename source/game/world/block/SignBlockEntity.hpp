#ifndef GAME_WORLD_BLOCK_SIGNBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_SIGNBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockEntity.hpp"

class CompoundTag;
class Level;
class Packet;
class Player;
struct _STRING_VERIFY_RESPONSE;

class SignBlockEntity : public BlockEntity
{
public:
    SignBlockEntity();
    virtual ~SignBlockEntity();

    static BlockEntity* create();
    void setUniqueId();
    void SetMessage(int line, std::wstring& message);
    bool IsVerified();
    void SetVerified(bool verified);
    bool IsCensored();
    void SetCensored(bool censored);
    void setCreatedByRestrictedPlayer(bool restricted);
    bool wasCreatedByRestrictedPlayer() const;
    bool wasCreatedByHostPlayer();
    void setCreatedByHostPlayer(bool host);
    virtual void tick();
    virtual CompoundTag* save(CompoundTag* tag);
    virtual void setLevelForLoading(Level* level);
    virtual void load(CompoundTag* tag);
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    virtual bool onlyOpCanSetNbt();
    void setCreatedPlayer(const std::wstring& player);
    std::wstring getCreatedPlayerUUID();
    static unsigned int StringVerifyCallback(void* context, _STRING_VERIFY_RESPONSE* response);
    virtual void setChanged();
    virtual boost::shared_ptr<BlockEntity> clone();
    bool executeClickCommands(boost::shared_ptr<Player> player);
    int getUniqueId();
    void setTextCacheIndex(int index);
    int getTextCacheIndex();
    void disableTextCache();
    bool getTextCacheEnabled();
    virtual unsigned long long GetType();
    bool isEditable();
    void setEditable(bool editable);

private:
    std::wstring mCreatedPlayer;
    bool mEditable;
    bool mVerified;
    bool mCensored;
    bool mCreatedByRestrictedPlayer;
    bool mCreatedByHostPlayer;

public:
    int mSelectedLine;
    std::wstring mMessages[4];

private:
    int mUniqueId;
    int mTextCacheIndex;
    bool mTextCacheEnabled;
};

#endif
