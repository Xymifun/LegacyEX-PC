#include "game/core/CensoredString.hpp"

#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/core/L10N.hpp"

static const std::wstring PENDING_FALLBACK;
static const std::wstring CENSORED_FALLBACK;
static const std::wstring OWNER_UUID_KEY(L"CSownerUUID");
static const std::wstring NAMED_BY_RESTRICTED_KEY(L"CSnamedByRestricted");
static const std::wstring CREATED_ON_HOST_KEY(L"CScreatedOnHost");

CensoredString::CensoredString()
    : boost::enable_shared_from_this<CensoredString>(),
      m_useLocalization(false),
      m_flags(0),
      m_pending(),
      m_censored(),
      m_uuid()
{
    _init(NULL, -1, -1);
}

CensoredString::CensoredString(int firstId, int secondId)
    : boost::enable_shared_from_this<CensoredString>(),
      m_useLocalization(false),
      m_flags(0),
      m_pending(),
      m_censored(),
      m_uuid()
{
    _init(NULL, firstId, secondId);
}

CensoredString::CensoredString(TickAction* action, int firstId, int secondId)
    : boost::enable_shared_from_this<CensoredString>(),
      m_useLocalization(false),
      m_flags(0),
      m_pending(),
      m_censored(),
      m_uuid()
{
    _init(action, firstId, secondId);
}

CensoredString::~CensoredString()
{
}

void CensoredString::_init(TickAction* action, int firstId, int secondId)
{
    m_tickAction = action;
    m_censored = L"";
    m_flags &= static_cast<unsigned char>(0xf8);
    if (m_useLocalization)
    {
        m_pending = L10N::GetString(firstId);
    }
    else
    {
        m_pending = L"";
        firstId = secondId = -1;
    }
    m_pendingId = firstId;
    m_censoredId = secondId;
    m_uuid = L"";
    m_chatCensored = false;
    m_uuidLocal = true;
}

std::wstring CensoredString::getPendingString()
{
    if (m_pendingId != -1)
        return std::wstring(L10N::GetString(m_pendingId));
    return PENDING_FALLBACK;
}

std::wstring CensoredString::getCensoredString()
{
    if (m_censoredId != -1)
        return std::wstring(L10N::GetString(m_censoredId));
    return CENSORED_FALLBACK;
}

void CensoredString::_verified(const std::wstring& value, bool censored)
{
    m_censored = L"";
    m_flags &= static_cast<unsigned char>(0xfe);
    m_flags |= static_cast<unsigned char>(4);
    m_pending = value;
    if (censored)
        m_flags = static_cast<unsigned char>(m_flags | 2);
    else
        m_flags = static_cast<unsigned char>(m_flags & 0xfd);
}

void CensoredString::_changed()
{
    if (m_tickAction != NULL)
    {
        if ((m_flags & 4) > 0)
            m_tickAction->Execute();
    }
    m_flags = static_cast<unsigned char>(m_flags & 0xfb);
}

void CensoredString::set(const std::wstring& value, const std::wstring& uuid, bool chatCensored, bool uuidLocal)
{
    m_uuid = uuid;
    m_chatCensored = chatCensored;
    m_uuidLocal = uuidLocal;
    if (m_useLocalization)
    {
        if (!((m_flags & 1) > 0 && m_censored == value))
        {
            if (value.empty())
            {
                _verified(L"", false);
            }
            else if (m_pending == value && (m_flags & 1) > 0)
            {
                _verified(m_pending, (m_flags & 2) > 0);
            }
        }
    }
    else
    {
        _verified(value, false);
    }
    _changed();
}

uint32_t CensoredString::checkParentalFilter()
{
    uint32_t isLocal = gApp.IsUUIDLocal(m_uuid);
#ifdef MINECRAFT_PC
    bool censored = false;
    (void)isLocal;
#else
    bool censored = !isLocal && (gApp.IsConsoleChatCensored() || m_chatCensored);
#endif
    bool blocked = !m_uuidLocal && censored;
    if (CGameNetworkManager::sInstance.IsHost() && blocked)
        return 0;
    if (!CGameNetworkManager::sInstance.IsHost() && censored)
        return 0;
    return 1;
}

std::wstring CensoredString::raw()
{
    std::wstring result;
    if (m_useLocalization)
    {
        if ((m_flags & 1) > 0)
            result = m_censored;
        else
            result = m_pending;
        return result;
    }
    return m_pending;
}

void CensoredString::saveToTag(CompoundTag* tag)
{
    tag->putBoolean(CREATED_ON_HOST_KEY, m_uuidLocal);
    tag->putBoolean(NAMED_BY_RESTRICTED_KEY, m_chatCensored);
    tag->putString(OWNER_UUID_KEY, m_uuid);
    if (!raw().empty())
        tag->putString(std::wstring(L"CustomName"), raw());
}

void CensoredString::loadFromTag(CompoundTag* tag)
{
    if (tag->contains(CREATED_ON_HOST_KEY, Tag::TAG_BYTE))
        m_uuidLocal = tag->getBoolean(CREATED_ON_HOST_KEY);
    if (tag->contains(NAMED_BY_RESTRICTED_KEY, Tag::TAG_BYTE))
        m_chatCensored = tag->getBoolean(NAMED_BY_RESTRICTED_KEY);
    if (tag->contains(OWNER_UUID_KEY, Tag::TAG_STRING))
        m_uuid = tag->getString(OWNER_UUID_KEY);
    if (tag->contains(std::wstring(L"CustomName"), Tag::TAG_STRING))
        set(tag->getString(std::wstring(L"CustomName")), m_uuid, m_chatCensored, m_uuidLocal);
}

std::wstring CensoredString::get()
{
    if (m_useLocalization)
    {
        if ((m_flags & 1) > 0)
            return getPendingString();
        if ((m_flags & 2) > 0)
            return getCensoredString();
    }
    return m_pending;
}
