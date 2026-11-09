#include "vendor/_ConnectionProgressParams.hpp"

#include <cstddef>

_ConnectionProgressParams::_ConnectionProgressParams()
    : m_playerId(0),
      m_stringId(-1),
      m_showTooltips(false),
      m_setFailTimer(false),
      m_timerTime(0),
      m_cancelFunc(NULL),
      m_cancelFuncParam(NULL),
      m_miniGameId(eMiniGameId_Any)
{
}
