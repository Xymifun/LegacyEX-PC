#ifndef FFLI_SYSTEM_H_
#define FFLI_SYSTEM_H_

#include <nn/ffl/types.h>

struct FFLiAuthorID;

#ifdef __cplusplus
extern "C"
{
#endif

const FFLiAuthorID* FFLiGetUserID();

#ifdef __cplusplus
}
#endif

#endif // FFLI_SYSTEM_H_
