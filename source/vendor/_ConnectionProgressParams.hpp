#ifndef VENDOR__CONNECTIONPROGRESSPARAMS_HPP
#define VENDOR__CONNECTIONPROGRESSPARAMS_HPP

#include "game/minigame/EMiniGameId.hpp"

class _ConnectionProgressParams
{
public:
    _ConnectionProgressParams();

    int m_playerId;
    int m_stringId;
    bool m_showTooltips;
    bool m_setFailTimer;
    int m_timerTime;
    void (*m_cancelFunc)(void*);
    void* m_cancelFuncParam;
    EMiniGameId m_miniGameId;
};

#endif
