#ifndef GAME_CORE_CENSOREDSTRING_HPP
#define GAME_CORE_CENSOREDSTRING_HPP

#include <string>
#include <stdint.h>

#include <boost/enable_shared_from_this.hpp>

#include "game/core/TickAction.hpp"
#include "game/data/CompoundTag.hpp"

class CensoredString : public boost::enable_shared_from_this<CensoredString>
{
public:
    CensoredString();
    CensoredString(int firstId, int secondId);
    CensoredString(TickAction* action, int firstId, int secondId);
    ~CensoredString();

    void _init(TickAction* action, int firstId, int secondId);
    std::wstring getPendingString();
    std::wstring getCensoredString();
    void _verified(const std::wstring& value, bool censored);
    void _changed();
    void set(const std::wstring& value, const std::wstring& uuid, bool chatCensored, bool uuidLocal);
    uint32_t checkParentalFilter();
    std::wstring raw();
    void saveToTag(CompoundTag* tag);
    void loadFromTag(CompoundTag* tag);
    std::wstring get();

    CensoredString(TickAction* action);
    void _update(const std::wstring& value);
    int compare(const std::wstring& value);

    std::wstring getUUID() { return m_uuid; }
    bool isUUIDLocal() { return m_uuidLocal; }
    bool isChatCensored() { return m_chatCensored; }

private:
    bool m_useLocalization;
    unsigned char m_stateBytes[47];
    unsigned char m_flags;
    int m_pendingId;
    int m_censoredId;
    std::wstring m_pending;
    std::wstring m_censored;
    TickAction* m_tickAction;
    std::wstring m_uuid;
    bool m_chatCensored;
    bool m_uuidLocal;
};

#endif
