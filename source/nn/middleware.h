#ifndef NN_MIDDLEWARE_H_
#define NN_MIDDLEWARE_H_

void SetModuleId(...);

#define NN_DEFINE_MIDDLEWARE(name, vendor, module) \
    static const char name[] __attribute__((section(".module_id"))) = "[SDK+" vendor ":" module "]"
#define NN_USING_MIDDLEWARE(name) SetModuleId(name)

#endif
