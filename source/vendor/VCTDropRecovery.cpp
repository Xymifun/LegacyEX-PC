#include "vendor/VCTDropRecovery.hpp"

#include "nn/pia/chat/AudioStream.hpp"
#include "nn/pia/chat/CodecFacade.hpp"
#include "vendor/VCTAudio.hpp"
#include "vendor/VCTQueue.hpp"

#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace
{
const unsigned int DROP_RECOVER_STREAM_MAX = 15;
const unsigned int DROP_RECOVER_FRAME_SAMPLE_MAX = 1536;
const unsigned int NOISE_SCALE_PERCENT_LIMIT = 801;

unsigned int s_maxNoiseAmplitude = 2400;
short s_noiseScalePercent = 100;
unsigned char s_continuousDropFadeLength = 2;
unsigned char s_dropRecoverMode = 4;
unsigned char s_interpolateSampleNum = 0x10;
unsigned char s_copyTailUnitLength = 0x20;

short s_previousFrame[DROP_RECOVER_STREAM_MAX][DROP_RECOVER_FRAME_SAMPLE_MAX];

const short s_pinkNoiseTable[DROP_RECOVER_FRAME_SAMPLE_MAX] = {
    26551,  -24066, 32575,  -27847, -18665, -37,    15067,  18606,  19515,  3147,   7792,   -5437,  -30742, -27840,
    24557,  21800,  -14058, 32408,  -11534, -10171, 5173,   6959,   -3013,  24257,  -25144, -14833, -10673, 12660,
    -31706, -535,   23400,  -2700,  17543,  5339,   26789,  -16270, -18599, -406,   -401,   24412,  -24642, -2635,
    25128,  11065,  6754,   -2054,  -15078, 2109,   30599,  20303,  5277,   27439,  19530,  -22112, -8310,  -32514,
    -14537, 30694,  31967,  -11106, -27635, -28246, -8268,  3878,   -9519,  22713,  -15267, -20945, 9750,   -9944,
    8926,   -13210, 15502,  -24181, 4759,   22642,  14797,  20335,  -29911, 22028,  29278,  3104,   -20825, 1372,
    -4343,  30105,  -30463, 3217,   -29855, -20536, 28851,  -59,    914,    -26109, 29868,  -7408,  32107,  25805,
    -31875, -32023, 22698,  972,    14275,  -16701, 21127,  14340,  30165,  8677,   18564,  11866,  18097,  26252,
    9843,   -4538,  -25563, -16574, -13368, 5333,   -5575,  629,    -3041,  -26646, -15901, 5218,   28431,  -29267,
    -10004, 11045,  13104,  13392,  11038,  -14057, 6250,   -18929, 8371,   -30357, 17792,  25948,  -6660,  6923,
    -25378, 28016,  24653,  1462,   -19298, 13045,  27695,  5462,   9695,   -7923,  20680,  -30815, 16200,  -8570,
    -27515, 22511,  26330,  10077,  -24822, 17760,  -18696, -14725, -11174, 28465,  -23888, -5209,  5448,   -23721,
    -21166, 22985,  -11999, -17571, -32462, 10602,  5266,   -23677, -3657,  7016,   16864,  -6123,  -19288, -2059,
    24136,  -6901,  -8521,  -18990, 8370,   -3101,  28493,  26063,  11676,  13242,  30615,  7812,   11334,  -14415,
    -22318, 25676,  -24294, -22658, 21970,  -13948, -18082, 24143,  13272,  -15614, 29722,  21479,  26260,  -4905,
    -11185, 31731,  -23687, 15174,  2128,   -30802, 30784,  -11256, -19836, -2636,  190,    -29767, -16576, -25559,
    -26646, 17316,  29691,  -30451, 20520,  -23623, -23616, 21720,  16090,  -4925,  13512,  17245,  -6358,  -8467,
    25469,  -12578, 20497,  -3976,  28305,  31082,  -28245, 20666,  -26502, 126,    -6722,  29533,  -21999, 20187,
    -30841, 8146,   25223,  -14170, -4410,  16660,  -32621, -12867, 12811,  -3676,  20689,  -29088, -25532, 32702,
    -4739,  -29186, -16001, 26082,  -25774, 28766,  -24828, 29571,  -4157,  -4877,  -28010, -24054, -21413, 1144,
    13574,  21005,  31599,  -11053, 25597,  -4907,  -1521,  -22744, 1795,   9739,   -17628, 5177,   23508,  22979,
    12263,  23305,  -10780, -15696, -24177, 17832,  -8914,  -31586, -22479, 14431,  -1981,  -14541, -13575, 18178,
    11665,  23785,  13707,  27792,  -22540, -19890, -20805, 16655,  -5509,  373,    15467,  -13913, 8059,   1472,
    8388,   16123,  20185,  -21549, 7321,   -6705,  -30828, 31182,  8984,   21974,  18575,  -21547, -5453,  -13605,
    -19287, 16572,  1867,   23571,  24054,  29460,  31141,  13761,  -19363, 23127,  -8182,  12555,  25431,  -20368,
    -19400, 26139,  11012,  -2194,  17039,  -27526, 115,    31622,  18048,  32167,  -32469, -4782,  22577,  -29185,
    1532,   -28609, -26770, 28615,  -7924,  14681,  16340,  25757,  -27876, -26047, 4671,   -32510, 1049,   -27549,
    -16525, 9789,   2794,   -28550, -16375, 22142,  -12167, 5705,   17819,  -16595, -9734,  8345,   12595,  -27251,
    -14717, -5774,  15089,  -24167, 3020,   10901,  -18692, 4894,   15941,  3650,   -23638, -744,   -20390, 19495,
    -4878,  -27831, 29953,  11942,  4367,   -31201, 4492,   31819,  -28039, 6342,   27309,  -32371, -7422,  -12916,
    -18234, -18749, -19380, -15243, -28814, -16792, -9870,  10270,  17494,  -18543, 9715,   17622,  16684,  8365,
    -2586,  9721,   3336,   -14719, 19283,  -10864, 6381,   31579,  -21255, -26073, -25706, -28002, 12831,  -12563,
    2172,   23662,  26584,  12815,  -4562,  7608,   -1185,  -15015, 18384,  16489,  21960,  -25338, -29195, -18456,
    -27409, 14567,  4761,   -10795, -29282, 17340,  29824,  2819,   -15617, -11057, 13749,  20665,  32579,  -28723,
    -16034, 13863,  31124,  16103,  27837,  -12760, -19143, -15567, 16266,  -1738,  -31225, 30512,  -28808, -14415,
    -6641,  24520,  -17667, -4013,  -30588, -14517, -28828, -26898, 21995,  7896,   9752,   -8344,  15753,  16892,
    17713,  2296,   -30709, 30856,  -28868, 3489,   5999,   -11599, 3655,   495,    16441,  -17562, -6603,  -20239,
    10203,  -8972,  14910,  12405,  -11172, -19490, -28216, 27018,  22858,  27639,  18532,  -11981, -12825, -7227,
    -10965, -12953, -1906,  -3565,  -2758,  -32175, -9371,  18899,  1417,   963,    -29111, -26561, 6132,   9982,
    4596,   -18137, -28796, 28833,  2163,   7577,   20477,  -9488,  32456,  -16195, -13138, 28741,  13275,  18149,
    -32141, -2598,  2215,   8953,   -22259, 8887,   -31326, -8696,  10149,  7929,   -24213, -8922,  6301,   18245,
    13496,  16778,  26778,  28812,  -14463, -16659, 23759,  10240,  -23703, -31716, 3211,   -21674, 22380,  -28193,
    6661,   -27331, 6351,   7181,   13926,  31260,  -21179, -12049, 15639,  -3821,  -13730, 8916,   -3507,  -15060,
    5683,   -4106,  -20271, 19168,  13654,  9356,   -27885, 5585,   -28169, 13103,  -7312,  -30010, -14253, 32625,
    -8254,  2529,   -7071,  5819,   7209,   -21208, -9026,  21350,  4376,   -10445, -2343,  17948,  -24412, 3631,
    89,     369,    -24824, -8417,  -30248, 9148,   -21893, -15029, -28997, 15163,  -8651,  20997,  13933,  -8089,
    11453,  4264,   -25912, -23917, 28024,  -7477,  -16452, 8906,   26991,  -12096, 10302,  -1814,  29536,  -30124,
    -19335, 16123,  22280,  5063,   -19153, -8490,  -21585, 23307,  24686,  -525,   -10765, -20163, 4988,   9103,
    17623,  25697,  3581,   -2805,  -18928, -2718,  -28062, -23432, -29306, -22967, -26730, 28035,  -29647, 23738,
    30697,  -17266, 29205,  5303,   18750,  -9213,  22429,  24464,  -13708, 17400,  -13798, -21719, 4625,   -6129,
    -2157,  19823,  27189,  8066,   30165,  -28644, 1470,   914,    28862,  14505,  31415,  24599,  19395,  18579,
    11782,  -28063, 708,    -3629,  7254,   -3508,  -10734, -1129,  -21438, -24149, 3515,   4931,   30145,  -8748,
    -8160,  -14928, 10967,  12168,  -14415, 7322,   3452,   5386,   -11259, 7277,   -24886, 631,    -15838, -25033,
    29538,  -11156, -6135,  16867,  22823,  4310,   16258,  29885,  14877,  -28132, 7210,   -14295, 26777,  30826,
    22714,  5272,   16558,  32094,  1946,   9430,   19161,  1497,   -3418,  -6795,  19434,  -29152, 30269,  21645,
    -4554,  19505,  -31853, -21054, -4102,  9323,   -10705, -29071, 783,    19552,  -22331, 3637,   -13864, 14324,
    30009,  1957,   -25248, -21316, -29709, 3400,   2260,   6032,   -29087, -14632, 9773,   -32646, 6345,   -19784,
    18454,  -18245, 10072,  -30794, -25317, -24776, 615,    -19708, -18411, -26865, -16884, -20877, 10711,  -29579,
    -26399, 10993,  -26596, 9167,   -14825, 19269,  -597,   -4331,  28347,  27401,  -52,    -15005, -2485,  11753,
    -31506, -26400, -12683, -11493, -1895,  26322,  26157,  2332,   -5998,  7681,   -6,     -16973, -6324,  -26802,
    31608,  -23006, -8298,  -885,   24111,  31656,  17793,  14755,  -22631, 10304,  -12141, 21486,  15469,  24469,
    -21718, -15411, -2767,  24643,  17201,  11396,  -25602, -9899,  10248,  3680,   26003,  12281,  -24142, 20233,
    -20063, -24183, -4808,  -19424, 7757,   -585,   19043,  -21476, 15113,  -2795,  18082,  361,    26265,  14286,
    -3792,  21440,  -23456, -8029,  -7031,  29637,  29929,  -6112,  20752,  -800,   -578,   -25568, 29951,  -13987,
    3172,   22406,  11666,  7364,   -16203, -2499,  -29328, -24451, -5534,  -16024, -23462, -29024, 26054,  -12307,
    -7147,  -22759, 1007,   5928,   -12645, -13564, -13845, -22549, -9856,  23109,  -27824, -11339, -19806, -20630,
    25612,  13874,  21012,  21949,  -2697,  -8453,  -31411, -10834, 19084,  6021,   -8307,  13150,  22413,  -32402,
    15245,  31629,  2237,   -30487, 14962,  -1366,  13427,  26500,  27627,  -1488,  17510,  13092,  2983,   8261,
    -24007, -19141, -32767, -12901, 13122,  1605,   1504,   5500,   28578,  10637,  13816,  11321,  -8061,  9501,
    20260,  26555,  4522,   20993,  29209,  11870,  16967,  -7999,  14929,  28692,  24829,  -12771, 24632,  -31300,
    7979,   -15351, 20631,  -27970, 10109,  -23830, -6848,  -15126, -15432, -20719, -21766, 3728,   32191,  -24968,
    24749,  14051,  -30739, 699,    -25646, 23691,  -1905,  -13478, -16875, 18057,  9405,   -9171,  -22452, 8016,
    -22938, -3033,  -10596, 30240,  -9839,  24653,  -11800, -23899, -8969,  14073,  8827,   -6449,  25027,  5963,
    12281,  -5389,  -10584, -22287, -17787, 28705,  -17884, 22551,  -9940,  21703,  8209,   2479,   -22120, 18311,
    -18826, -15387, 7415,   -15274, -10877, -31011, -29140, 14293,  26351,  -11858, 3375,   25843,  -20356, -28765,
    -29748, 26818,  19901,  27054,  22392,  -1894,  -6863,  -18383, -29547, -18836, 25727,  -28438, 28360,  -18584,
    -12274, -21190, -11590, -26415, -3640,  9377,   1262,   -16610, 29485,  14196,  -9726,  -25772, 11934,  -23072,
    -11255, 4769,   -15388, 20252,  -30394, 25661,  -17409, 5362,   -18419, 10622,  -14283, 16239,  -19466, 19514,
    -6056,  -8982,  6210,   3938,   -31655, -9678,  -5229,  -31268, -15090, -12518, -24376, 5021,   -18369, -17623,
    -24268, -10990, 3062,   11931,  19465,  2215,   27911,  -25696, -26876, -10569, -11041, -13362, -17328, 5017,
    21129,  -18969, -24128, -10233, 6917,   12239,  -6862,  2992,   -4915,  14542,  -17860, 15904,  4765,   -10234,
    1306,   27088,  -28274, -13520, 24283,  32136,  20047,  -32358, -4404,  -14338, 13760,  8036,   17692,  -24153,
    -19274, -31495, -23846, -3870,  2824,   -26932, 23307,  25670,  17887,  16723,  14635,  32587,  -4800,  -2743,
    6346,   -10983, -20271, -26593, 6783,   3500,   -24589, 9409,   8721,   -28367, -18175, 30866,  -30234, -2661,
    -9243,  2209,   14249,  -30074, 27611,  31532,  32320,  -24752, 11400,  24919,  30960,  24435,  9790,   -29394,
    -13365, 24872,  -25206, -16424, -2725,  16955,  27134,  -21477, -25200, -4310,  14405,  -30441, -29203, 28800,
    -31900, -20163, 16111,  23158,  20811,  2447,   9920,   15050,  -344,   11551,  13598,  -24357, -23068, -26428,
    10374,  -25594, -4741,  21269,  -22362, -13245, 18820,  -13132, 20992,  24832,  -896,   -2892,  7905,   -23999,
    25437,  -17049, -9851,  -8207,  22734,  -29359, -31870, -7285,  -17456, 22631,  -8580,  16365,  25970,  23947,
    -32245, 7035,   6639,   -15290, 10982,  20594,  16619,  -26939, 12117,  -112,   28935,  -11243, 17622,  -3871,
    6827,   -26976, 781,    24297,  -8684,  -29974, -12302, 2042,   22682,  12934,  8500,   -24113, 30022,  -20640,
    16114,  2919,   -29547, 4902,   -3170,  -9942,  -8314,  28439,  -10968, 5941,   14824,  8176,   4977,   24071,
    -14358, 4658,   -2060,  -27309, 27353,  -1383,  -32083, 283,    6463,   -16680, -12211, 29971,  5663,   -30056,
    13727,  -13766, 2953,   19633,  15420,  -3407,  21605,  -26453, -5009,  18397,  381,    10351,  4193,   -4411,
    26058,  31314,  23697,  28323,  13827,  18245,  -22160, -10800, -30876, -12971, -21930, 2254,   14777,  10856,
    -12883, 6070,   -26442, 17209,  12890,  -19003, 16496,  -3435,  -8590,  12095,  -28354, 31950,  -23786, 31118,
    16047,  -31864, 22957,  14744,  -14869, -2652,  26189,  -9124,  -10450, -18554, -21660, 7631,   -352,   -114,
    23200,  -26115, 6233,   -23851, 20265,  17497,  23475,  -12761, -8961,  -7832,  2515,   -6336,  16809,  -8562,
    2931,   -28085, 8746,   9269,   2546,   6038,   -15837, -16855, -12385, -23142, -8129,  30246,  -1216,  11050,
    25574,  -17206, 21222,  -14853, 18470,  31899,  16886,  339,    28635,  -19595, 20083,  24852,  20258,  -4582,
    -28277, -2835,  -27138, 529,    32609,  -4137,  18502,  -24117, -30176, 32724,  -4391,  -22803, 27710,  25070,
    -12804, 12029,  9744,   -4498,  19728,  -6602,  -1392,  -26284, -1153,  7331,   -2198,  8990,   -8484,  -32383,
    -11188, -2626,  -2465,  -24365, 13676,  27090,  8068,   32580,  -25250, -4054,  -14335, -4027,  -26096, 23417,
    6376,   -32161, -1755,  -14758, -30946, 7639,   3961,   -11899, -2821,  -20082, 24564,  13904,  -11967, -30194,
    11658,  -5780,  -8383,  12021,  -15211, 18904,  -27573, -7540,  -30892, 6185,   13910,  8094,   2496,   12797,
    10224,  -20067, -14183, 19900,  3433,   17140,  18342,  12946,  -18629, 32173,  -23724, 10408,  -580,   6726,
    -7828,  -25174, -1531,  30928,  -29390, 14708,  18467,  -24501, 27302,  -8418,  15999,  -16323, 19733,  22057,
    18060,  -28521, 23777,  -26614, -15696, 11209,  22674,  -552,   11635,  16166,  -25314, -13743, 24044,  -21129,
    372,    -13293, 7531,   -36,    -11954, 5142,   31331,  16613,  29494,  -912,   -31588, 7671,   -31271, -1211,
    24212,  27356,  -32628, -501,   -12227, 25508,  8986,   18694,  -14635, 18628};
}

unsigned int vct_calc_energy(const void* pAudio, unsigned int uiSize)
{
    const short* pSamples = static_cast<const short*>(pAudio);
    const unsigned int uiSamples = uiSize >> 1;
    unsigned int uiEnergy = 0;

    for (unsigned int uiSample = 0; uiSample < uiSamples; ++uiSample)
    {
        const short sSample = *pSamples++;
        uiEnergy += sSample * sSample;
    }

    return uiEnergy / uiSamples;
}

void vct_recover_drop_copy_tail(const void* pPrevious, void* pOut, unsigned int uiSize)
{
    const unsigned int uiCopySize = s_copyTailUnitLength * 16;
    if (uiSize < uiCopySize)
    {
        return;
    }

    const unsigned int uiPeriod = uiCopySize >> 1;
    const unsigned int uiSamples = uiSize >> 1;
    const int iTail = static_cast<int>(uiSamples - uiPeriod);
    const short* pSource = static_cast<const short*>(pPrevious);
    const short sLast = pSource[uiSamples - 1];
    short* pDestination = static_cast<short*>(pOut);

    for (unsigned int uiSample = 0; uiSample < uiSamples; ++uiSample)
    {
        const int iOffset = static_cast<int>(uiSample % uiPeriod);
        const int iSource = iTail + iOffset;
        if (iOffset < s_interpolateSampleNum)
        {
            pDestination[uiSample] = static_cast<short>(
                (sLast * (s_interpolateSampleNum - (iOffset + 1)) + pSource[iSource] * (iOffset + 1))
                / s_interpolateSampleNum);
        }
        else
        {
            pDestination[uiSample] = pSource[iSource];
        }
    }
}

void vct_recover_drop_white_noise(const void* pPrevious, void* pOut, unsigned int uiSize)
{
    unsigned int uiAmplitude = static_cast<unsigned int>(sqrt(static_cast<double>(vct_calc_energy(pPrevious, uiSize))));
    if (uiAmplitude > s_maxNoiseAmplitude)
    {
        uiAmplitude = s_maxNoiseAmplitude;
    }
    if (static_cast<unsigned int>(s_noiseScalePercent) < NOISE_SCALE_PERCENT_LIMIT)
    {
        uiAmplitude = uiAmplitude * s_noiseScalePercent / 100;
    }

    const unsigned int uiSamples = uiSize >> 1;
    short* pDestination = static_cast<short*>(pOut);
    for (unsigned int uiSample = 0; uiSample < uiSamples; ++uiSample)
    {
        const int iNoise = 0x7fff - rand() % 0xffff;
        pDestination[uiSample] = static_cast<short>((uiAmplitude * iNoise) >> 15);
    }
}

void vct_recover_drop_pink_noise(const void* pPrevious, void* pOut, unsigned int uiSize)
{
    unsigned int uiAmplitude = static_cast<unsigned int>(sqrt(static_cast<double>(vct_calc_energy(pPrevious, uiSize))));
    if (uiAmplitude > s_maxNoiseAmplitude)
    {
        uiAmplitude = s_maxNoiseAmplitude;
    }
    if (static_cast<unsigned int>(s_noiseScalePercent) < NOISE_SCALE_PERCENT_LIMIT)
    {
        uiAmplitude = uiAmplitude * s_noiseScalePercent / 100;
    }

    unsigned int uiPosition = rand() % 0x5b80;
    const unsigned int uiSamples = uiSize >> 1;
    short* pDestination = static_cast<short*>(pOut);
    for (unsigned int uiSample = 0; uiSample < uiSamples; ++uiSample)
    {
        const unsigned int uiIndex = uiPosition % DROP_RECOVER_FRAME_SAMPLE_MAX;
        ++uiPosition;
        pDestination[uiSample] = static_cast<short>((uiAmplitude * s_pinkNoiseTable[uiIndex]) >> 15);
    }
}

void vct_recover_drop(const void* pPrevious, void* pOut, unsigned int uiSize)
{
    switch (s_dropRecoverMode)
    {
    case 0:
        memset(pOut, 0, uiSize);
        return;
    case 1:
        memcpy(pOut, pPrevious, uiSize);
        return;
    case 2:
        vct_recover_drop_copy_tail(pPrevious, pOut, uiSize);
        return;
    case 3:
        vct_recover_drop_white_noise(pPrevious, pOut, uiSize);
        return;
    case 4:
        vct_recover_drop_pink_noise(pPrevious, pOut, uiSize);
        return;
    default:
        return;
    }
}

int vct_recover_drop_using_spare(nn::pia::chat::AudioStream* pStream, const _udp_packet* pPacket, void* pOut,
                                 unsigned int)
{
    const unsigned char ucFlags = pPacket->m_Flags;
    if ((ucFlags & VCT_PACKET_FLAG_SPARE) == 0)
    {
        return 0;
    }
    if (pStream->m_LastSequence + 2 != pPacket->m_Sequence)
    {
        return 0;
    }

    const VCTAudioFrame* pFrame = pPacket->m_pFrame;
    const int iPacketSize = vct_get_packet_size(pFrame->m_SparePacket.m_Codec);
    pStream->m_LastSequence = pPacket->m_Sequence - 1;
    nn::pia::chat::g_codecFacade->Decode(pFrame->m_SparePacket.m_Data, static_cast<short*>(pOut),
                                         static_cast<unsigned int>(iPacketSize - 12),
                                         static_cast<VCTCodec>(pFrame->m_SparePacket.m_Codec));
    ++pStream->m_Info.m_SpareRecoverCount;
    return 1;
}

void vct_filter_fade_in(const short* pSrc, short* pDst, unsigned int uiCount, unsigned int uiFadeLength,
                        short sPrevious)
{
    if (uiFadeLength == 0)
    {
        return;
    }
    if (uiFadeLength > uiCount)
    {
        uiFadeLength = uiCount;
    }

    for (unsigned int uiSample = 0; uiSample < uiFadeLength; ++uiSample)
    {
        const int iWeight = static_cast<int>(uiSample + 1);
        pDst[uiSample] = static_cast<short>(
            (sPrevious * static_cast<int>(uiFadeLength - iWeight) + pSrc[uiSample] * iWeight)
            / static_cast<int>(uiFadeLength));
    }
}

void vct_filter_fade_rate(const short* pSrc, short* pDst, unsigned int uiCount, float fStartGain, float fEndGain)
{
    const float fStep = (fEndGain - fStartGain) / static_cast<float>(uiCount);
    float fGain = fStartGain;

    for (unsigned int uiSample = 0; uiSample < uiCount; ++uiSample)
    {
        pDst[uiSample] = static_cast<short>(static_cast<float>(pSrc[uiSample]) * fGain);
        fGain += fStep;
    }
}

int vct_recover_drop_fade(const void* pPrevious, void* pOut, unsigned int uiSize, unsigned int uiStreamIndex)
{
    nn::pia::chat::AudioStream* pStream = vct_get_audio_stream(uiStreamIndex);
    short* pDestination = static_cast<short*>(pOut);

    if (pStream->m_Info.m_ContinuousDropCount == 0)
    {
        if (pPrevious != 0)
        {
            vct_recover_drop(pPrevious, s_previousFrame[uiStreamIndex], uiSize);
            memcpy(pOut, s_previousFrame[uiStreamIndex], uiSize);
            vct_filter_fade_in(pDestination, pDestination, uiSize >> 1, s_interpolateSampleNum,
                               static_cast<const short*>(pPrevious)[(uiSize >> 1) - 1]);

            pStream->m_LastRecoveredSample = pDestination[(uiSize >> 1) - 1];
            pStream->m_DropRecovered = 1;
            pStream->m_Info.m_ContinuousDropCount++;
            return 1;
        }
    }
    else
    {
        const unsigned int uiFadeLength = s_continuousDropFadeLength;
        if (uiFadeLength >= 6)
        {
            const unsigned char ucRamp = static_cast<unsigned char>(uiFadeLength - 5);
            const int iStep = static_cast<int>(pStream->m_Info.m_ContinuousDropCount - 1);
            if (iStep < ucRamp)
            {
                const float fRamp = ucRamp;
                vct_filter_fade_rate(s_previousFrame[uiStreamIndex], pDestination, uiSize >> 1, 1.0f - iStep / fRamp,
                                     1.0f - (iStep + 1) / fRamp);

                pStream->m_LastRecoveredSample = pDestination[(uiSize >> 1) - 1];
                pStream->m_DropRecovered = 1;
                pStream->m_Info.m_ContinuousDropCount++;
                return 1;
            }
        }
        else if (pStream->m_Info.m_ContinuousDropCount < uiFadeLength)
        {
            const short* pFrame = s_previousFrame[uiStreamIndex];
            const unsigned int uiSamples = uiSize >> 1;
            for (unsigned int uiSample = 0; uiSample < uiSamples; ++uiSample)
            {
                const short sSample = pFrame[uiSample];
                pDestination[uiSample] = sSample
                                         - static_cast<int>(sSample * pStream->m_Info.m_ContinuousDropCount)
                                               / s_continuousDropFadeLength;
            }

            pStream->m_LastRecoveredSample = pDestination[uiSamples - 1];
            pStream->m_DropRecovered = 1;
            pStream->m_Info.m_ContinuousDropCount++;
            return 1;
        }

        memset(pOut, 0, uiSize);
        if (++pStream->m_Info.m_ContinuousDropCount == 0)
        {
            pStream->m_Info.m_ContinuousDropCount = s_continuousDropFadeLength;
        }
    }
    return 0;
}

int vct_recover_drop_stream(nn::pia::chat::AudioStream* pStream, void* pOut, unsigned int uiSize)
{
    int iRecovered;

    if (pStream->m_PreviousPacket == 0)
    {
        iRecovered = vct_recover_drop_fade(0, pOut, uiSize, pStream->m_StreamIndex);
    }
    else
    {
        iRecovered = vct_recover_drop_fade(pStream->m_PreviousPacket->m_pFrame, pOut, uiSize, pStream->m_StreamIndex);
        vct_free_packet_buffer(pStream->m_PreviousPacket);
        pStream->m_PreviousPacket = 0;
    }

    if (iRecovered == 0)
    {
        return 0;
    }
    pStream->m_Info.m_FadeRecoverCount = pStream->m_Info.m_FadeRecoverCount + 1;
    return 1;
}

void VCT_SetDropRecoverContinuousDropFadeLength(unsigned char ucLength)
{
    if (ucLength == 0)
    {
        return;
    }
    s_continuousDropFadeLength = ucLength;
}

void VCT_SetDropRecoverMode(unsigned char ucMode)
{
    s_dropRecoverMode = ucMode;
}

unsigned char VCT_GetDropRecoverInterpolateSampleNum()
{
    return s_interpolateSampleNum;
}
