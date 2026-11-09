#ifndef PLATFORM_WIIU_MIIICON_MAKEICON_HPP
#define PLATFORM_WIIU_MIIICON_MAKEICON_HPP

#include "platform/wiiu/MiiIcon/IconShader.hpp"

#include <cafe/gx2.h>
#include <nn/ffl/FFLCharModel.h>
#include <nn/ffl/FFLDataSource.h>
#include <nn/ffl/FFLStandard.h>

void* CreateModel(FFLCharModel* model, FFLDataSource dataSource, FFLStoreData* storeData, int index);
void* CreateIconTexture(GX2Texture* texture, const FFLCharModel* model);
void* MakeIcon(GX2Texture* texture, IconShaderType shaderType, FFLDataSource dataSource, FFLStoreData* storeData,
               int index);

#endif
