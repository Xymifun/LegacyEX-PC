#include "platform/pc/PcMusicTest.hpp"

#include "platform/pc/PcOs.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "vendor/MilesApi.hpp"

#include <stdio.h>
#include <string.h>

extern _DIG_DRIVER* g_pcMilesDriver;
extern "C" const char* AIL_last_error();

namespace
{
const char* const TRACKS[] = {
    "music/music/calm1", "music/music/calm2", "music/music/calm3", "music/music/hal1", "music/music/hal2", "music/music/hal3", "music/music/hal4",
    "music/music/nuance1", "music/music/nuance2", "music/music/piano1", "music/music/piano2", "music/music/piano3", "music/music/nether1",
    "music/music/nether2", "music/music/nether3", "music/music/nether4", "music/music/the_end_dragon_alive", "music/music/the_end_end",
    "music/music/creative1", "music/music/creative2", "music/music/creative3", "music/music/creative4", "music/music/creative5",
    "music/music/creative6", "music/music/menu1", "music/music/menu2", "music/music/menu3", "music/music/menu4", "music/music/MG01_01",
    "music/music/MG01_02", "music/music/MG01_03", "music/music/MG01_04", "music/music/MG02_01", "music/music/MG02_02", "music/music/MG02_03",
    "music/music/MG02_04", "music/music/MG02_05", "music/music/MG02_06", "music/music/MG02_07", "music/music/MG02_08", "music/music/MG02_09",
    "music/music/MG02_10", "music/music/MG03_Vanilla_Map1Track1", "music/music/MG03_Vanilla_Map2Track1",
    "music/music/MG03_Vanilla_Map3Track1", "music/music/MG03_Vanilla_Map4Track1", "music/music/MG03_Vanilla_Map5Track1",
    "music/music/MG03_Vanilla_Map6Track1", "music/music/MG03_Vanilla_Map7Track1", "music/music/MG03_Vanilla_Pack2TrackBody1",
    "music/music/MG03_Vanilla_Pack2TrackBody2", "music/music/MG03_Vanilla_Pack2TrackBody3", "music/music/MG03_Vanilla_Canyon1",
    "music/music/MG03_Vanilla_Canyon2", "music/music/MG03_Vanilla_Canyon3", "music/music/MG03_Plastic_Pack2TrackShrunk1",
    "music/music/MG03_Plastic_Pack2TrackShrunk2", "music/music/MG03_Plastic_Pack2TrackShrunk3",
    "music/music/MG03_Vanilla_Pack3_Celts1", "music/music/MG03_Vanilla_Pack3_Celts2", "music/music/MG03_Fantasy_Pack3_Excalibur1",
    "music/music/MG03_Fantasy_Pack3_Excalibur2", "music/music/Fantasy_BornToArms", "music/music/Fantasy_Crusaders",
    "music/music/Fantasy_Hastilude", "music/music/City_8BitsPiecesChunks", "music/music/City_BeatAroundTheBlock",
    "music/music/City_FromDistrictToDistrict", "music/music/City_IndustrialQuarter", "music/music/City_PrecinctsAtNight",
    "music/music/Steampunk_AirshipAdventurers", "music/music/Steampunk_ClockworkCrafter", "music/music/Steampunk_MagnificentMachines",
    "music/music/Steampunk_SteamingSuperstructures", "music/music/MG01_Plastic_Bedroom_ToysOnATear",
    "music/music/MG01_Plastic_Bedroom_DanceOTBlocks", "music/music/MG01_Plastic_Shrunk_MasterBuilders",
    "music/music/MG01_Vanilla_Western_DesertDuel", "music/music/MG01_Vanilla_Western_SunsetRiders",
    "music/music/MG01_Vanilla_Western_TheOutlaw", "music/cds/11", "music/cds/13", "music/cds/blocks", "music/cds/cat",
    "music/cds/chirp", "music/cds/far", "music/cds/mall", "music/cds/mellohi", "music/cds/stal",
    "music/cds/strad", "music/cds/ward", "music/cds/where_are_we_now",
};
const unsigned int PLAY_MILLISECONDS = 2000;

int TestThread(void*)
{
    while (g_pcMilesDriver == 0)
        PcOsSleepMilliseconds(500);
    PcOsSleepMilliseconds(5000);
    unsigned int failed = 0;
    for (unsigned int i = 0; i < sizeof(TRACKS) / sizeof(TRACKS[0]); ++i)
    {
        char path[128];
        snprintf(path, sizeof(path), "%s.binka", TRACKS[i]);
        _STREAM* streams[2] = {0, 0};
        const char* err = 0;
        for (int which = 0; which < 2 && err == 0; ++which)
        {
            streams[which] = AIL_open_stream(g_pcMilesDriver, path, 0);
            if (streams[which] == 0)
                err = AIL_last_error();
        }
        int positions[2] = {0, 0};
        int statuses[2] = {0, 0};
        if (err == 0)
        {
            for (int which = 0; which < 2; ++which)
            {
                AIL_set_sample_volume_levels(AIL_stream_sample_handle(streams[which]), 0.05f, 0.05f);
                AIL_start_stream(streams[which]);
            }
            PcOsSleepMilliseconds(PLAY_MILLISECONDS);
            for (int which = 0; which < 2; ++which)
            {
                positions[which] = AIL_stream_position(streams[which]);
                statuses[which] = AIL_stream_status(streams[which]);
            }
        }
        for (int which = 0; which < 2; ++which)
        {
            if (streams[which] != 0)
                AIL_close_stream(streams[which]);
        }
        const bool ok = err == 0 && positions[0] > 0 && positions[1] > 0;
        failed += ok ? 0 : 1;
        printf("[pc] music-test %-4s %s: %s (position %d/%d, status %d/%d)\n", ok ? "ok" : "FAIL", path,
               err != 0 ? err : "opened", positions[0], positions[1], statuses[0], statuses[1]);
        fflush(stdout);
    }
    printf("[pc] music-test done: %u of %u failed\n", failed,
           (unsigned int)(sizeof(TRACKS) / sizeof(TRACKS[0])));
    fflush(stdout);
    return 0;
}
}

void PcMusicTestStart()
{
    C4JThreadImpl* t = new C4JThreadImpl(TestThread, 0, "MusicTest", 0);
    t->SetDeleteOnExit(true);
    t->Run();
}
