#ifndef GAME_CORE_CRITICALSECTION_HPP
#define GAME_CORE_CRITICALSECTION_HPP

#include <coreinit/fastmutex.h>

void InitializeCriticalSection(OSFastMutex* pCriticalSection);
void InitializeCriticalSectionAndSpinCount(OSFastMutex* pCriticalSection, unsigned long spinCount);
void EnterCriticalSection(OSFastMutex* pCriticalSection);
int TryEnterCriticalSection(OSFastMutex* pCriticalSection);
void LeaveCriticalSection(OSFastMutex* pCriticalSection);
void DeleteCriticalSection(OSFastMutex* pCriticalSection);

#endif
