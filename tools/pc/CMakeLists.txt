cmake_minimum_required(VERSION 3.20)
project(MinecraftPC C CXX)

option(PC_WUT_TYPES "Take Cafe OS type declarations from wut's headers until the PC compat layer replaces them" ON)
option(PC_POINTER_WARNINGS "Report only conversions that cut a pointer or a 64-bit value to 32 bits (finds 64-bit build bugs)" OFF)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(SOURCE_ROOT "${REPO_ROOT}/source")

add_compile_options("-fmacro-prefix-map=${REPO_ROOT}/=")
set(DEPS "${REPO_ROOT}/build/deps" CACHE PATH "Directory tools/build/deps.py fills")

foreach(required boost/boost/shared_ptr.hpp zlib/zlib.h libpng/png.c stb/stb_image.h curl/include/curl/curl.h lua/lua.h opus/include/opus.h)
    if(NOT EXISTS "${DEPS}/${required}")
        message(FATAL_ERROR "${DEPS}/${required} is missing; run: python tools/build/deps.py build/deps")
    endif()
endforeach()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PC_64BIT ON)
else()
    set(PC_64BIT OFF)
endif()

set(ARCH_FLAGS)
if(NOT WIN32 AND NOT PC_64BIT)
    list(APPEND ARCH_FLAGS -m32 -malign-double)
endif()
add_compile_options(${ARCH_FLAGS} -fsigned-char -ffunction-sections -fdata-sections -fno-omit-frame-pointer)
add_link_options(${ARCH_FLAGS})

file(GLOB ZLIB_SOURCES "${DEPS}/zlib/*.c")
list(FILTER ZLIB_SOURCES EXCLUDE REGEX "/(example|minigzip|gz[a-z]*)\\.c$")
add_library(pc_zlib STATIC ${ZLIB_SOURCES})
target_include_directories(pc_zlib PUBLIC "${DEPS}/zlib")
target_compile_options(pc_zlib PRIVATE -w)

set(PNG_NAMES png pngerror pngget pngmem pngpread pngread pngrio pngrtran pngrutil pngset pngtrans pngwio
    pngwtran pngwutil)
list(TRANSFORM PNG_NAMES PREPEND "${DEPS}/libpng/")
list(TRANSFORM PNG_NAMES APPEND ".c")
set(PNG_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/libpng")
configure_file("${DEPS}/libpng/scripts/pnglibconf.h.prebuilt" "${PNG_CONFIG_DIR}/pnglibconf.h" COPYONLY)
foreach(header pngconf.h pngdebug.h pnginfo.h pngpriv.h pngstruct.h)
    configure_file("${DEPS}/libpng/${header}" "${PNG_CONFIG_DIR}/${header}" COPYONLY)
endforeach()
configure_file("${REPO_ROOT}/tools/build/overrides/libpng/png.h" "${PNG_CONFIG_DIR}/png.h" COPYONLY)
add_library(pc_png STATIC ${PNG_NAMES} "${REPO_ROOT}/tools/build/overrides/libpng/pngwrite.c")
target_include_directories(pc_png PUBLIC "${PNG_CONFIG_DIR}" PRIVATE "${DEPS}/libpng")
target_link_libraries(pc_png PUBLIC pc_zlib)
target_compile_options(pc_png PRIVATE -w)

file(GLOB LUA_SOURCES "${DEPS}/lua/*.c")
list(FILTER LUA_SOURCES EXCLUDE REGEX "/(lua|luac|liolib|loadlib|ldblib|linit)\\.c$")
add_library(pc_lua STATIC ${LUA_SOURCES})
target_include_directories(pc_lua SYSTEM PUBLIC "${DEPS}/lua")
target_compile_options(pc_lua PRIVATE -w)
if(NOT WIN32)
    target_compile_definitions(pc_lua PRIVATE LUA_USE_POSIX)
endif()

function(pc_opus_sources list file variable)
    file(READ "${DEPS}/opus/${file}" text)
    string(REGEX MATCH "${variable} =([^\n]*\\\\\n)*[^\n]*" found "${text}")
    string(REGEX REPLACE "^${variable} =" "" found "${found}")
    string(REGEX REPLACE "[\\\\\n\t]" " " found "${found}")
    separate_arguments(found)
    list(TRANSFORM found PREPEND "${DEPS}/opus/")
    set(${list} ${${list}} ${found} PARENT_SCOPE)
endfunction()
set(OPUS_SOURCES)
pc_opus_sources(OPUS_SOURCES celt_sources.mk CELT_SOURCES)
pc_opus_sources(OPUS_SOURCES silk_sources.mk SILK_SOURCES)
pc_opus_sources(OPUS_SOURCES silk_sources.mk SILK_SOURCES_FLOAT)
pc_opus_sources(OPUS_SOURCES opus_sources.mk OPUS_SOURCES)
pc_opus_sources(OPUS_SOURCES opus_sources.mk OPUS_SOURCES_FLOAT)
add_library(pc_opus STATIC ${OPUS_SOURCES})
target_include_directories(pc_opus SYSTEM PUBLIC "${DEPS}/opus/include")
target_include_directories(pc_opus PRIVATE "${DEPS}/opus" "${DEPS}/opus/celt" "${DEPS}/opus/silk"
    "${DEPS}/opus/silk/float")
target_compile_definitions(pc_opus PRIVATE OPUS_BUILD USE_ALLOCA HAVE_LRINT HAVE_LRINTF)
target_compile_options(pc_opus PRIVATE -w -O2)

add_library(pc_game_flags INTERFACE)
target_include_directories(pc_game_flags INTERFACE
    "${SOURCE_ROOT}/platform/pc/overlay"
    "${SOURCE_ROOT}"
    "${DEPS}/curl/include"
    "${DEPS}/stb"
    "${REPO_ROOT}/tools/build/third_party/rapidxml")
target_include_directories(pc_game_flags SYSTEM INTERFACE "${DEPS}/boost")
if(WIN32)
    target_include_directories(pc_game_flags SYSTEM INTERFACE "${SOURCE_ROOT}/platform/pc/compat/win32")
endif()
if(PC_WUT_TYPES)
    set(WUT_TYPES_DIR "${CMAKE_CURRENT_BINARY_DIR}/wut-types")
    file(GLOB WUT_ENTRIES RELATIVE "${DEPS}/wut/include" "${DEPS}/wut/include/*")
    list(REMOVE_ITEM WUT_ENTRIES sys netinet arpa netdb.h poll.h)
    file(MAKE_DIRECTORY "${WUT_TYPES_DIR}")
    if(PC_64BIT)
        list(REMOVE_ITEM WUT_ENTRIES wut_structsize.h coreinit)
        file(READ "${DEPS}/wut/include/wut_structsize.h" WUT_TEXT)
        string(REPLACE "#if defined(static_assert) || defined(__cplusplus)" "#if 0" WUT_TEXT "${WUT_TEXT}")
        file(WRITE "${WUT_TYPES_DIR}/wut_structsize.h" "${WUT_TEXT}")
        file(MAKE_DIRECTORY "${WUT_TYPES_DIR}/coreinit")
        file(GLOB COREINIT_ENTRIES RELATIVE "${DEPS}/wut/include/coreinit" "${DEPS}/wut/include/coreinit/*")
        list(REMOVE_ITEM COREINIT_ENTRIES filesystem.h)
        foreach(entry ${COREINIT_ENTRIES})
            if(NOT EXISTS "${WUT_TYPES_DIR}/coreinit/${entry}")
                file(CREATE_LINK "${DEPS}/wut/include/coreinit/${entry}" "${WUT_TYPES_DIR}/coreinit/${entry}" SYMBOLIC)
            endif()
        endforeach()
        file(READ "${DEPS}/wut/include/coreinit/filesystem.h" WUT_TEXT)
        string(REPLACE "(uint32_t)client" "(uintptr_t)client" WUT_TEXT "${WUT_TEXT}")
        string(REPLACE "(uint32_t)cmdBlock" "(uintptr_t)cmdBlock" WUT_TEXT "${WUT_TEXT}")
        file(WRITE "${WUT_TYPES_DIR}/coreinit/filesystem.h" "${WUT_TEXT}")
    endif()
    foreach(entry ${WUT_ENTRIES})
        if(NOT EXISTS "${WUT_TYPES_DIR}/${entry}")
            file(CREATE_LINK "${DEPS}/wut/include/${entry}" "${WUT_TYPES_DIR}/${entry}" SYMBOLIC)
        endif()
    endforeach()
    target_include_directories(pc_game_flags SYSTEM INTERFACE "${WUT_TYPES_DIR}")
endif()
target_compile_definitions(pc_game_flags INTERFACE
    MINECRAFT_PC NDEBUG BOOST_SP_NO_SYNC BOOST_ENABLE_ASSERT_HANDLER BOOST_NO_EXCEPTIONS __BASE__=__FILE_NAME__)
target_compile_options(pc_game_flags INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-fshort-wchar -frtti -fno-exceptions -fno-threadsafe-statics -Wno-c++11-narrowing
    -fno-delete-null-pointer-checks>)
if(NOT PC_POINTER_WARNINGS)
    target_compile_options(pc_game_flags INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-w>)
else()
    target_compile_options(pc_game_flags INTERFACE -Wno-everything -Wint-to-pointer-cast -Wvoid-pointer-to-int-cast
        -Wpointer-to-int-cast -Wshorten-64-to-32)
endif()

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "${SOURCE_ROOT}/game/*.cpp")
list(REMOVE_ITEM GAME_SOURCES "${SOURCE_ROOT}/game/core/PIXNamedEvents.cpp")
add_library(mcgame STATIC ${GAME_SOURCES})
target_link_libraries(mcgame PUBLIC pc_game_flags pc_png pc_zlib)

set(PC_SHARED_UNITS
    vendor/_ConnectionProgressParams vendor/_LoadingInputParams vendor/_MapDataMappings vendor/_MapDataMappings_old
    vendor/_NetworkGameInitData vendor/_Polygon vendor/_SaveListDetails vendor/_SkinAdjustments
    vendor/_TutorialPopupInfo vendor/_XPARTY_USER_INFO vendor/_XUSER_SIGNIN_INFO vendor/stb_truetype
    platform/wiiu/FJ_WiiUDRC platform/wiiu/Memory platform/wiiu/PlayerUID platform/wiiu/UIComponent_WiiUDRC
    platform/wiiu/C4JStorage platform/wiiu/CInput platform/wiiu/CProfile platform/wiiu/CStorage
    platform/wiiu/C_AOC platform/wiiu/cCommerceNintendo platform/wiiu/cNintendoDLCInfo platform/wiiu/InputManager
    platform/wiiu/WiiULeaderboardManager platform/wiiu/WiiUMilesFileSystem platform/wiiu/WiiU_Platform
    platform/wiiu/CPlatformNetworkManagerNintendo platform/wiiu/NetworkPlayerNintendo platform/wiiu/NQRNetworkManager
    platform/wiiu/NQRNetworkPlayer
    vendor/BinkADec vendor/BinkAudioDecode vendor/MilesApi vendor/MilesAsync vendor/MilesBank vendor/MilesCoreApi
    vendor/MilesEventError vendor/MilesEventExec vendor/MilesFilters vendor/MilesGenericMss vendor/MilesHandle
    vendor/MilesIO vendor/MilesMarker vendor/MilesMem vendor/MilesMixer vendor/MilesRadSS vendor/MilesRib vendor/MilesRR
    vendor/MilesStream vendor/MilesTime
    nn/nex/StringConversion
    cafe/mtx/mtx cafe/mtx/mtx44 cafe/mtx/mtx44vec cafe/mtx/mtxvec cafe/mtx/quat cafe/mtx/vec)
set(PC_SHARED_SOURCES)
foreach(unit ${PC_SHARED_UNITS})
    list(APPEND PC_SHARED_SOURCES "${SOURCE_ROOT}/${unit}.cpp")
endforeach()

file(GLOB PC_PLATFORM_SOURCES CONFIGURE_DEPENDS "${SOURCE_ROOT}/platform/pc/*.cpp")
add_executable(MinecraftPC ${PC_PLATFORM_SOURCES} ${PC_SHARED_SOURCES})
file(GLOB PC_KEYBOARD_GLYPHS CONFIGURE_DEPENDS "${SOURCE_ROOT}/platform/pc/glyphs/keyboard/*.png")
set(PC_KEYBOARD_GLYPH_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/PcKeyboardGlyphData.cpp")
add_custom_command(OUTPUT "${PC_KEYBOARD_GLYPH_SOURCE}"
    COMMAND "${CMAKE_COMMAND}" "-DFOLDER=${SOURCE_ROOT}/platform/pc/glyphs/keyboard"
            "-DOUTPUT=${PC_KEYBOARD_GLYPH_SOURCE}" -P "${CMAKE_CURRENT_SOURCE_DIR}/embed-glyphs.cmake"
    DEPENDS ${PC_KEYBOARD_GLYPHS} "${CMAKE_CURRENT_SOURCE_DIR}/embed-glyphs.cmake"
    VERBATIM)
target_sources(MinecraftPC PRIVATE "${PC_KEYBOARD_GLYPH_SOURCE}")
file(GLOB_RECURSE PC_CONTENT_FILES CONFIGURE_DEPENDS "${SOURCE_ROOT}/*.cpp" "${SOURCE_ROOT}/*.hpp" "${SOURCE_ROOT}/*.h"
     "${SOURCE_ROOT}/*.c" "${SOURCE_ROOT}/*.inc")
list(FILTER PC_CONTENT_FILES EXCLUDE REGEX "/platform/wiiu/shaders/")
set(PC_CONTENT_VERSION_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/PcContentVersion.cpp")
set(PC_NATIVE_SDK_DIR "${CMAKE_CURRENT_BINARY_DIR}/native-sdk")
file(MAKE_DIRECTORY "${PC_NATIVE_SDK_DIR}")
add_custom_command(OUTPUT "${PC_CONTENT_VERSION_SOURCE}" "${PC_NATIVE_SDK_DIR}/PcGameBuild.h"
    COMMAND "${CMAKE_COMMAND}" "-DROOT=${SOURCE_ROOT}" "-DOUTPUT=${PC_CONTENT_VERSION_SOURCE}"
            "-DHEADER=${PC_NATIVE_SDK_DIR}/PcGameBuild.h"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/content-version.cmake"
    DEPENDS ${PC_CONTENT_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/content-version.cmake"
    VERBATIM)
target_sources(MinecraftPC PRIVATE "${PC_CONTENT_VERSION_SOURCE}")
target_link_libraries(MinecraftPC PRIVATE pc_game_flags -Wl,--whole-archive mcgame -Wl,--no-whole-archive pc_png pc_zlib pc_lua pc_opus)
target_link_options(MinecraftPC PRIVATE -fuse-ld=lld -Wl,--gc-sections -Wl,--error-limit=0)
if(WIN32)
    target_sources(MinecraftPC PRIVATE "${SOURCE_ROOT}/vendor/dlmalloc.cpp")
    enable_language(RC)
    if(PC_64BIT)
        set(CMAKE_RC_FLAGS "--target=pe-x86-64")
    else()
        set(CMAKE_RC_FLAGS "--target=pe-i386")
    endif()
    target_sources(MinecraftPC PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/MinecraftPC.rc")
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/MinecraftPC.rc" PROPERTIES OBJECT_DEPENDS
                                "${CMAKE_CURRENT_SOURCE_DIR}/MinecraftPC.ico")
    set_source_files_properties("${SOURCE_ROOT}/vendor/dlmalloc.cpp" PROPERTIES LANGUAGE C COMPILE_OPTIONS "-x;c;-w")
    if(PC_64BIT)
        set_source_files_properties("${SOURCE_ROOT}/vendor/dlmalloc.cpp" PROPERTIES COMPILE_DEFINITIONS MSPACES=1)
        target_link_options(MinecraftPC PRIVATE -Wl,--image-base=0x10000000 -Wl,--disable-dynamicbase
            -Wl,--disable-high-entropy-va)
    endif()
    target_link_libraries(MinecraftPC PRIVATE -Wl,--start-group stdc++ winpthread -Wl,--end-group ws2_32 gdi32 opengl32 winmm hid setupapi bcrypt ole32)
    target_link_options(MinecraftPC PRIVATE -static)
    if(NOT PC_64BIT)
        target_link_options(MinecraftPC PRIVATE -Wl,--large-address-aware)
    endif()
    add_custom_command(TARGET MinecraftPC POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink MinecraftPC.exe Minecraft.Client.exe
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
else()
    target_link_libraries(MinecraftPC PRIVATE pthread)
endif()

if(WIN32 AND PC_64BIT)
    find_program(PC_NM NAMES llvm-nm REQUIRED)
    find_program(PC_DLLTOOL NAMES llvm-dlltool REQUIRED)
    add_custom_command(OUTPUT "${PC_NATIVE_SDK_DIR}/libpcgame.a" "${PC_NATIVE_SDK_DIR}/pcgame.def"
        COMMAND "${CMAKE_COMMAND}" "-DEXE=$<TARGET_FILE:MinecraftPC>" "-DNM=${PC_NM}" "-DDLLTOOL=${PC_DLLTOOL}"
                "-DMACHINE=i386:x86-64" "-DOUT=${PC_NATIVE_SDK_DIR}" -P "${CMAKE_CURRENT_SOURCE_DIR}/native-sdk.cmake"
        DEPENDS MinecraftPC "${CMAKE_CURRENT_SOURCE_DIR}/native-sdk.cmake"
        VERBATIM)
    add_custom_target(pc_native_sdk DEPENDS "${PC_NATIVE_SDK_DIR}/libpcgame.a")

    function(pc_add_native_mod target)
        add_library(${target} SHARED EXCLUDE_FROM_ALL ${ARGN})
        add_dependencies(${target} pc_native_sdk)
        target_include_directories(${target} PRIVATE "${PC_NATIVE_SDK_DIR}")
        target_link_libraries(${target} PRIVATE "${PC_NATIVE_SDK_DIR}/libpcgame.a" pc_game_flags)
        set_target_properties(${target} PROPERTIES PREFIX "" OUTPUT_NAME win64
                              ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/native-imports/${target}")
        target_link_options(${target} PRIVATE -fuse-ld=lld -static -Wl,--image-base=0x30000000
            -Wl,--disable-high-entropy-va)
    endfunction()

    pc_add_native_mod(pc_native_example "${CMAKE_CURRENT_SOURCE_DIR}/native-example/ExampleMod.cpp")
    set_target_properties(pc_native_example PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/native-example"
                          RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/native-example")

    set(PC_NATIVE_MOD_DIRS "" CACHE STRING "folders of native mods to build with the game (pc_add_native_mod)")
    foreach(dir IN LISTS PC_NATIVE_MOD_DIRS)
        get_filename_component(name "${dir}" NAME)
        add_subdirectory("${dir}" "${CMAKE_CURRENT_BINARY_DIR}/native-mods/${name}")
    endforeach()
endif()
