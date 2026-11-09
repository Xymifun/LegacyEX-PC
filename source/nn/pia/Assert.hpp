#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_ASSERT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_ASSERT_HPP

#include <cstddef>

namespace nn { namespace pia {

void Assert(const char* condition, const char* fileName, unsigned int line, const char* format, ...);

}}

#if defined(PIA_DISABLE_ASSERT)
#define PIA_ASSERT(exp) ((void)0)
#else
#define PIA_ASSERT(exp) ((exp) ? (void)0 : ::nn::pia::Assert(#exp, __BASE__, __LINE__, NULL))
#endif

#endif
