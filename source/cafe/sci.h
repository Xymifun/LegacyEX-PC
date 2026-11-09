#ifndef CAFE_SCI_H
#define CAFE_SCI_H

#ifdef __cplusplus
extern "C"
{
#endif

int SCIGetPlatformRegion(unsigned int* pRegion);
int SCIGetCafeCountry(unsigned int* pCountry);
const char* SCIGetCountryCodeA2(unsigned int uiCountry);

#ifdef __cplusplus
}
#endif

#endif
