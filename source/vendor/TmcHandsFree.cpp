#include "vendor/TmcHandsFree.hpp"

#include <math.h>

namespace
{

enum
{
    kTmcMixSpectrum = 0x0000,
    kTmcResidualSpectrum = 0x0200,
    kTmcPreviousMix = 0x0400,
    kTmcPreviousLeft = 0x0500,
    kTmcPreviousRight = 0x0580,
    kTmcEchoEstimate = 0x0600,
    kTmcOutputFilter = 0x0800,
    kTmcFlags = 0x0828,
    kTmcAgcGainIndex = 0x082A,
    kTmcAgcAttenIndex = 0x082C,
    kTmcEchoMode = 0x082E,
    kTmcNoiseLevel = 0x0830,
    kTmcInputMode = 0x0834,
    kTmcOutputMode = 0x0838,
    kTmcMicFilter = 0x0840,
    kTmcAgcFilter = 0x0850,
    kTmcNoiseState = 0x08E4,
    kTmcNoiseGain = 0x1B18,
    kTmcNoiseFloor = 0x1B10,
    kTmcAnalysisState = 0x1BD8,
    kTmcEchoDelayed = 0x2248,
    kTmcLevelState = 0x2C04,
    kTmcCoherenceSeedA = 0x2C14,
    kTmcCoherenceSeedB = 0x2C18,
    kTmcLevelTracker = 0x2C1C,
    kTmcMicResampler = 0x2C20,
    kTmcOutputResampler = 0x2E7E,
    kTmcMicResamplerHist = 0x30DC,
    kTmcOutputResampHist = 0x319A,
    kTmcLeftDecimator = 0x3258,
    kTmcRightDecimator = 0x34B6,
    kTmcLeftDecimatorHst = 0x3714,
    kTmcRightDecimatorHt = 0x37D2,
    kTmcDispatchTable = 0x3890,
    kTmcAnalysisMic = 0x38B4,
    kTmcAnalysisRef = 0x3EB4,
    kTmcAnalysisNear = 0x44B4,
    kTmcSynthesisNear = 0x4AB4,
    kTmcSynthesisEcho = 0x50B4,
    kTmcFarSpectrumMic = 0x56B4,
    kTmcFarSpectrumRef = 0xEF0C,
    kTmcLongTaps = 0x18764,
    kTmcShortTaps = 0x21FBC,
    kTmcStepSize = 0x2485C,
    kTmcOverlapNear = 0x24960,
    kTmcOverlapEcho = 0x24A60,
    kTmcCoherence = 0x24D68,
    kTmcActiveDelay = 0x24E70,
    kTmcLockedDelay = 0x24E72,
    kTmcDelayLocked = 0x24E74,
    kTmcDelayLine = 0x24E76,
    kTmcDelayWriteIndex = 0x2506A,
    kTmcDelayHistogram = 0x2506C
};

inline float* tmcFloat(void* base, unsigned int offset)
{
    return reinterpret_cast<float*>(reinterpret_cast<char*>(base) + offset);
}

inline short* tmcShort(void* base, unsigned int offset)
{
    return reinterpret_cast<short*>(reinterpret_cast<char*>(base) + offset);
}

inline unsigned short* tmcUShort(void* base, unsigned int offset)
{
    return reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(base) + offset);
}

inline int* tmcInt(void* base, unsigned int offset)
{
    return reinterpret_cast<int*>(reinterpret_cast<char*>(base) + offset);
}

inline void* tmcAt(void* base, unsigned int offset)
{
    return reinterpret_cast<char*>(base) + offset;
}

typedef void (*TmcHfEntry)();

struct TmcHfDispatch
{
    int index;
    TmcHfEntry entry[8];
};

inline TmcHfDispatch* tmcDispatch(void* base)
{
    return reinterpret_cast<TmcHfDispatch*>(reinterpret_cast<char*>(base) + kTmcDispatchTable);
}

extern "C"
{
void TMC_HF_FUNC_001(void* handle, float* signal);
void TMC_HF_FUNC_002(void* handle, float* state, float* levels, float* mix, float* residual);
void TMC_HF_FUNC_003(void* handle, const short* micHistory, const short* referenceHistory, const float* nearInput,
                     float* nearOutput, short count, int echoMode);
void TMC_HF_FUNC_004(short* state, const short* source, short* destination, short* history, int count);
void TMC_HF_FUNC_005(short* state, const short* source, short* destination, short* history, int count);
void TMC_HF_FUNC_006(const float* spectrum, int* destination, float* history);
void TMC_HF_FUNC_009(float* state);
void TMC_HF_FUNC_010(void* state, const float* input, float* spectrum, int adaptEnabled);
void TMC_HF_FUNC_011(float* state, float* unused, float* signal);
void TMC_HF_FUNC_012(float* state, float* signal);
void TMC_HF_FUNC_013(float* state, const short* source, float* destination, int count);
void TMC_HF_FUNC_014(float* state, int target, short* signal);
void TMC_HF_FUNC_015(void* handle, float* state, float* spectrumA, float* spectrumB, float* alias);
void TMC_HF_FUNC_030(float* data, const int* order);
void TMC_HF_FUNC_031(float* data);
void TMC_HF_FUNC_032(float* data, const int* order);
void TMC_HF_FUNC_033(float* data);
void TMC_HF_FUNC_034(float* data);
void TMC_HF_FUNC_035(float* data, const int* order);
void TMC_HF_FUNC_036(float* data);
void TMC_HF_FUNC_037(float* data, const int* order);
void TMC_HF_FUNC_038(float* data);
void TMC_HF_FUNC_039(float* data, const int* order);
void TMC_HF_FUNC_040(float* data);
void TMC_HF_FUNC_041(float* data);
void TMC_HF_FUNC_042(float* data, const int* order);
void TMC_HF_FUNC_043(float* data);
int TMC_HF_FUNC_044(int value);
void TMC_HF_FUNC_045(TmcHfDispatch* table);
void TMC_HF_FUNC_046(TmcHfDispatch* table, void* handle, const short* micHistory, const short* referenceHistory,
                     const float* nearInput, float* nearOutput, short count, int echoMode);
void TMC_HF_FUNC_047(TmcHfDispatch* table, float* state, float* unused, float* signal);
void TMC_HF_FUNC_048(TmcHfDispatch* table, float* state, int target, short* signal);
void TMC_HF_FUNC_049(TmcHfDispatch* table, void* handle, float* state, float* levels, float* mix, float* residual);
void TMC_HF_FUNC_050(TmcHfDispatch* table, float* state, float* signal);
void TMC_HF_FUNC_051(TmcHfDispatch* table, float* state, const short* source, float* destination, int count);
void TMC_HF_FUNC_052(TmcHfDispatch* table, void* handle, float* signal);
void TMC_HF_FUNC_053(TmcHfDispatch* table, short* state, const short* source, short* destination, short* history,
                     int count);
}

const float kTmcAnalysisWindow[128] = {
    0.00019999999494757503f, 0.6467999815940857f,  0.0006000000284984708f, 0.6464999914169312f,
    0.0010000000474974513f,  0.6459000110626221f,  0.001500000013038516f,  0.6449999809265137f,
    0.0019000000320374966f,  0.6437000036239624f,  0.002400000113993883f,  0.6421999931335449f,
    0.002899999963119626f,   0.6403999924659729f,  0.0035000001080334187f, 0.6381999850273132f,
    0.004100000020116568f,   0.635699987411499f,   0.004699999932199717f,  0.6330000162124634f,
    0.005400000140070915f,   0.6299999952316284f,  0.006099999882280827f,  0.6266000270843506f,
    0.006899999920278788f,   0.6230000257492065f,  0.007699999958276749f,  0.6190999746322632f,
    0.008700000122189522f,   0.6148999929428101f,  0.009700000286102295f,  0.6104999780654907f,
    0.010700000450015068f,   0.6057999730110168f,  0.011900000274181366f,  0.6007999777793884f,
    0.013100000098347664f,   0.5954999923706055f,  0.014499999582767487f,  0.5900999903678894f,
    0.01590000092983246f,    0.5842999815940857f,  0.017500000074505806f,  0.5784000158309937f,
    0.019099999219179153f,   0.5722000002861023f,  0.020899999886751175f,  0.5658000111579895f,
    0.02280000038444996f,    0.5590999722480774f,  0.024800000712275505f,  0.552299976348877f,
    0.026900000870227814f,   0.545199990272522f,   0.029200000688433647f,  0.5379999876022339f,
    0.031599998474121094f,   0.5306000113487244f,  0.0340999998152256f,    0.5230000019073486f,
    0.03680000081658363f,    0.5152999758720398f,  0.03959999978542328f,   0.5073999762535095f,
    0.04259999841451645f,    0.4993000030517578f,  0.04569999873638153f,   0.491100013256073f,
    0.04899999871850014f,    0.4828000068664551f,  0.05249999836087227f,   0.47429999709129333f,
    0.05609999969601631f,    0.4657999873161316f,  0.05990000069141388f,   0.4571000039577484f,
    0.06379999965429306f,    0.44830000400543213f, 0.06790000200271606f,   0.43950000405311584f,
    0.0722000002861023f,     0.43059998750686646f, 0.07660000026226044f,   0.42160001397132874f,
    0.08129999786615372f,    0.41260001063346863f, 0.08609999716281891f,   0.4034999907016754f,
    0.09099999815225601f,    0.3944000005722046f,  0.09619999676942825f,   0.38519999384880066f,
    0.1014999970793724f,     0.37599998712539673f, 0.10700000077486038f,   0.3668000102043152f,
    0.11259999871253967f,    0.357699990272522f,   0.1185000017285347f,    0.34850001335144043f,
    0.12449999898672104f,    0.3393000066280365f,  0.1307000070810318f,    0.3301999866962433f,
    0.13699999451637268f,    0.32109999656677246f, 0.14350000023841858f,   0.31200000643730164f,
    0.1501999944448471f,     0.30300000309944153f, 0.15710000693798065f,   0.2939999997615814f,
    0.16410000622272491f,    0.2851000130176544f,  0.1712000072002411f,    0.27630001306533813f,
    0.1784999966621399f,     0.26759999990463257f, 0.1860000044107437f,    0.258899986743927f,
    0.19359999895095825f,    0.25040000677108765f, 0.2012999951839447f,    0.2418999969959259f,
    0.20919999480247498f,    0.23350000381469727f, 0.21719999611377716f,   0.22529999911785126f,
};

const float kTmcBandWindow[64] = {
    0.0006000000284984708f,
    0.002400000113993883f,
    0.0052999998442828655f,
    0.009499999694526196f,
    0.014800000004470348f,
    0.021199999377131462f,
    0.02879999950528145f,
    0.03750000149011612f,
    0.04729999974370003f,
    0.05810000002384186f,
    0.07010000199079514f,
    0.08299999684095383f,
    0.09690000116825104f,
    0.11180000007152557f,
    0.12759999930858612f,
    0.14429999887943268f,
    0.16179999709129333f,
    0.18019999563694f,
    0.19930000603199005f,
    0.2190999984741211f,
    0.2395000010728836f,
    0.2606000006198883f,
    0.2822999954223633f,
    0.3043999969959259f,
    0.32710000872612f,
    0.35010001063346863f,
    0.3734999895095825f,
    0.39719998836517334f,
    0.4212000072002411f,
    0.44530001282691956f,
    0.46959999203681946f,
    0.49390000104904175f,
    0.5182999968528748f,
    0.5425999760627747f,
    0.5667999982833862f,
    0.5907999873161316f,
    0.6146000027656555f,
    0.6381999850273132f,
    0.6614000201225281f,
    0.6843000054359436f,
    0.7067000269889832f,
    0.728600025177002f,
    0.75f,
    0.770799994468689f,
    0.7908999919891357f,
    0.8104000091552734f,
    0.8291000127792358f,
    0.847000002861023f,
    0.8641999959945679f,
    0.8804000020027161f,
    0.8956999778747559f,
    0.9101999998092651f,
    0.9236000180244446f,
    0.9359999895095825f,
    0.9473999738693237f,
    0.9577999711036682f,
    0.9670000076293945f,
    0.9751999974250793f,
    0.982200026512146f,
    0.9879999756813049f,
    0.9927999973297119f,
    0.9962999820709229f,
    0.9987000226974487f,
    0.9998999834060669f,
};

const float kTmcAgcAttenuation[40] = {
    1.0f,
    0.8913000226020813f,
    0.7943000197410583f,
    0.7078999876976013f,
    0.6309999823570251f,
    0.5623000264167786f,
    0.5012000203132629f,
    0.44670000672340393f,
    0.39809998869895935f,
    0.3547999858856201f,
    0.31619998812675476f,
    0.2818000018596649f,
    0.25119999051094055f,
    0.22390000522136688f,
    0.19949999451637268f,
    0.1777999997138977f,
    0.15850000083446503f,
    0.1412999927997589f,
    0.125900000333786f,
    0.11219999939203262f,
    0.10000000149011612f,
    0.08910000324249268f,
    0.07940000295639038f,
    0.07079999893903732f,
    0.06310000270605087f,
    0.05620000138878822f,
    0.05009999871253967f,
    0.04470000043511391f,
    0.039799999445676804f,
    0.03550000116229057f,
    0.031599998474121094f,
    0.028200000524520874f,
    0.025100000202655792f,
    0.02239999920129776f,
    0.019999999552965164f,
    0.017799999564886093f,
    0.015799999237060547f,
    0.014100000262260437f,
    0.012600000016391277f,
    0.01119999960064888f,
};

const float kTmcAgcGain[31] = {
    1.0f,
    1.121999979019165f,
    1.2589000463485718f,
    1.412500023841858f,
    1.5848000049591064f,
    1.7782000303268433f,
    1.9952000379562378f,
    2.238600015640259f,
    2.5118000507354736f,
    2.8183000087738037f,
    3.1621999740600586f,
    3.5480000972747803f,
    3.9809000492095947f,
    4.466700077056885f,
    5.01170015335083f,
    5.623199939727783f,
    6.3094000816345215f,
    7.07919979095459f,
    7.942999839782715f,
    8.912199974060059f,
    10.0f,
    11.220000267028809f,
    12.58899974822998f,
    14.125f,
    15.848999977111816f,
    17.783000946044922f,
    19.952999114990234f,
    22.386999130249023f,
    25.118999481201172f,
    28.18400001525879f,
    31.62299919128418f,
};

const float kTmcInterpolatorTaps[23] = {
    -0.00012206999963382259f, 0.0f, 0.0003662100061774254f,  0.0f, -0.0010375999845564365f, 0.0f,
    0.002227779943495989f,    0.0f, -0.0043334998190402985f, 0.0f, 0.0076599097810685635f,  0.0f,
    -0.012847900390625f,      0.0f, 0.0207214392721653f,     0.0f, -0.0330810509622097f,    0.0f,
    0.05419921875f,           0.0f, -0.100189208984375f,     0.0f, 0.3162841796875f,
};

const short kTmcDecimatorTaps[23] = {
    -4, 0, 12, 0, -34, 0, 73, 0, -142, 0, 251, 0, -421, 0, 679, 0, -1084, 0, 1776, 0, -3283, 0, 10364,
};

const short kTmcUpsamplerTaps[24] = {
    -1, 3, -9, 18, -36, 62, -106, 169, -271, 444, -821, 2591, 2591, -821, 444, -271, 169, -106, 62, -36, 18, -9, 3, -1,
};

const int kTmcBandUpperBin[45] = {
    0,  0,  1,  2,  3,  4,  4,  5,  6,  7,  8,  9,  10, 11, 12, 14, 15, 16, 18,  19,  21,  23,  25,
    27, 29, 31, 34, 37, 40, 43, 46, 50, 53, 58, 62, 67, 72, 77, 83, 89, 96, 103, 111, 119, 127,
};

const int kTmcBandLowerBin[45] = {
    -1, 0,  0,  1,  2,  3,  4,  4,  5,  6,  7,  8,  9,  10, 11, 12, 14, 15, 16, 18,  19,  21,  23,
    25, 27, 29, 31, 34, 37, 40, 43, 46, 50, 53, 58, 62, 67, 72, 77, 83, 89, 96, 103, 111, 119,
};

const float kTmcBandUpperWeight[45] = {
    0.2434999942779541f,
    0.0f,
    0.7457000017166138f,
    0.5120000243186951f,
    0.2937999963760376f,
    0.09480000287294388f,
    0.0f,
    0.7716000080108643f,
    0.6557999849319458f,
    0.5766000151634216f,
    0.5385000109672546f,
    0.5465999841690063f,
    0.6060000061988831f,
    0.722100019454956f,
    0.9004999995231628f,
    0.14740000665187836f,
    0.4690000116825104f,
    0.8720999956130981f,
    0.36390000581741333f,
    0.9520000219345093f,
    0.6445000171661377f,
    0.44999998807907104f,
    0.37779998779296875f,
    0.4375999867916107f,
    0.6399999856948853f,
    0.9962999820709229f,
    0.5184999704360962f,
    0.21930000185966492f,
    0.1128000020980835f,
    0.2134999930858612f,
    0.5372999906539917f,
    0.10119999945163727f,
    0.9233999848365784f,
    0.023399999365210533f,
    0.4221000075340271f,
    0.1420000046491623f,
    0.2071000039577484f,
    0.6434000134468079f,
    0.47850000858306885f,
    0.7422000169754028f,
    0.4666000008583069f,
    0.6858999729156494f,
    0.43700000643730164f,
    0.7594000101089478f,
    0.5f,
};

const float kTmcBandLowerWeight[45] = {
    0.5f,
    0.7473000288009644f,
    0.009200000204145908f,
    0.25429999828338623f,
    0.4880000054836273f,
    0.7062000036239624f,
    0.8245000243186951f,
    0.08060000091791153f,
    0.22840000689029694f,
    0.3441999852657318f,
    0.42340001463890076f,
    0.46149998903274536f,
    0.45339998602867126f,
    0.39399999380111694f,
    0.27790001034736633f,
    0.09950000047683716f,
    0.8525999784469604f,
    0.531000018119812f,
    0.12790000438690186f,
    0.6360999941825867f,
    0.04800000041723251f,
    0.3555000126361847f,
    0.550000011920929f,
    0.6222000122070312f,
    0.5623999834060669f,
    0.36000001430511475f,
    0.003700000001117587f,
    0.4814999997615814f,
    0.7807000279426575f,
    0.8871999979019165f,
    0.7864999771118164f,
    0.4627000093460083f,
    0.8988000154495239f,
    0.07660000026226044f,
    0.9765999913215637f,
    0.5778999924659729f,
    0.8579999804496765f,
    0.792900025844574f,
    0.35659998655319214f,
    0.5214999914169312f,
    0.25780001282691956f,
    0.5333999991416931f,
    0.3140999972820282f,
    0.5630000233650208f,
    0.24060000479221344f,
};

const float kTmcBinWeight[125] = {
    0.0f,
    0.8440999984741211f,
    0.17599999904632568f,
    0.484499990940094f,
    0.7599999904632568f,
    0.995199978351593f,
    0.1851000040769577f,
    0.32659998536109924f,
    0.41850000619888306f,
    0.4607999920845032f,
    0.454800009727478f,
    0.40220001339912415f,
    0.3052000105381012f,
    0.1662999987602234f,
    0.9879000186920166f,
    0.772599995136261f,
    0.5227000117301941f,
    0.24060000479221344f,
    0.9283999800682068f,
    0.5881999731063843f,
    0.22169999778270721f,
    0.8307999968528748f,
    0.4171000123023987f,
    0.9818999767303467f,
    0.5267999768257141f,
    0.052799999713897705f,
    0.5612000226974487f,
    0.05310000106692314f,
    0.5292999744415283f,
    0.9908000230789185f,
    0.438400000333786f,
    0.8729000091552734f,
    0.29490000009536743f,
    0.7052000164985657f,
    0.10440000146627426f,
    0.492900013923645f,
    0.871399998664856f,
    0.2402999997138977f,
    0.6000999808311462f,
    0.9510999917984009f,
    0.2939000129699707f,
    0.6287000179290771f,
    0.9559999704360962f,
    0.2759999930858612f,
    0.5889999866485596f,
    0.8953999876976013f,
    0.19529999792575836f,
    0.48919999599456787f,
    0.7771000266075134f,
    0.059300001710653305f,
    0.3361000120639801f,
    0.607699990272522f,
    0.8741000294685364f,
    0.13570000231266022f,
    0.39259999990463257f,
    0.6449000239372253f,
    0.8928999900817871f,
    0.13660000264644623f,
    0.37619999051094055f,
    0.6118000149726868f,
    0.8435999751091003f,
    0.07169999927282333f,
    0.296099990606308f,
    0.5170999765396118f,
    0.7347000241279602f,
    0.9490000009536743f,
    0.16009999811649323f,
    0.36809998750686646f,
    0.5730000138282776f,
    0.7750999927520752f,
    0.9743000268936157f,
    0.17080000042915344f,
    0.3646000027656555f,
    0.5557000041007996f,
    0.7443000078201294f,
    0.930400013923645f,
    0.11410000175237656f,
    0.2955000102519989f,
    0.47450000047683716f,
    0.6513000130653381f,
    0.8259000182151794f,
    0.9983999729156494f,
    0.1687999963760376f,
    0.33719998598098755f,
    0.503600001335144f,
    0.6679999828338623f,
    0.8305000066757202f,
    0.9911999702453613f,
    0.1500999927520752f,
    0.30720001459121704f,
    0.4625999927520752f,
    0.6162999868392944f,
    0.7682999968528748f,
    0.9186999797821045f,
    0.06759999692440033f,
    0.21480000019073486f,
    0.36059999465942383f,
    0.504800021648407f,
    0.647599995136261f,
    0.7889000177383423f,
    0.9289000034332275f,
    0.0674000009894371f,
    0.2046000063419342f,
    0.34049999713897705f,
    0.47510001063346863f,
    0.6083999872207642f,
    0.7404999732971191f,
    0.8712999820709229f,
    0.0010000000474974513f,
    0.12939999997615814f,
    0.2567000091075897f,
    0.38280001282691956f,
    0.5077999830245972f,
    0.6317999958992004f,
    0.7545999884605408f,
    0.8763999938964844f,
    0.9970999956130981f,
    0.12489999830722809f,
    0.251800000667572f,
    0.37770000100135803f,
    0.5024999976158142f,
    0.6262999773025513f,
    0.7490000128746033f,
    0.8708000183105469f,
    0.991599977016449f,
};

const int kTmcBinBand[125] = {
    0,  0,  2,  3,  4,  5,  7,  8,  9,  10, 11, 12, 13, 14, 14, 15, 16, 17, 17, 18, 19, 19, 20, 20, 21,
    22, 22, 23, 23, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31,
    31, 31, 31, 32, 32, 32, 32, 33, 33, 33, 33, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 36, 36, 36, 36,
    36, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40,
    40, 41, 41, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 43, 43, 43,
};

const float kTmcGainMemory[45] = {
    0.9251000285148621f,  0.8167999982833862f,  0.7423999905586243f,  0.6880999803543091f,  0.6467000246047974f,
    0.6139000058174133f,  0.5874000191688538f,  0.5652999877929688f,  0.5467000007629395f,  0.5306000113487244f,
    0.516700029373169f,   0.5044999718666077f,  0.4936000108718872f,  0.4839000105857849f,  0.47510001063346863f,
    0.46720001101493835f, 0.46000000834465027f, 0.45339998602867126f, 0.447299987077713f,   0.4417000114917755f,
    0.436599999666214f,   0.4318999946117401f,  0.42750000953674316f, 0.42340001463890076f, 0.4196000099182129f,
    0.41609999537467957f, 0.41290000081062317f, 0.4097999930381775f,  0.40700000524520874f, 0.4043999910354614f,
    0.4018999934196472f,  0.39959999918937683f, 0.39750000834465027f, 0.3955000042915344f,  0.3935999870300293f,
    0.3919000029563904f,  0.3903000056743622f,  0.388700008392334f,   0.3873000144958496f,  0.38600000739097595f,
    0.384799987077713f,   0.38359999656677246f, 0.38260000944137573f, 0.3815999925136566f,  0.3806999921798706f,
};

const float kTmcGainUpdate[45] = {
    0.07490000128746033f, 0.18320000171661377f, 0.25760000944137573f, 0.31189998984336853f, 0.353300005197525f,
    0.38609999418258667f, 0.41260001063346863f, 0.43470001220703125f, 0.45329999923706055f, 0.46939998865127563f,
    0.48330000042915344f, 0.49549999833106995f, 0.5063999891281128f,  0.5160999894142151f,  0.5249000191688538f,
    0.532800018787384f,   0.5400000214576721f,  0.5465999841690063f,  0.5526999831199646f,  0.5583000183105469f,
    0.5633999705314636f,  0.5680999755859375f,  0.5724999904632568f,  0.5766000151634216f,  0.5803999900817871f,
    0.583899974822998f,   0.5871000289916992f,  0.5902000069618225f,  0.5929999947547913f,  0.5956000089645386f,
    0.5981000065803528f,  0.6003999710083008f,  0.6025000214576721f,  0.6044999957084656f,  0.6064000129699707f,
    0.6080999970436096f,  0.6097000241279602f,  0.611299991607666f,   0.6126999855041504f,  0.6140000224113464f,
    0.6151999831199646f,  0.6164000034332275f,  0.6173999905586243f,  0.618399977684021f,   0.6193000078201294f,
};

const float kTmcTwiddle[256] = {
    1.0f,
    0.0f,
    -4.371138828673793e-08f,
    1.0f,
    0.7071067690849304f,
    0.7071067690849304f,
    -0.7071068286895752f,
    0.7071067094802856f,
    0.9238795042037964f,
    0.3826834559440613f,
    -0.38268348574638367f,
    0.9238795042037964f,
    0.3826833963394165f,
    0.9238795638084412f,
    -0.9238795638084412f,
    0.3826833665370941f,
    0.9807852506637573f,
    0.19509032368659973f,
    -0.1950903683900833f,
    0.9807852506637573f,
    0.5555701851844788f,
    0.8314696550369263f,
    -0.8314696550369263f,
    0.5555701851844788f,
    0.8314695954322815f,
    0.5555702447891235f,
    -0.5555703043937683f,
    0.8314695954322815f,
    0.19509027898311615f,
    0.9807853102684021f,
    -0.9807853102684021f,
    0.19509023427963257f,
    0.9951847195625305f,
    0.0980171412229538f,
    -0.09801718592643738f,
    0.9951847195625305f,
    0.6343932747840881f,
    0.7730104923248291f,
    -0.7730104923248291f,
    0.6343932151794434f,
    0.8819212317466736f,
    0.4713967442512512f,
    -0.4713967740535736f,
    0.8819212317466736f,
    0.2902846336364746f,
    0.9569403529167175f,
    -0.9569403529167175f,
    0.2902846038341522f,
    0.9569403529167175f,
    0.2902846932411194f,
    -0.2902847230434418f,
    0.9569402933120728f,
    0.47139671444892883f,
    0.8819212913513184f,
    -0.8819212913513184f,
    0.47139668464660645f,
    0.7730104327201843f,
    0.6343932747840881f,
    -0.6343933343887329f,
    0.7730104327201843f,
    0.09801709651947021f,
    0.9951847195625305f,
    -0.9951847195625305f,
    0.09801705926656723f,
    0.9987954497337341f,
    0.049067676067352295f,
    -0.049067720770835876f,
    0.9987954497337341f,
    0.6715589165687561f,
    0.7409511208534241f,
    -0.7409511804580688f,
    0.6715589165687561f,
    0.903989315032959f,
    0.427555114030838f,
    -0.4275551438331604f,
    0.9039892554283142f,
    0.3368898332118988f,
    0.9415440559387207f,
    -0.9415441155433655f,
    0.33688977360725403f,
    0.9700312614440918f,
    0.24298018217086792f,
    -0.2429802268743515f,
    0.9700312614440918f,
    0.5141026973724365f,
    0.8577286005020142f,
    -0.8577286601066589f,
    0.5141026973724365f,
    0.803207516670227f,
    0.5956993103027344f,
    -0.5956993699073792f,
    0.803207516670227f,
    0.146730437874794f,
    0.9891765117645264f,
    -0.9891765117645264f,
    0.14673039317131042f,
    0.9891765117645264f,
    0.1467304825782776f,
    -0.14673052728176117f,
    0.9891765117645264f,
    0.5956993103027344f,
    0.8032075762748718f,
    -0.8032075762748718f,
    0.5956992506980896f,
    0.8577286005020142f,
    0.5141027569770813f,
    -0.5141028165817261f,
    0.8577286005020142f,
    0.24298013746738434f,
    0.9700312614440918f,
    -0.9700312614440918f,
    0.24298010766506195f,
    0.9415440559387207f,
    0.3368898630142212f,
    -0.3368898928165436f,
    0.9415440559387207f,
    0.42755505442619324f,
    0.903989315032959f,
    -0.903989315032959f,
    0.42755502462387085f,
    0.7409511208534241f,
    0.6715589761734009f,
    -0.6715589761734009f,
    0.7409510612487793f,
    0.04906763136386871f,
    0.9987954497337341f,
    -0.9987954497337341f,
    0.04906758666038513f,
    0.99969881772995f,
    0.024541229009628296f,
    -0.024541273713111877f,
    0.99969881772995f,
    0.6895405054092407f,
    0.7242470979690552f,
    -0.7242471575737f,
    0.6895405054092407f,
    0.9142097234725952f,
    0.40524131059646606f,
    -0.40524137020111084f,
    0.9142097234725952f,
    0.3598949909210205f,
    0.9329928159713745f,
    -0.9329928159713745f,
    0.3598949611186981f,
    0.9757021069526672f,
    0.21910125017166138f,
    -0.21910129487514496f,
    0.9757021069526672f,
    0.5349975824356079f,
    0.8448535799980164f,
    -0.8448535799980164f,
    0.5349975824356079f,
    0.8175848126411438f,
    0.5758082270622253f,
    -0.5758082270622253f,
    0.817584753036499f,
    0.17096185684204102f,
    0.9852776527404785f,
    -0.9852776527404785f,
    0.17096181213855743f,
    0.9924795627593994f,
    0.12241067737340927f,
    -0.12241072207689285f,
    0.9924795031547546f,
    0.6152315735816956f,
    0.7883464694023132f,
    -0.7883464694023132f,
    0.6152315139770508f,
    0.8700869679450989f,
    0.49289819598197937f,
    -0.49289825558662415f,
    0.8700869679450989f,
    0.2667127251625061f,
    0.9637760519981384f,
    -0.9637761116027832f,
    0.2667126953601837f,
    0.949528157711029f,
    0.3136817514896393f,
    -0.31368178129196167f,
    0.949528157711029f,
    0.4496113061904907f,
    0.89322429895401f,
    -0.8932243585586548f,
    0.44961127638816833f,
    0.7572088241577148f,
    0.6531728506088257f,
    -0.6531729102134705f,
    0.7572088241577148f,
    0.07356452196836472f,
    0.9972904324531555f,
    -0.9972904920578003f,
    0.07356447726488113f,
    0.9972904324531555f,
    0.0735645666718483f,
    -0.07356461137533188f,
    0.9972904324531555f,
    0.6531728506088257f,
    0.7572088837623596f,
    -0.7572088837623596f,
    0.6531727910041809f,
    0.89322429895401f,
    0.4496113359928131f,
    -0.4496113657951355f,
    0.89322429895401f,
    0.3136817216873169f,
    0.9495282173156738f,
    -0.9495282173156738f,
    0.3136816620826721f,
    0.9637760519981384f,
    0.2667127549648285f,
    -0.26671281456947327f,
    0.9637760519981384f,
    0.492898166179657f,
    0.8700870275497437f,
    -0.8700870275497437f,
    0.4928981363773346f,
    0.7883464097976685f,
    0.6152316331863403f,
    -0.6152316331863403f,
    0.7883464097976685f,
    0.12241063266992569f,
    0.9924795627593994f,
    -0.9924795627593994f,
    0.1224106028676033f,
    0.9852776527404785f,
    0.1709618866443634f,
    -0.17096193134784698f,
    0.9852776527404785f,
    0.5758081674575806f,
    0.8175848126411438f,
    -0.8175848722457886f,
    0.5758081078529358f,
    0.8448535799980164f,
    0.5349976420402527f,
    -0.5349976420402527f,
    0.8448535203933716f,
    0.2191012054681778f,
    0.975702166557312f,
    -0.975702166557312f,
    0.2191011607646942f,
    0.9329928159713745f,
    0.3598950505256653f,
    -0.35989508032798767f,
    0.9329927563667297f,
    0.4052412807941437f,
    0.91420978307724f,
    -0.91420978307724f,
    0.4052412509918213f,
    0.7242470979690552f,
    0.6895405650138855f,
    -0.6895405650138855f,
    0.7242470383644104f,
    0.024541186168789864f,
    0.99969881772995f,
    -0.99969881772995f,
    0.024541141465306282f,
};

const int kTmcBitReverse128[64] = {
    0,  64,  32, 96,  16, 80,  48, 112, 8,  72,  40, 104, 24, 88, 56, 120, 4,  68,  36, 100, 20, 84,
    52, 116, 12, 76,  44, 108, 28, 92,  60, 124, 2,  66,  34, 98, 18, 82,  50, 114, 10, 74,  42, 106,
    26, 90,  58, 122, 6,  70,  38, 102, 22, 86,  54, 118, 14, 78, 46, 110, 30, 94,  62, 126,
};

const int kTmcBitReverse256[128] = {
    0,   128, 64,  192, 32, 160, 96,  224, 16,  144, 80,  208, 48,  176, 112, 240, 8,   136, 72,  200, 40,  168,
    104, 232, 24,  152, 88, 216, 56,  184, 120, 248, 4,   132, 68,  196, 36,  164, 100, 228, 20,  148, 84,  212,
    52,  180, 116, 244, 12, 140, 76,  204, 44,  172, 108, 236, 28,  156, 92,  220, 60,  188, 124, 252, 2,   130,
    66,  194, 34,  162, 98, 226, 18,  146, 82,  210, 50,  178, 114, 242, 10,  138, 74,  202, 42,  170, 106, 234,
    26,  154, 90,  218, 58, 186, 122, 250, 6,   134, 70,  198, 38,  166, 102, 230, 22,  150, 86,  214, 54,  182,
    118, 246, 14,  142, 78, 206, 46,  174, 110, 238, 30,  158, 94,  222, 62,  190, 126, 254,
};

void TMC_HF_FUNC_030(float* data, const int* order)
{
    int index = 0;
    for (int i = 0; i < 0x40; ++i)
    {
        const int target = order[i];
        const float realValue = data[index];
        const float imagValue = data[index + 1];
        if (index <= target)
        {
            data[index] = data[target];
            data[index + 1] = data[target + 1];
            data[target] = realValue;
            data[target + 1] = imagValue;
        }
        index += 2;
    }
}

void TMC_HF_FUNC_031(float* data)
{
    int i;
    for (i = 0; i < 0x40; i += 2)
    {
        const float even = data[i];
        const float odd = data[i + 1];
        data[i] = even + data[i + 0x40];
        data[i + 0x40] = even - data[i + 0x40];
        data[i + 1] = odd + data[i + 0x41];
        data[i + 0x41] = odd - data[i + 0x41];
    }
    for (i = 0; i < 0x20; i += 2)
    {
        const float even = data[i];
        const float odd = data[i + 1];
        data[i] = even + data[i + 0x20];
        data[i + 0x20] = even - data[i + 0x20];
        data[i + 1] = odd + data[i + 0x21];
        data[i + 0x21] = odd - data[i + 0x21];
    }
    for (i = 0; i < 0x20; i += 2)
    {
        const float rotatedReal = data[i + 0x60] * -4.371138828673793e-08f + data[i + 0x61];
        const float rotatedImag = data[i + 0x61] * -4.371138828673793e-08f - data[i + 0x60];
        const float odd = data[i + 0x41];
        data[i + 0x60] = data[i + 0x40] - rotatedReal;
        data[i + 0x40] = data[i + 0x40] + rotatedReal;
        data[i + 0x41] = odd + rotatedImag;
        data[i + 0x61] = odd - rotatedImag;
    }
    int stride = 0x40;
    int count = 2;
    int span = 0x20;
    int quarter;
    do
    {
        const int half = span >> 1;
        quarter = span >> 2;
        span = 0;
        count = count << 1;
        stride = stride >> 1;
        if (half != 0)
        {
            do
            {
                const int mate = half + span;
                float lowReal = data[span];
                float highReal = data[mate];
                float lowImag = data[span + 1];
                float highImag = data[mate + 1];
                data[span] = lowReal + highReal;
                data[span + 1] = lowImag + highImag;
                data[mate] = lowReal - highReal;
                data[mate + 1] = lowImag - highImag;
                int twiddle = 2;
                for (int step = 1; step < count; ++step)
                {
                    const int high = mate + stride * step;
                    const int low = span + stride * step;
                    const float rotatedReal = data[high] * kTmcTwiddle[twiddle]
                                              + data[high + 1] * kTmcTwiddle[twiddle + 1];
                    const float rotatedImag = data[high + 1] * kTmcTwiddle[twiddle]
                                              - data[high] * kTmcTwiddle[twiddle + 1];
                    lowReal = data[low];
                    lowImag = data[low + 1];
                    data[low] = lowReal + rotatedReal;
                    data[low + 1] = lowImag + rotatedImag;
                    data[high] = lowReal - rotatedReal;
                    data[high + 1] = lowImag - rotatedImag;
                    twiddle += 2;
                }
                span += 2;
            } while (span < half);
        }
        span = half;
    } while (quarter != 2);
    const float firstReal = data[0];
    const float firstImag = data[1];
    data[0] = firstReal + data[2];
    data[1] = firstImag + data[3];
    data[2] = firstReal - data[2];
    data[3] = firstImag - data[3];
    int twiddle = 2;
    for (int base = 4; twiddle < 0x40; base += 4)
    {
        float* pair = data + base;
        const float rotatedReal = pair[2] * kTmcTwiddle[twiddle] + pair[3] * kTmcTwiddle[twiddle + 1];
        const float rotatedImag = pair[3] * kTmcTwiddle[twiddle] - pair[2] * kTmcTwiddle[twiddle + 1];
        const float lowReal = pair[0];
        const float lowImag = pair[1];
        pair[0] = lowReal + rotatedReal;
        pair[1] = lowImag + rotatedImag;
        pair[2] = lowReal - rotatedReal;
        pair[3] = lowImag - rotatedImag;
        twiddle += 2;
    }
}

void TMC_HF_FUNC_032(float* data, const int* order)
{
    const float first = data[0];
    data[0] = first + data[1];
    data[1] = first - data[1];
    int index = 2;
    for (int bin = 1; bin < 0x20; ++bin)
    {
        float* pair = data + index;
        float diffReal = (data[index] - data[0x80 - index]) * 0.5f;
        const int twiddle = order[bin];
        const float diffImag = (data[0x81 - index] - pair[1]) * 0.5f;
        float sumImag = (pair[1] + data[0x81 - index]) * 0.5f;
        const float sumReal = (data[index] + data[0x80 - index]) * 0.5f;
        const float crossImag = kTmcTwiddle[twiddle] * sumImag;
        const float crossReal = kTmcTwiddle[twiddle] * diffReal;
        diffReal = kTmcTwiddle[twiddle + 1] * diffReal;
        sumImag = kTmcTwiddle[twiddle + 1] * sumImag;
        pair[0] = (sumReal + crossImag) - diffReal;
        pair[1] = (-diffImag - crossReal) - sumImag;
        data[0x80 - index] = (sumReal - crossImag) + diffReal;
        data[0x81 - index] = (diffImag - crossReal) - sumImag;
        index += 2;
    }
    data[0x41] = -data[0x41];
}

void TMC_HF_FUNC_033(float* data)
{
    TMC_HF_FUNC_031(data);
    TMC_HF_FUNC_030(data, kTmcBitReverse128);
    TMC_HF_FUNC_032(data, kTmcBitReverse128);
}

void TMC_HF_FUNC_034(float* data)
{
    int j;
    int step;
    int twiddle;
    int stride;
    int count;
    int span;
    int remaining;
    int i;
    for (i = 0; i < 0x20; i += 2)
    {
        const float x0i = data[i + 1] + data[i + 0x41];
        const float x2r = data[i + 0x20] + data[i + 0x60];
        const float x0r = data[i] + data[i + 0x40];
        const float x3r = data[i + 0x20] - data[i + 0x60];
        const float x1r = data[i] - data[i + 0x40];
        const float x2i = data[i + 0x21] + data[i + 0x61];
        const float x3i = data[i + 0x21] - data[i + 0x61];
        const float x1i = data[i + 1] - data[i + 0x41];
        data[i] = x0r + x2r;
        data[i + 0x20] = x0r - x2r;
        data[i + 1] = x0i + x2i;
        data[i + 0x21] = x0i - x2i;
        data[i + 0x40] = x1r - x3i;
        data[i + 0x41] = x1i + x3r;
        data[i + 0x60] = x1r + x3i;
        data[i + 0x61] = x1i - x3r;
    }
    stride = 0x40;
    count = 2;
    span = 0x20;
    do
    {
        span = span >> 1;
        remaining = span / 2 - 1;
        count = count << 1;
        stride = stride >> 1;
        for (j = 0; j < span; j += 2)
        {
            const int mate = span + j;
            float lowReal = data[j];
            float highReal = data[mate];
            float highImag = data[mate + 1];
            float lowImag = data[j + 1];
            data[j] = lowReal + highReal;
            data[j + 1] = lowImag + highImag;
            data[mate + 1] = lowImag - highImag;
            data[mate] = lowReal - highReal;
            twiddle = 2;
            for (step = 1; step < count; ++step)
            {
                const int high = mate + stride * step;
                const int low = j + stride * step;
                const float rotatedReal = data[high] * kTmcTwiddle[twiddle] - data[high + 1] * kTmcTwiddle[twiddle + 1];
                const float rotatedImag = data[high + 1] * kTmcTwiddle[twiddle] + data[high] * kTmcTwiddle[twiddle + 1];
                lowReal = data[low];
                lowImag = data[low + 1];
                data[low] = lowReal + rotatedReal;
                data[low + 1] = lowImag + rotatedImag;
                data[high + 1] = lowImag - rotatedImag;
                data[high] = lowReal - rotatedReal;
                twiddle += 2;
            }
        }
    } while (remaining != 0);
}

void TMC_HF_FUNC_035(float* data, const int* order)
{
    const float first = (data[0] + data[1]) * 0.5f;
    data[1] = (data[0] - data[1]) * 0.5f;
    data[0] = first;
    int index = 2;
    for (int bin = 1; bin < 0x20; ++bin)
    {
        float* pair = data + index;
        float diffReal = (pair[0] - data[0x80 - index]) * 0.5f;
        const int twiddle = order[bin];
        const float sumReal = (pair[0] + data[0x80 - index]) * 0.5f;
        float sumImag = -((pair[1] + data[0x81 - index]) * 0.5f);
        const float crossReal = kTmcTwiddle[twiddle] * diffReal;
        const float diffImag = (pair[1] - data[0x81 - index]) * 0.5f;
        const float crossImag = kTmcTwiddle[twiddle] * sumImag;
        diffReal = kTmcTwiddle[twiddle + 1] * diffReal;
        sumImag = kTmcTwiddle[twiddle + 1] * sumImag;
        pair[0] = (sumReal + crossImag) - diffReal;
        pair[1] = diffImag + crossReal + sumImag;
        data[0x80 - index] = (sumReal - crossImag) + diffReal;
        data[0x81 - index] = sumImag - (diffImag - crossReal);
        index += 2;
    }
    data[0x41] = -data[0x41];
}

void TMC_HF_FUNC_036(float* data)
{
    TMC_HF_FUNC_035(data, kTmcBitReverse128);
    TMC_HF_FUNC_034(data);
    TMC_HF_FUNC_030(data, kTmcBitReverse128);
}

void TMC_HF_FUNC_037(float* data, const int* order)
{
    int index = 0;
    for (int i = 0; i < 0x80; ++i)
    {
        const int target = order[i];
        const float realValue = data[index];
        const float imagValue = data[index + 1];
        if (index <= target)
        {
            data[index] = data[target];
            data[index + 1] = data[target + 1];
            data[target] = realValue;
            data[target + 1] = imagValue;
        }
        index += 2;
    }
}

void TMC_HF_FUNC_038(float* data)
{
    int i;
    for (i = 0; i < 0x80; i += 2)
    {
        const float even = data[i];
        const float odd = data[i + 1];
        data[i] = even + data[i + 0x80];
        data[i + 0x80] = even - data[i + 0x80];
        data[i + 1] = odd + data[i + 0x81];
        data[i + 0x81] = odd - data[i + 0x81];
    }
    for (i = 0; i < 0x40; i += 2)
    {
        const float even = data[i];
        const float odd = data[i + 1];
        data[i] = even + data[i + 0x40];
        data[i + 0x40] = even - data[i + 0x40];
        data[i + 1] = odd + data[i + 0x41];
        data[i + 0x41] = odd - data[i + 0x41];
    }
    for (i = 0; i < 0x40; i += 2)
    {
        const float rotatedReal = data[i + 0xc0] * -4.371138828673793e-08f + data[i + 0xc1];
        const float rotatedImag = data[i + 0xc1] * -4.371138828673793e-08f - data[i + 0xc0];
        const float odd = data[i + 0x81];
        data[i + 0xc0] = data[i + 0x80] - rotatedReal;
        data[i + 0x80] = data[i + 0x80] + rotatedReal;
        data[i + 0x81] = odd + rotatedImag;
        data[i + 0xc1] = odd - rotatedImag;
    }
    int stride = 0x80;
    int count = 2;
    int span = 0x40;
    int quarter;
    do
    {
        const int half = span >> 1;
        quarter = span >> 2;
        span = 0;
        count = count << 1;
        stride = stride >> 1;
        if (half != 0)
        {
            do
            {
                const int mate = half + span;
                float lowReal = data[span];
                float highReal = data[mate];
                float lowImag = data[span + 1];
                float highImag = data[mate + 1];
                data[span] = lowReal + highReal;
                data[span + 1] = lowImag + highImag;
                data[mate] = lowReal - highReal;
                data[mate + 1] = lowImag - highImag;
                int twiddle = 2;
                for (int step = 1; step < count; ++step)
                {
                    const int high = mate + stride * step;
                    const int low = span + stride * step;
                    const float rotatedReal = data[high] * kTmcTwiddle[twiddle]
                                              + data[high + 1] * kTmcTwiddle[twiddle + 1];
                    const float rotatedImag = data[high + 1] * kTmcTwiddle[twiddle]
                                              - data[high] * kTmcTwiddle[twiddle + 1];
                    lowReal = data[low];
                    lowImag = data[low + 1];
                    data[low] = lowReal + rotatedReal;
                    data[low + 1] = lowImag + rotatedImag;
                    data[high] = lowReal - rotatedReal;
                    data[high + 1] = lowImag - rotatedImag;
                    twiddle += 2;
                }
                span += 2;
            } while (span < half);
        }
        span = half;
    } while (quarter != 2);
    const float firstReal = data[0];
    const float firstImag = data[1];
    data[0] = firstReal + data[2];
    data[1] = firstImag + data[3];
    data[2] = firstReal - data[2];
    data[3] = firstImag - data[3];
    int twiddle = 2;
    for (int base = 4; twiddle < 0x80; base += 4)
    {
        float* pair = data + base;
        const float rotatedReal = pair[2] * kTmcTwiddle[twiddle] + pair[3] * kTmcTwiddle[twiddle + 1];
        const float rotatedImag = pair[3] * kTmcTwiddle[twiddle] - pair[2] * kTmcTwiddle[twiddle + 1];
        const float lowReal = pair[0];
        const float lowImag = pair[1];
        pair[0] = lowReal + rotatedReal;
        pair[1] = lowImag + rotatedImag;
        pair[2] = lowReal - rotatedReal;
        pair[3] = lowImag - rotatedImag;
        twiddle += 2;
    }
}

void TMC_HF_FUNC_039(float* data, const int* order)
{
    const float first = data[0];
    data[0] = first + data[1];
    data[1] = first - data[1];
    int index = 2;
    for (int bin = 1; bin < 0x40; ++bin)
    {
        float* pair = data + index;
        float diffReal = (data[index] - data[0x100 - index]) * 0.5f;
        const int twiddle = order[bin];
        const float diffImag = (data[0x101 - index] - pair[1]) * 0.5f;
        float sumImag = (pair[1] + data[0x101 - index]) * 0.5f;
        const float sumReal = (data[index] + data[0x100 - index]) * 0.5f;
        const float crossImag = kTmcTwiddle[twiddle] * sumImag;
        const float crossReal = kTmcTwiddle[twiddle] * diffReal;
        diffReal = kTmcTwiddle[twiddle + 1] * diffReal;
        sumImag = kTmcTwiddle[twiddle + 1] * sumImag;
        pair[0] = (sumReal + crossImag) - diffReal;
        pair[1] = (-diffImag - crossReal) - sumImag;
        data[0x100 - index] = (sumReal - crossImag) + diffReal;
        data[0x101 - index] = (diffImag - crossReal) - sumImag;
        index += 2;
    }
    data[0x81] = -data[0x81];
}

void TMC_HF_FUNC_040(float* data)
{
    TMC_HF_FUNC_038(data);
    TMC_HF_FUNC_037(data, kTmcBitReverse256);
    TMC_HF_FUNC_039(data, kTmcBitReverse256);
}

void TMC_HF_FUNC_041(float* data)
{
    int j;
    int step;
    int twiddle;
    int stride;
    int count;
    int span;
    int remaining;
    int i;
    for (i = 0; i < 0x40; i += 2)
    {
        const float x0i = data[i + 1] + data[i + 0x81];
        const float x2r = data[i + 0x40] + data[i + 0xc0];
        const float x0r = data[i] + data[i + 0x80];
        const float x3r = data[i + 0x40] - data[i + 0xc0];
        const float x1r = data[i] - data[i + 0x80];
        const float x2i = data[i + 0x41] + data[i + 0xc1];
        const float x3i = data[i + 0x41] - data[i + 0xc1];
        const float x1i = data[i + 1] - data[i + 0x81];
        data[i] = x0r + x2r;
        data[i + 0x40] = x0r - x2r;
        data[i + 1] = x0i + x2i;
        data[i + 0x41] = x0i - x2i;
        data[i + 0x80] = x1r - x3i;
        data[i + 0x81] = x1i + x3r;
        data[i + 0xc0] = x1r + x3i;
        data[i + 0xc1] = x1i - x3r;
    }
    stride = 0x80;
    count = 2;
    span = 0x40;
    do
    {
        span = span >> 1;
        remaining = span / 2 - 1;
        count = count << 1;
        stride = stride >> 1;
        for (j = 0; j < span; j += 2)
        {
            const int mate = span + j;
            float lowReal = data[j];
            float highReal = data[mate];
            float highImag = data[mate + 1];
            float lowImag = data[j + 1];
            data[j] = lowReal + highReal;
            data[j + 1] = lowImag + highImag;
            data[mate + 1] = lowImag - highImag;
            data[mate] = lowReal - highReal;
            twiddle = 2;
            for (step = 1; step < count; ++step)
            {
                const int high = mate + stride * step;
                const int low = j + stride * step;
                const float rotatedReal = data[high] * kTmcTwiddle[twiddle] - data[high + 1] * kTmcTwiddle[twiddle + 1];
                const float rotatedImag = data[high + 1] * kTmcTwiddle[twiddle] + data[high] * kTmcTwiddle[twiddle + 1];
                lowReal = data[low];
                lowImag = data[low + 1];
                data[low] = lowReal + rotatedReal;
                data[low + 1] = lowImag + rotatedImag;
                data[high + 1] = lowImag - rotatedImag;
                data[high] = lowReal - rotatedReal;
                twiddle += 2;
            }
        }
    } while (remaining != 0);
}

void TMC_HF_FUNC_042(float* data, const int* order)
{
    const float first = (data[0] + data[1]) * 0.5f;
    data[1] = (data[0] - data[1]) * 0.5f;
    data[0] = first;
    int index = 2;
    for (int bin = 1; bin < 0x40; ++bin)
    {
        float* pair = data + index;
        float diffReal = (pair[0] - data[0x100 - index]) * 0.5f;
        const int twiddle = order[bin];
        const float sumReal = (pair[0] + data[0x100 - index]) * 0.5f;
        float sumImag = -((pair[1] + data[0x101 - index]) * 0.5f);
        const float crossReal = kTmcTwiddle[twiddle] * diffReal;
        const float diffImag = (pair[1] - data[0x101 - index]) * 0.5f;
        const float crossImag = kTmcTwiddle[twiddle] * sumImag;
        diffReal = kTmcTwiddle[twiddle + 1] * diffReal;
        sumImag = kTmcTwiddle[twiddle + 1] * sumImag;
        pair[0] = (sumReal + crossImag) - diffReal;
        pair[1] = diffImag + crossReal + sumImag;
        data[0x100 - index] = (sumReal - crossImag) + diffReal;
        data[0x101 - index] = sumImag - (diffImag - crossReal);
        index += 2;
    }
    data[0x81] = -data[0x81];
}

void TMC_HF_FUNC_043(float* data)
{
    TMC_HF_FUNC_042(data, kTmcBitReverse256);
    TMC_HF_FUNC_041(data);
    TMC_HF_FUNC_037(data, kTmcBitReverse256);
}

int TMC_HF_FUNC_044(int value)
{
    if (value > 32767)
        value = 32767;
    else if (value < -32768)
        value = -32768;
    return value;
}

void TMC_HF_FUNC_004(short* state, const short* source, short* destination, short* history, int count)
{
    const int span = count * 2;
    int i;
    for (i = 0; i < 0x2f; ++i)
    {
        state[i] = history[i];
        history[i] = source[span - 0x2f + i];
    }
    for (i = 0; i < span; ++i)
        state[i + 0x2f] = source[i];
    for (i = 0; i < count; ++i)
    {
        int leading = static_cast<int>(state[i * 2 + 0x17]) << 14;
        int trailing = 0;
        for (int tap = 0; tap < 0x17; ++tap)
        {
            leading += static_cast<int>(state[i * 2 + tap]) * static_cast<int>(kTmcDecimatorTaps[tap]);
            trailing += static_cast<int>(state[i * 2 + 0x2e - tap]) * static_cast<int>(kTmcDecimatorTaps[tap]);
        }
        leading += trailing;
        if (leading < 0x3fff8001)
        {
            if (leading < -0x40000000)
                leading = -0x40000000;
            destination[i] = static_cast<short>(leading >> 15);
        }
        else
        {
            destination[i] = 0x7fff;
        }
    }
}

void TMC_HF_FUNC_005(short* state, const short* source, short* destination, short* history, int count)
{
    int i;
    for (i = 0; i < 0x17; ++i)
    {
        state[i] = history[i];
        history[i] = source[count - 0x17 + i];
    }
    for (i = 0; i < count; ++i)
        state[i + 0x17] = source[i];
    for (i = 0; i < count; ++i)
    {
        int accumulator = 0;
        for (int tap = 0; tap < 0x18; ++tap)
        {
            accumulator += static_cast<int>(state[i + tap]) * static_cast<int>(kTmcUpsamplerTaps[0x17 - tap]);
        }
        destination[i * 2] = state[i + 0x0b];
        if (accumulator < 0x7ffffff)
        {
            destination[i * 2 + 1] = accumulator < -0x8000000 ? static_cast<short>(-0x8000)
                                                              : static_cast<short>(accumulator >> 12);
        }
        else
        {
            destination[i * 2 + 1] = 0x7fff;
        }
    }
}

void TMC_HF_FUNC_006(const float* spectrum, int* destination, float* history)
{
    float buffer[175];
    int i;
    for (i = 0; i < 0x2f; ++i)
    {
        buffer[i] = history[i];
        history[i] = spectrum[i + 0x51];
    }
    for (i = 0; i < 0x80; ++i)
        buffer[i + 0x2f] = spectrum[i];
    for (i = 0; i < 0x40; ++i)
    {
        float leading = buffer[i * 2 + 0x17] * 0.5f;
        float trailing = 0.0f;
        for (int tap = 0; tap < 0x17; ++tap)
        {
            leading += buffer[i * 2 + tap] * kTmcInterpolatorTaps[tap];
            trailing += buffer[0x2e + i * 2 - tap] * kTmcInterpolatorTaps[tap];
        }
        destination[i] = static_cast<int>(leading + trailing);
    }
}

void TMC_HF_FUNC_009(float* state)
{
    for (int i = 0; i < 0x30; ++i)
    {
        state[i] = 16.0f;
        state[i + 0x210] = 4194304.0f;
        for (int block = 0; block < 2; ++block)
            state[block * 0x30 + i + 0x30] = 4194304.0f;
    }
}

void TMC_HF_FUNC_011(float* state, float* unused, float* signal)
{
    if (50.0f <= state[6])
        return;
    for (int i = 0; i < 0x80; ++i)
    {
        float attack;
        if (200000.0f <= state[0] || state[2] <= 10000.0f)
            attack = state[4] * 0.9900000095367432f + 0.009999999776482582f;
        else
            attack = state[4] * 0.9994999766349792f + 6.25000029685907e-05f;
        state[4] = attack;
        const float scaled = signal[i] * attack;
        signal[i] = scaled;
        const float energy = state[1] * 0.9950000047683716f + scaled * scaled * 0.004999999888241291f;
        state[1] = energy;
        bool fast = state[0] + state[0] < state[3];
        if (fast && (10000.0f <= energy || state[2] <= 10000.0f))
            fast = false;
        float release;
        if (fast)
            release = state[5] * 0.9994999766349792f + 6.25000029685907e-05f;
        else
            release = state[5] * 0.9900000095367432f + 0.009999999776482582f;
        state[5] = release;
        signal[i] = signal[i] * release;
    }
}

void TMC_HF_FUNC_012(float* state, float* signal)
{
    for (int i = 0; i < 0x80; ++i)
    {
        const float delay4 = state[4];
        const float delay5 = state[5];
        state[5] = delay4;
        const float delay6 = state[6];
        const float delay7 = state[7];
        const float delay8 = state[8];
        state[8] = delay7;
        state[7] = delay6;
        state[9] = delay8;
        const float delay1 = state[1];
        const float delay2 = state[2];
        state[2] = delay1;
        const float delay3 = state[3];
        const float delay0 = state[0];
        state[4] = delay3;
        state[1] = delay0;
        state[6] = delay5;
        state[3] = delay2;
        const float input = signal[i];
        state[0] = input;
        signal[i] = -0.03880000114440918f * delay8 + -0.03880000114440918f * input + 0.007499999832361937f * delay0
                    + 0.007499999832361937f * delay7 + 0.051100000739097595f * delay1 + 0.051100000739097595f * delay6
                    + -0.16930000483989716f * delay2 + -0.16930000483989716f * delay5 + 0.6495000123977661f * delay3
                    + 0.6495000123977661f * delay4;
    }
}

void TMC_HF_FUNC_013(float* state, const short* source, float* destination, int count)
{
    for (int i = 0; i < count; ++i)
    {
        const float previousInput = state[2];
        const float olderInput = state[3];
        const float input = static_cast<float>(static_cast<int>(source[i]));
        state[3] = previousInput;
        state[2] = input;
        destination[i] = state[1] * -0.911376953125f + state[0] * 1.906005859375f + input * 0.92724609375f
                         + previousInput * -1.8544940948486328f + olderInput * 0.9272469282150269f;
        state[1] = state[0];
        state[0] = destination[i];
    }
}

void TMC_HF_FUNC_014(float* state, int target, short* signal)
{
    short* cursor = reinterpret_cast<short*>(state + 0x24);
    for (int i = 0; i < 0x80; ++i)
    {
        const int slot = static_cast<int>(*cursor);
        float running = state[3];
        const float input = static_cast<float>(static_cast<int>(signal[i]));
        state[slot + 4] = input;
        const float magnitude = fabsf(input);
        running = (running - state[slot + 0x14]) + magnitude;
        state[slot + 0x14] = magnitude;
        const float average = running * 0.0625f;
        state[3] = running;
        const unsigned int next = static_cast<unsigned int>(slot + 1) & 0xfu;
        *cursor = static_cast<short>(next);
        float fastEnvelope;
        float slowEnvelope;
        if (average - state[2] <= 0.0f)
        {
            fastEnvelope = state[1] * 0.75f + average * 16384.0f;
            slowEnvelope = state[1] * 0.9990000128746033f + average * 8.0f;
        }
        else
        {
            fastEnvelope = state[0] * 0.75f + average * 16384.0f;
            slowEnvelope = state[0] * 0.9990000128746033f + average * 8.0f;
        }
        float envelope = slowEnvelope;
        if (state[2] - average < 0.0f)
            envelope = fastEnvelope;
        envelope = envelope * 1.52587890625e-05f;
        const float level = static_cast<float>(target);
        state[1] = slowEnvelope;
        state[2] = envelope;
        const float headroom = level - envelope;
        state[0] = fastEnvelope;
        float gain = 1.0f;
        if (headroom < 0.0f)
            gain = level / envelope;
        int sample = static_cast<int>(state[next + 4] * gain);
        if (sample < 0x8000)
        {
            if (sample < -0x8000)
                sample = -0x8000;
            signal[i] = static_cast<short>(sample);
        }
        else
        {
            signal[i] = 0x7fff;
        }
    }
}

void TMC_HF_FUNC_010(void* state, const float* input, float* spectrum, int adaptEnabled)
{
    float* const bandFloor = tmcFloat(state, 0x000);
    float* const minimumHistory = tmcFloat(state, 0x0C0);
    float* const bandPeak = tmcFloat(state, 0x840);
    float* const bandSmooth = tmcFloat(state, 0x900);
    float* const bandMean = tmcFloat(state, 0x9C0);
    float* const bandLevel = tmcFloat(state, 0xA80);
    float* const bandMeanSquare = tmcFloat(state, 0xB40);
    int* const peakValid = tmcInt(state, 0xC00);
    int* const updatePhase = tmcInt(state, 0xCC0);
    int* const holdEnable = tmcInt(state, 0xCC4);
    float* const coherence = tmcFloat(state, 0xCC8);
    int* const voicedCount = tmcInt(state, 0xCCC);
    float* const farLevel = tmcFloat(state, 0xCE4);
    float* const nearLevel = tmcFloat(state, 0xCE8);
    int* const frameCount = tmcInt(state, 0xCEC);
    int* const sampleHistory = tmcInt(state, 0xCF0);
    int* const currentSamples = tmcInt(state, 0xF70);
    float* const resampleHistory = tmcFloat(state, 0x1170);
    float* const gainCeiling = tmcFloat(state, 0x122C);
    float* const bandGain = tmcFloat(state, 0x1234);

    float bandPower[48];
    float bandExcess[48];
    float bandFloorDb[48];
    float flatness[48];
    float attackShape[48];
    float releaseShape[48];
    int i;

    for (i = 0; i < 0x80; ++i)
        currentSamples[i] = static_cast<int>(input[i]);
    TMC_HF_FUNC_006(input, currentSamples, resampleHistory);

    const float savedBin = spectrum[1];
    spectrum[1] = 0.0f;
    float speechSum = 0.0f;
    for (i = 0; i < 0x30; ++i)
    {
        float power;
        if (i < 0x20)
        {
            const float* pair = spectrum + i * 4;
            power = pair[1] * pair[1] + pair[0] * pair[0] + pair[2] * pair[2] + pair[3] * pair[3];
        }
        else
        {
            const float* group = spectrum + i * 8 - 0x80;
            power = 0.0f;
            for (int bin = 0; bin < 8; ++bin)
                power += group[bin] * group[bin] * 0.5f;
        }
        power = power * 0.5f + 9.99999993922529e-09f;
        bandPower[i] = power;
        const float powerDb = 10.0f * static_cast<float>(log10(static_cast<double>(power)));
        const float floorDb = 10.0f * static_cast<float>(log10(static_cast<double>(bandFloor[i])));
        bandExcess[i] = powerDb - floorDb;
        bandFloorDb[i] = floorDb;
        if (bandExcess[i] < 0.0f)
            bandExcess[i] = 0.0f;
        if (static_cast<unsigned int>(i - 1) < 0x2du)
            speechSum += powerDb;
    }

    if (*frameCount == 0)
    {
        for (i = 0; i < 0x30; ++i)
        {
            bandLevel[i] = bandExcess[i];
            bandSmooth[i] = bandPower[i];
        }
    }

    float excessSum = 0.0f;
    for (i = 1; i < 0x2e; ++i)
        excessSum += bandExcess[i];
    float rate = 0.125f;
    if (excessSum / 45.0f - *nearLevel < 0.0f)
        rate = 0.03125f;
    *nearLevel = (1.0f - rate) * *nearLevel + rate * (excessSum / 45.0f);
    rate = 0.03125f;
    if (speechSum / 45.0f - *farLevel < 0.0f)
        rate = 0.00390625f;
    *farLevel = (1.0f - rate) * *farLevel + rate * (speechSum / 45.0f);
    for (i = 0; i < 0x30; ++i)
        bandLevel[i] = bandLevel[i] * 0.5f + bandExcess[i] * 0.5f;

    if (adaptEnabled == 0)
    {
        for (i = 0; i < 0x30; ++i)
            bandGain[i] = 1.0f;
        spectrum[1] = savedBin;
    }
    else
    {
        float loudest = 0.0f;
        for (i = 0; i < 0x30; ++i)
        {
            if (loudest < bandFloorDb[i])
                loudest = bandFloorDb[i];
        }
        loudest = -loudest;
        float offset = *gainCeiling;
        if (*gainCeiling <= loudest)
        {
            offset = -5.0f;
            if (loudest < -5.0f)
                offset = loudest;
        }
        float gain = 1.0f;
        for (i = 0; i < 0x30; ++i)
        {
            float excess = bandLevel[i] - 2.0f;
            if (excess < 0.0f)
                excess = 0.0f;
            if (0.0f <= excess + offset)
                gain = 1.0f;
            else
                gain = static_cast<float>(pow(10.0, static_cast<double>((excess + offset) / 20.0f)));
            if (i < 0x20)
            {
                float* pair = spectrum + i * 4;
                pair[0] = pair[0] * gain;
                pair[2] = pair[2] * gain;
                pair[1] = pair[1] * gain;
                pair[3] = pair[3] * gain;
            }
            else
            {
                float* group = spectrum + i * 8 - 0x80;
                for (int bin = 0; bin < 8; ++bin)
                    group[bin] = group[bin] * gain;
            }
            bandGain[i] = gain;
        }
        spectrum[1] = savedBin * gain;
    }

    float smoothTotal = 0.0f;
    float powerTotal = 0.0f;
    for (i = 0; i < 0x30; ++i)
    {
        smoothTotal += bandSmooth[i];
        powerTotal += bandPower[i];
    }
    const float mismatch = (smoothTotal - powerTotal) / powerTotal;
    float tracking = 1.0f / (mismatch * mismatch + 1.0f);
    if (tracking < 0.699999988079071f)
        tracking = 0.699999988079071f;
    const bool loudNear = 5.226560115814209f <= *nearLevel;
    *coherence = 0.699999988079071f * *coherence + 0.30000001192092896f * tracking;
    float lowerBound;
    if (loudNear)
    {
        lowerBound = static_cast<float>(pow(10.0, -static_cast<double>(*nearLevel / 10.0f)));
    }
    else
    {
        lowerBound = 0.30000001192092896f;
    }
    const float smoothScale = 0.9599999785423279f * *coherence;
    for (i = 0; i < 0x30; ++i)
    {
        const float floorValue = bandFloor[i];
        float ratio = (bandSmooth[i] - floorValue) / floorValue;
        ratio = smoothScale / (ratio * ratio + 1.0f);
        if (ratio < lowerBound)
            ratio = lowerBound;
        const float squared = ratio * ratio;
        const float smoothed = ratio * bandSmooth[i] + (1.0f - ratio) * bandPower[i];
        bandSmooth[i] = smoothed;
        float memory;
        if (squared <= 0.800000011920929f)
        {
            memory = squared;
            if (memory < 3.052000101888552e-05f)
                memory = 3.052000101888552e-05f;
        }
        else
        {
            memory = 0.800000011920929f;
        }
        const float mean = memory * bandMean[i] + (1.0f - memory) * smoothed;
        const float meanSquare = memory * bandMeanSquare[i] + (1.0f - memory) * smoothed * smoothed;
        float variance = fabsf(-(mean * mean - meanSquare)) / ((floorValue + floorValue) * floorValue);
        bandMean[i] = mean;
        bandMeanSquare[i] = meanSquare;
        if (0.5f < variance)
        {
            variance = 0.5f;
        }
        else if (variance < 9.99999993922529e-09f)
        {
            variance = 9.99999993922529e-09f;
        }
        flatness[i] = variance;
        float attack = (1.0f / variance - 1.809999942779541f) / 0.09500002861022949f;
        if (attack < 0.0019531298894435167f)
            attack = 0.0019531298894435167f;
        float release = (1.0f / variance - 1.3359999656677246f) / 0.3320000171661377f;
        attackShape[i] = 47.25f / attack + 1.0f;
        if (release < 0.0019531298894435167f)
            release = 0.0019531298894435167f;
        releaseShape[i] = 9.0f / release + 1.0f;
    }

    float flatnessSum = 0.0f;
    for (i = 0; i < 0x30; ++i)
        flatnessSum += flatness[i];
    const double meanFlatness = static_cast<double>(flatnessSum / 48.0f);
    const float spread = 2.119999885559082f * static_cast<float>(sqrt(meanFlatness)) + 1.0f;
    for (i = 0; i < 0x30; ++i)
    {
        const float candidate = bandSmooth[i] * attackShape[i] * spread;
        if (candidate < minimumHistory[i])
        {
            minimumHistory[i] = candidate;
            bandPeak[i] = bandSmooth[i] * releaseShape[i] * spread;
            if (*updatePhase == 0x12)
                peakValid[i] = 0;
            else if (*updatePhase != 0)
                peakValid[i] = 1;
        }
    }

    int phase = *updatePhase;
    bool resetPhase = true;
    if (phase == 0x12)
    {
        if (voicedCount[0] < 0xd)
        {
            if (((voicedCount[1] < 0xd) && (voicedCount[2] < 0xd)) || (voicedCount[0] < 0xb))
                *holdEnable = 0;
        }
        else if ((0xc < voicedCount[1]) && (0xc < voicedCount[2]) && (0xc < voicedCount[3]) && (0xc < voicedCount[4])
                 && (0xc < voicedCount[5]))
        {
            *holdEnable = 1;
        }
        voicedCount[5] = voicedCount[4];
        voicedCount[4] = voicedCount[3];
        voicedCount[3] = voicedCount[2];
        voicedCount[2] = voicedCount[1];
        voicedCount[1] = voicedCount[0];
        voicedCount[0] = 0;
        float expansion;
        if (0.029999999329447746 <= meanFlatness)
        {
            if (0.05000000074505806 <= meanFlatness)
            {
                expansion = 1.2000000476837158f;
                if (static_cast<float>(meanFlatness - 0.05999999865889549) < 0.0f)
                    expansion = 2.0f;
            }
            else
            {
                expansion = 4.0f;
            }
        }
        else
        {
            expansion = 8.0f;
        }
        for (i = 0; i < 0x30; ++i)
        {
            float minimum = minimumHistory[i];
            int slot;
            for (slot = 1; slot < 10; ++slot)
            {
                if (minimumHistory[i + slot * 0x30] < minimum)
                    minimum = minimumHistory[i + slot * 0x30];
            }
            if (peakValid[i] == 1)
            {
                const float peak = bandPeak[i];
                if (peak < expansion * minimum && minimum < peak)
                {
                    for (slot = 0; slot < 10; ++slot)
                        minimumHistory[i + slot * 0x30] = peak;
                    minimum = peak;
                }
            }
            peakValid[i] = 0;
            for (slot = 9; slot > 0; --slot)
            {
                minimumHistory[i + slot * 0x30] = minimumHistory[i + (slot - 1) * 0x30];
            }
            minimumHistory[i] = 4194304.0f;
            bandPeak[i] = 4194304.0f;
            if (*holdEnable == 0)
            {
                if (minimum < 0.0625f)
                    minimum = 0.0625f;
                bandFloor[i] = minimum;
            }
        }
        *updatePhase = 0x13;
    }
    else
    {
        if (*holdEnable == 0)
        {
            for (i = 0; i < 0x30; ++i)
            {
                float peak = bandPeak[i];
                if (peak < bandFloor[i])
                {
                    if (peak < 0.0625f)
                        peak = 0.0625f;
                    bandFloor[i] = peak;
                }
            }
        }
        phase = phase + 1;
        *updatePhase = phase;
        if (phase != 0x13)
            resetPhase = false;
    }
    if (resetPhase)
        *updatePhase = 0;

    int counter = *frameCount + 1;
    *frameCount = counter;
    if (counter == 0x7fff)
        *frameCount = 0x7ffe;

    bool voiced = false;
    int energy = 0;
    int tap;
    for (tap = -143; tap < 40; ++tap)
    {
        const int sample = currentSamples[tap];
        energy = energy + sample * sample;
        if (energy < 0)
            energy = 0x7fffffff;
    }
    int scaled[183];
    if (energy == 0x7fffffff)
    {
        for (tap = -143; tap < 40; ++tap)
            scaled[tap + 143] = currentSamples[tap] >> 4;
    }
    else if (energy - 0x100000 < 0)
    {
        for (tap = -143; tap < 40; ++tap)
            scaled[tap + 143] = currentSamples[tap] << 3;
    }
    else
    {
        for (tap = -143; tap < 40; ++tap)
            scaled[tap + 143] = currentSamples[tap];
    }

    int correlation[144];
    for (int lag = 143; lag >= 18; --lag)
    {
        int accumulator = 0;
        for (int j = 0; j < 40; ++j)
            accumulator += scaled[143 + j] * scaled[143 - lag + j];
        correlation[lag] = accumulator;
    }

    static const int kSegmentBase[3] = {17, 35, 71};
    static const int kSegmentSpan[3] = {18, 36, 72};
    int segmentPeak = 0;
    int segmentEnergy = 0;
    for (int segment = 0; segment < 3; ++segment)
    {
        int bestLag = kSegmentSpan[segment] + kSegmentBase[segment];
        int bestValue = -10000;
        for (int step = kSegmentSpan[segment]; step > 0; --step)
        {
            const int value = correlation[step + kSegmentBase[segment]];
            if (value - bestValue >= 0)
            {
                bestLag = step + kSegmentBase[segment];
                bestValue = value;
            }
        }
        int lagEnergy = 0;
        for (int j = 0; j < 40; ++j)
        {
            const int sample = scaled[143 - bestLag + j];
            lagEnergy += sample * sample;
        }
        lagEnergy = (lagEnergy + 0x8000) >> 16;
        segmentPeak = bestValue;
        segmentEnergy = lagEnergy;
        if (segment < 2)
        {
            if (0 < lagEnergy && 0 < bestValue + lagEnergy * -0xa666)
                voiced = true;
        }
    }
    if ((0 < segmentEnergy && 0 < segmentPeak + segmentEnergy * -0xa666) || voiced)
    {
        voicedCount[0] = voicedCount[0] + 1;
    }

    for (i = 0; i < 0xa0; ++i)
        sampleHistory[i] = sampleHistory[i + 0x80];
}

void TMC_HF_FUNC_015(void* handle, float* state, float* spectrumA, float* spectrumB, float* alias)
{
    const float* const noiseState = tmcFloat(handle, kTmcNoiseState);
    const float* const noiseGain = tmcFloat(handle, kTmcNoiseGain);
    const short mode = *tmcShort(handle, kTmcEchoMode);

    float leakage;
    if (mode == 0)
        leakage = 0.25f;
    else if (mode == 1)
        leakage = 1.0f;
    else
        leakage = 2.0f;

    float bandThreshold[128];
    float powerA[129];
    float powerB[129];
    float bandEnergyA[45];
    float bandEnergyB[45];
    int i;

    for (i = 0; i < 0x20; ++i)
    {
        const float gain = noiseGain[i];
        const float value = noiseState[i] * (gain * gain);
        bandThreshold[i * 2] = value;
        bandThreshold[i * 2 + 1] = value;
    }
    for (i = 0; i < 0x10; ++i)
    {
        const float gain = noiseGain[i + 0x20];
        const float value = noiseState[i + 0x20] * (gain * gain);
        bandThreshold[0x40 + i * 4] = value;
        bandThreshold[0x40 + i * 4 + 1] = value;
        bandThreshold[0x40 + i * 4 + 2] = value;
        bandThreshold[0x40 + i * 4 + 3] = value;
    }
    for (i = 0; i < 0x80; ++i)
    {
        const float imagB = spectrumB[i * 2 + 1];
        const float imagA = spectrumA[i * 2 + 1];
        const float realB = spectrumB[i * 2];
        const float realA = spectrumA[i * 2];
        powerB[i] = realB * realB + imagB * imagB;
        powerA[i] = realA * realA + imagA * imagA;
    }
    powerB[0x80] = powerB[0];
    powerA[0x80] = powerA[0];

    for (i = 0; i < 0x2d; ++i)
    {
        const int lower = kTmcBandLowerBin[i];
        const float lowerWeight = kTmcBandLowerWeight[i];
        const int upper = kTmcBandUpperBin[i];
        const float upperWeight = kTmcBandUpperWeight[i];
        int bin;
        float sumA = powerA[lower + 1] * lowerWeight;
        for (bin = lower + 1; bin <= upper - 1; ++bin)
            sumA += powerA[bin];
        sumA = powerA[upper + 1] * upperWeight + sumA;
        bandEnergyA[i] = sumA;
        if (sumA < 1.0f)
            bandEnergyA[i] = 1.0f;
        float sumB = powerB[lower + 1] * lowerWeight;
        for (bin = lower + 1; bin <= upper - 1; ++bin)
            sumB += powerB[bin];
        sumB = powerB[upper + 1] * upperWeight + sumB;
        bandEnergyB[i] = sumB;
        if (sumB < 1.0f)
            bandEnergyB[i] = 1.0f;
    }

    float* const smoothA = state;
    float* const smoothB = state + 0x2d;
    float* const bandSuppression = state + 0x35e;
    float* const binSuppression = state + 0x38b;

    for (i = 0; i < 0x2c; ++i)
    {
        const float memory = kTmcGainMemory[i];
        const float update = kTmcGainUpdate[i];
        float energyA = bandEnergyA[i];
        const float trackedA = smoothA[i] * memory + energyA * update;
        smoothA[i] = trackedA;
        if (energyA < trackedA)
        {
            bandEnergyA[i] = trackedA;
            energyA = trackedA;
        }
        float energyB = bandEnergyB[i];
        const float trackedB = smoothB[i] * memory + energyB * update;
        smoothB[i] = trackedB;
        if (energyB < trackedB)
        {
            bandEnergyB[i] = trackedB;
            energyB = trackedB;
        }
        if (energyB <= energyA * leakage)
        {
            bandSuppression[i] = -((1.0f - bandSuppression[i]) * 0.6000000238418579f - 1.0f);
        }
        else
        {
            bandSuppression[i] = 0.0f;
        }
    }
    bandSuppression[0x2c] = 0.0f;

    for (i = 0; i < 0x7d; ++i)
    {
        const int band = kTmcBinBand[i];
        const float lower = bandSuppression[band];
        float gain = binSuppression[i] * 0.009999999776482582f
                     + (kTmcBinWeight[i] * (bandSuppression[band + 1] - lower) + lower) * 0.9900000095367432f;
        if (gain < 0.20000000298023224f)
            gain = 0.20000000298023224f;
        float* const echo = spectrumB + i * 2;
        float* const mix = spectrumA + i * 2;
        if (bandThreshold[i] < powerA[i])
        {
            const float ratio = bandThreshold[i] / (powerA[i] + 1.0f);
            const float leakReal = mix[0] * ratio;
            echo[0] = leakReal;
            const float leakImag = mix[1] * ratio;
            echo[1] = leakImag;
            mix[0] = (mix[0] - leakReal) * gain;
            mix[1] = (mix[1] - leakImag) * gain;
        }
        else
        {
            echo[0] = mix[0];
            echo[1] = mix[1];
            mix[0] = 0.0f * gain;
            mix[1] = 0.0f * gain;
        }
        binSuppression[i] = gain;
    }

    for (i = 0xfa; i < 0x100; ++i)
    {
        spectrumB[i] = 0.0f;
        spectrumA[i] = 0.0f;
    }
    for (i = 0; i < 0x2d; ++i)
    {
        smoothA[i] = bandEnergyA[i];
        smoothB[i] = bandEnergyB[i];
    }
}

void TMC_HF_FUNC_002(void* handle, float* state, float* levels, float* mix, float* residual)
{
    float* const spectrumA = state + 0x5a;
    float* const spectrumB = state + 0x15c;
    float* const historyA = state + 0x25e;
    float* const historyB = state + 0x29e;
    float* const overlapA = state + 0x2de;
    float* const overlapB = state + 0x31e;
    int i;

    for (i = 0; i < 0x40; ++i)
    {
        spectrumA[i] = historyA[i];
        spectrumB[i] = historyB[i];
    }
    for (i = 0; i < 0x40; ++i)
    {
        historyA[i] = mix[0x40 + i];
        historyB[i] = spectrumB[0x80 + i];
    }
    for (i = 0; i < 0x80; ++i)
        spectrumA[0x40 + i] = mix[i];
    for (i = 0xc0; i < 0x100; ++i)
    {
        spectrumA[i] = 0.0f;
        spectrumB[i] = 0.0f;
    }

    float noiseFloor = static_cast<float>(-static_cast<int>(*tmcShort(handle, kTmcNoiseLevel)));
    if (-18.0f <= noiseFloor)
    {
        if (-6.0f - noiseFloor < 0.0f)
            noiseFloor = -6.0f;
        *tmcFloat(handle, kTmcNoiseFloor) = noiseFloor;
    }
    else
    {
        *tmcFloat(handle, kTmcNoiseFloor) = -18.0f;
    }

    TMC_HF_FUNC_040(spectrumB);
    const float nyquistB = spectrumB[1];
    spectrumB[1] = 0.0f;
    spectrumB[0x100] = nyquistB;
    spectrumB[0x101] = 0.0f;
    TMC_HF_FUNC_040(spectrumA);
    spectrumA[0x100] = spectrumA[1];
    spectrumA[1] = 0.0f;
    spectrumA[0x101] = 0.0f;

    for (i = 0; i < 0x80; ++i)
    {
        const float realPart = spectrumB[i * 2];
        const float imagPart = spectrumB[i * 2 + 1];
        levels[2] = levels[2] * 0.9950000047683716f
                    + (imagPart * imagPart + realPart * realPart) * 0.004999999888241291f;
    }
    for (i = 0; i < 0x100; ++i)
    {
        const float scaledB = spectrumB[i] * 0.00390625f;
        spectrumB[i] = scaledB + scaledB;
        const float scaledA = spectrumA[i] * 0.00390625f;
        spectrumA[i] = scaledA + scaledA;
    }

    const unsigned short flags = *tmcUShort(handle, kTmcFlags);
    spectrumB[1] = 0.0f;
    spectrumA[1] = 0.0f;
    TMC_HF_FUNC_010(tmcAt(handle, kTmcNoiseState), mix, spectrumA, (flags & 2) != 0 ? 1 : 0);

    for (i = 0; i < 0x80; ++i)
    {
        const float realPart = spectrumA[i * 2];
        const float imagPart = spectrumA[i * 2 + 1];
        levels[3] = levels[3] * 0.9950000047683716f
                    + (realPart * 256.0f * 0.5f * realPart * 256.0f * 0.5f
                       + imagPart * 256.0f * 0.5f * imagPart * 256.0f * 0.5f)
                          * 0.004999999888241291f;
    }

    if ((*tmcUShort(handle, kTmcFlags) & 1) == 0)
    {
        for (i = 0; i < 0x100; ++i)
            spectrumB[i] = 0.0f;
    }
    else
    {
        TMC_HF_FUNC_015(handle, state, spectrumA, spectrumB, spectrumA);
    }

    for (i = 0; i < 0x80; ++i)
    {
        const float realPart = spectrumA[i * 2];
        const float imagPart = spectrumA[i * 2 + 1];
        levels[0] = levels[0] * 0.9950000047683716f
                    + (realPart * 256.0f * 0.5f * realPart * 256.0f * 0.5f
                       + imagPart * 256.0f * 0.5f * imagPart * 256.0f * 0.5f)
                          * 0.004999999888241291f;
    }

    TMC_HF_FUNC_043(spectrumB);
    TMC_HF_FUNC_043(spectrumA);

    for (i = 0; i < 0x40; ++i)
    {
        const float rising = kTmcBandWindow[i];
        const float falling = kTmcBandWindow[0x3f - i];
        mix[i] = overlapA[i] * falling + spectrumA[i] * rising;
        residual[i] = overlapB[i] * falling + spectrumB[i] * rising;
    }
    for (i = 0; i < 0x40; ++i)
    {
        overlapA[i] = spectrumA[0x80 + i];
        overlapB[i] = spectrumB[0x80 + i];
    }
    for (i = 0x40; i < 0x80; ++i)
    {
        mix[i] = spectrumA[i];
        residual[i] = spectrumB[i];
    }
}

void TMC_HF_FUNC_001(void* handle, float* signal)
{
    float* const echo = tmcFloat(handle, kTmcEchoEstimate);
    float* const echoCopy = tmcFloat(handle, kTmcEchoDelayed);
    for (int i = 0; i < 0x80; ++i)
        echoCopy[i] = echo[i];
    TMC_HF_FUNC_049(tmcDispatch(handle), handle, tmcFloat(handle, kTmcAnalysisState), tmcFloat(handle, kTmcLevelState),
                    signal, tmcFloat(handle, kTmcResidualSpectrum));
    if ((*tmcUShort(handle, kTmcFlags) & 1) != 0)
    {
        TMC_HF_FUNC_047(tmcDispatch(handle), tmcFloat(handle, kTmcLevelState), echo, signal);
    }
}

void TMC_HF_FUNC_003(void* handle, const short* micHistory, const short* referenceHistory, const float* nearInput,
                     float* nearOutput, short count, int echoMode)
{
    float* const analysisMic = tmcFloat(handle, kTmcAnalysisMic);
    float* const analysisRef = tmcFloat(handle, kTmcAnalysisRef);
    float* const analysisNear = tmcFloat(handle, kTmcAnalysisNear);
    float* const synthesisNear = tmcFloat(handle, kTmcSynthesisNear);
    float* const synthesisEcho = tmcFloat(handle, kTmcSynthesisEcho);
    float* const farSpectrumMic = tmcFloat(handle, kTmcFarSpectrumMic);
    float* const farSpectrumRef = tmcFloat(handle, kTmcFarSpectrumRef);
    float* const longTaps = tmcFloat(handle, kTmcLongTaps);
    float* const shortTaps = tmcFloat(handle, kTmcShortTaps);
    float* const stepSize = tmcFloat(handle, kTmcStepSize);
    float* const echoEstimate = tmcFloat(handle, kTmcEchoEstimate);
    float* const overlapNear = tmcFloat(handle, kTmcOverlapNear);
    float* const overlapEcho = tmcFloat(handle, kTmcOverlapEcho);
    const float* const levelTracker = tmcFloat(handle, kTmcLevelTracker);
    short* const activeDelay = tmcShort(handle, kTmcActiveDelay);
    short* const lockedDelay = tmcShort(handle, kTmcLockedDelay);
    short* const delayLocked = tmcShort(handle, kTmcDelayLocked);
    short* const delayRing = tmcShort(handle, kTmcDelayLine);
    short* const delayWriteIndex = tmcShort(handle, kTmcDelayWriteIndex);
    short* const delayHistogram = tmcShort(handle, kTmcDelayHistogram);

    int peak = 0;
    int i;
    for (i = 0; i < 0x80; ++i)
    {
        short magnitude = static_cast<short>(static_cast<int>(nearInput[0x40 + i]));
        if (magnitude < 0)
            magnitude = static_cast<short>(-(magnitude + 1));
        peak |= magnitude;
    }
    short frameLimit;
    if (echoMode == 0)
    {
        frameLimit = 30000;
    }
    else
    {
        frameLimit = 5000;
        if (echoMode == 1)
            frameLimit = 10000;
    }
    const bool quietFrame = peak < 5000;

    short frameHits[75];
    float bandVector[40];
    float subtracted[66];
    float windowedMic[150];
    float windowedReference[150];
    float windowedNear[130];
    float longEcho[130];
    float nearResidual[130];
    float shortEcho[130];

    for (short offset = 0; offset < count; offset = static_cast<short>(offset + 0x40))
    {
        int band;
        for (band = 0; band < 0x80; ++band)
        {
            float* const historyMic = analysisMic + band * 3;
            float* const historyRef = analysisRef + band * 3;
            float* const historyNear = analysisNear + band * 3;
            historyMic[2] = historyMic[1];
            historyMic[1] = historyMic[0];
            historyRef[2] = historyRef[1];
            historyRef[1] = historyRef[0];
            historyNear[2] = historyNear[1];
            historyNear[1] = historyNear[0];
            const int tap = (offset - band) + 0x7f;
            historyMic[0] = static_cast<float>(static_cast<int>(micHistory[tap]));
            historyRef[0] = static_cast<float>(static_cast<int>(referenceHistory[tap]));
            historyNear[0] = nearInput[tap];
            float rising;
            float falling;
            if (band < 0x40)
            {
                rising = kTmcAnalysisWindow[band * 2];
                falling = kTmcAnalysisWindow[band * 2 + 1];
            }
            else
            {
                rising = kTmcAnalysisWindow[0xff - band * 2];
                falling = kTmcAnalysisWindow[0xfe - band * 2];
            }
            windowedMic[band] = historyMic[0] * rising + historyMic[2] * falling;
            windowedReference[band] = historyRef[0] * rising + historyRef[2] * falling;
            windowedNear[band] = historyNear[0] * rising + historyNear[2] * falling;
        }

        TMC_HF_FUNC_033(windowedMic);
        TMC_HF_FUNC_033(windowedReference);
        TMC_HF_FUNC_033(windowedNear);

        for (i = 0; i < 0x82; ++i)
        {
            const float scaledReference = windowedReference[i] * 0.0078125f;
            const float scaledMic = windowedMic[i] * 0.0078125f;
            const float scaledNear = windowedNear[i] * 0.0078125f;
            windowedMic[i] = (scaledMic + scaledMic) * 3.0f;
            windowedReference[i] = (scaledReference + scaledReference) * 3.0f;
            windowedNear[i] = scaledNear + scaledNear;
        }
        windowedMic[0x80] = windowedMic[1];
        windowedMic[0x81] = 0.0f;
        windowedMic[1] = 0.0f;
        windowedReference[0x80] = windowedReference[1];
        windowedReference[0x81] = 0.0f;
        windowedReference[1] = 0.0f;
        windowedNear[0x80] = windowedNear[1];
        windowedNear[0x81] = 0.0f;
        windowedNear[1] = 0.0f;

        for (band = 0x40; band >= 0; --band)
        {
            float* const slotMic = farSpectrumMic + band * 150;
            float* const slotRef = farSpectrumRef + band * 150;
            for (int entry = 74; entry >= 1; --entry)
            {
                slotMic[entry * 2] = slotMic[entry * 2 - 2];
                slotMic[entry * 2 + 1] = slotMic[entry * 2 - 1];
                slotRef[entry * 2] = slotRef[entry * 2 - 2];
                slotRef[entry * 2 + 1] = slotRef[entry * 2 - 1];
            }
            slotMic[0] = windowedMic[band * 2];
            slotMic[1] = windowedMic[band * 2 + 1];
            slotRef[0] = windowedReference[band * 2];
            slotRef[1] = windowedReference[band * 2 + 1];
        }

        for (i = 0; i < 0x4b; ++i)
            frameHits[i] = 0;

        for (band = 0; band <= 0x40; ++band)
        {
            const bool longBand = ((band & 1) == 0) && (band < 0x21);
            float* const longBandTaps = longTaps + band * 150;
            float* const shortBandTaps = shortTaps + band * 40;
            const int delayOffset = static_cast<int>(*activeDelay) * 2;
            const float* const delayedMic = farSpectrumMic + band * 150 + delayOffset;
            const float* const delayedRef = farSpectrumRef + band * 150 + delayOffset;

            if (longBand)
            {
                for (i = 0; i < 150; ++i)
                {
                    windowedReference[i] = farSpectrumMic[band * 150 + i] * 0.5f
                                           + farSpectrumRef[band * 150 + i] * 0.5f;
                }
                float longReal = 0.0f;
                float longImag = 0.0f;
                for (i = 0; i < 0x4b; ++i)
                {
                    longImag = longImag + longBandTaps[i * 2 + 1] * windowedReference[i * 2]
                               + longBandTaps[i * 2] * windowedReference[i * 2 + 1];
                    longReal = longReal + longBandTaps[i * 2] * windowedReference[i * 2]
                               - longBandTaps[i * 2 + 1] * windowedReference[i * 2 + 1];
                }
                longEcho[band * 2] = longReal * 3.0517578125e-05f;
                longEcho[band * 2 + 1] = longImag * 3.0517578125e-05f;
            }

            float shortReal = 0.0f;
            float shortImag = 0.0f;
            for (i = 0; i < 10; ++i)
            {
                shortImag = shortImag + shortBandTaps[i * 4 + 1] * delayedMic[i * 2]
                            + shortBandTaps[i * 4] * delayedMic[i * 2 + 1]
                            + shortBandTaps[i * 4 + 3] * delayedRef[i * 2]
                            + shortBandTaps[i * 4 + 2] * delayedRef[i * 2 + 1];
                shortReal = shortReal + shortBandTaps[i * 4] * delayedMic[i * 2]
                            - shortBandTaps[i * 4 + 1] * delayedMic[i * 2 + 1]
                            + shortBandTaps[i * 4 + 2] * delayedRef[i * 2]
                            - shortBandTaps[i * 4 + 3] * delayedRef[i * 2 + 1];
            }
            shortEcho[band * 2] = shortReal * 3.0517578125e-05f;
            shortEcho[band * 2 + 1] = shortImag * 3.0517578125e-05f;

            if (longBand)
            {
                subtracted[band * 2] = windowedNear[band * 2] - longEcho[band * 2];
                subtracted[band * 2 + 1] = windowedNear[band * 2 + 1] - longEcho[band * 2 + 1];
            }
            nearResidual[band * 2] = windowedNear[band * 2] - shortEcho[band * 2];
            nearResidual[band * 2 + 1] = windowedNear[band * 2 + 1] - shortEcho[band * 2 + 1];

            if (longBand)
            {
                for (i = 0; i < 0x4b; ++i)
                {
                    windowedMic[i * 2] = windowedReference[i * 2];
                    windowedMic[i * 2 + 1] = -windowedReference[i * 2 + 1];
                }
            }

            if (band < 0x21)
            {
                const float step = stepSize[band];
                for (i = 0; i < 10; ++i)
                {
                    bandVector[i * 4] = step * delayedMic[i * 2];
                    bandVector[i * 4 + 1] = -step * delayedMic[i * 2 + 1];
                    bandVector[i * 4 + 2] = step * delayedRef[i * 2];
                    bandVector[i * 4 + 3] = -step * delayedRef[i * 2 + 1];
                }
            }
            else
            {
                for (i = 0; i < 10; ++i)
                {
                    bandVector[i * 4] = delayedMic[i * 2];
                    bandVector[i * 4 + 1] = -delayedMic[i * 2 + 1];
                    bandVector[i * 4 + 2] = delayedRef[i * 2];
                    bandVector[i * 4 + 3] = -delayedRef[i * 2 + 1];
                }
            }

            float longNorm = 0.0f;
            if (longBand)
            {
                float accumulator = 0.0f;
                for (i = 0; i < 0x4b; ++i)
                {
                    accumulator = accumulator + windowedReference[i * 2] * windowedMic[i * 2]
                                  - windowedReference[i * 2 + 1] * windowedMic[i * 2 + 1];
                }
                longNorm = fabsf(accumulator * 0.125f * 1.52587890625e-05f + 1.0f);
            }

            float shortAccumulator = 0.0f;
            for (i = 0; i < 10; ++i)
            {
                shortAccumulator = shortAccumulator + delayedMic[i * 2] * bandVector[i * 4]
                                   - delayedMic[i * 2 + 1] * bandVector[i * 4 + 1]
                                   + delayedRef[i * 2] * bandVector[i * 4 + 2]
                                   - delayedRef[i * 2 + 1] * bandVector[i * 4 + 3];
            }
            const float shortNorm = fabsf(shortAccumulator * 0.125f * 1.52587890625e-05f + 1.0f);

            if (longBand && 1.0f < longNorm)
            {
                for (i = 0; i < 0x4b; ++i)
                {
                    windowedMic[i * 2] = windowedMic[i * 2] / longNorm;
                    windowedMic[i * 2 + 1] = windowedMic[i * 2 + 1] / longNorm;
                }
            }
            if (1.0f < shortNorm)
            {
                for (i = 0; i < 0x14; ++i)
                {
                    bandVector[i * 2] = bandVector[i * 2] / shortNorm;
                    bandVector[i * 2 + 1] = bandVector[i * 2 + 1] / shortNorm;
                }
            }

            if (quietFrame)
            {
                if (longBand)
                {
                    const float errorReal = subtracted[band * 2];
                    const float errorImag = subtracted[band * 2 + 1];
                    for (i = 0; i < 0x4b; ++i)
                    {
                        const float update = (windowedMic[i * 2 + 1] * errorReal + windowedMic[i * 2] * errorImag)
                                             * 0.0625f;
                        longBandTaps[i * 2] = (windowedMic[i * 2] * errorReal - windowedMic[i * 2 + 1] * errorImag)
                                                  * 0.0625f
                                              + longBandTaps[i * 2];
                        longBandTaps[i * 2 + 1] = update + longBandTaps[i * 2 + 1];
                    }
                }
                const float errorReal = nearResidual[band * 2];
                const float errorImag = nearResidual[band * 2 + 1];
                for (i = 0; i < 0x14; ++i)
                {
                    const float update = (bandVector[i * 2 + 1] * errorReal + bandVector[i * 2] * errorImag) * 0.0625f;
                    shortBandTaps[i * 2] = (bandVector[i * 2] * errorReal - bandVector[i * 2 + 1] * errorImag) * 0.0625f
                                           + shortBandTaps[i * 2];
                    shortBandTaps[i * 2 + 1] = update + shortBandTaps[i * 2 + 1];
                }
            }

            if (band < 0x21)
            {
                if ((band & 1) == 0)
                {
                    int strongest = 0;
                    double strongestEnergy = 0.0;
                    for (i = 0; i < 0x4b; ++i)
                    {
                        const float imagPart = longBandTaps[i * 2 + 1];
                        const float realPart = longBandTaps[i * 2];
                        const double magnitude = static_cast<double>(realPart * realPart + imagPart * imagPart);
                        if (strongestEnergy < magnitude)
                        {
                            strongest = i;
                            strongestEnergy = magnitude;
                        }
                    }
                    frameHits[strongest] = static_cast<short>(frameHits[strongest] + 1);
                }
                if (fabs(static_cast<double>(shortAccumulator * 0.125f))
                    <= static_cast<double>(static_cast<int>(frameLimit)))
                {
                    if (stepSize[band] < 0.5f)
                    {
                        const float grown = stepSize[band] * 1.0048828125f;
                        stepSize[band] = grown;
                        if (1.0f < grown)
                            stepSize[band] = 1.0f;
                    }
                }
                else
                {
                    const float shrunk = stepSize[band] * 0.970703125f;
                    stepSize[band] = shrunk;
                    if (shrunk < 0.0010000000474974513f)
                        stepSize[band] = 0.0010000000474974513f;
                }
            }

            if (*levelTracker < 12.0f)
            {
                float refreshReal = 0.0f;
                float refreshImag = 0.0f;
                for (i = 0; i < 10; ++i)
                {
                    refreshImag = refreshImag + shortBandTaps[i * 4 + 1] * delayedMic[i * 2]
                                  + shortBandTaps[i * 4] * delayedMic[i * 2 + 1]
                                  + shortBandTaps[i * 4 + 3] * delayedRef[i * 2]
                                  + shortBandTaps[i * 4 + 2] * delayedRef[i * 2 + 1];
                    refreshReal = refreshReal + shortBandTaps[i * 4] * delayedMic[i * 2]
                                  - shortBandTaps[i * 4 + 1] * delayedMic[i * 2 + 1]
                                  + shortBandTaps[i * 4 + 2] * delayedRef[i * 2]
                                  - shortBandTaps[i * 4 + 3] * delayedRef[i * 2 + 1];
                }
                shortEcho[band * 2] = refreshReal * 3.0517578125e-05f;
                shortEcho[band * 2 + 1] = refreshImag * 3.0517578125e-05f;
                nearResidual[band * 2] = windowedNear[band * 2] - shortEcho[band * 2];
                nearResidual[band * 2 + 1] = windowedNear[band * 2 + 1] - shortEcho[band * 2 + 1];
            }
        }

        short bestCount = 0;
        int bestTap = 0;
        for (i = 0; i < 0x4b; ++i)
        {
            if (bestCount < frameHits[i])
            {
                bestTap = i;
                bestCount = frameHits[i];
            }
        }
        const int oldest = static_cast<int>(delayRing[*delayWriteIndex]);
        if (oldest >= 0)
        {
            const short occupancy = delayHistogram[oldest];
            if (occupancy > 0)
                delayHistogram[oldest] = static_cast<short>(occupancy - 1);
        }
        delayHistogram[bestTap] = static_cast<short>(delayHistogram[bestTap] + 1);
        const short writeIndex = *delayWriteIndex;
        *delayWriteIndex = static_cast<short>(writeIndex + 1);
        delayRing[writeIndex] = static_cast<short>(bestTap);
        if (*delayLocked == 0)
        {
            if (*delayWriteIndex > 0x31)
                *delayWriteIndex = 0;
        }
        else if (*delayWriteIndex > 199)
        {
            *delayWriteIndex = 0;
        }

        short leaderCount = 0;
        int leader = 0;
        for (i = 0; i < 0x4b; ++i)
        {
            const short occupancy = delayHistogram[i];
            if (leaderCount < occupancy)
            {
                leader = i;
                leaderCount = occupancy;
            }
        }
        leaderCount = delayHistogram[leader];
        bool relock = false;
        if (*delayLocked == 0)
        {
            if (leaderCount > 0x27)
                relock = true;
        }
        else if (leaderCount > 0x9f && (*lockedDelay + 2 <= leader || leader < *lockedDelay))
        {
            relock = true;
        }
        if (relock)
        {
            if (leader < 0x40)
            {
                const short chosen = leader < 2 ? static_cast<short>(leader) : static_cast<short>(leader - 2);
                *activeDelay = chosen;
                *delayLocked = 1;
                *lockedDelay = chosen;
            }
            else
            {
                *activeDelay = 0x40;
                *delayLocked = 1;
                *lockedDelay = 0x40;
            }
        }

        nearResidual[0x81] = 0.0f;
        nearResidual[1] = nearResidual[0x80];
        nearResidual[0x80] = 0.0f;
        shortEcho[0x81] = 0.0f;
        shortEcho[1] = shortEcho[0x80];
        shortEcho[0x80] = 0.0f;
        TMC_HF_FUNC_036(nearResidual);
        TMC_HF_FUNC_036(shortEcho);

        for (band = 0x7f; band >= 0; --band)
        {
            float* const slotNear = synthesisNear + band * 3;
            float* const slotEcho = synthesisEcho + band * 3;
            slotNear[2] = slotNear[1];
            slotNear[1] = slotNear[0];
            slotEcho[2] = slotEcho[1];
            slotEcho[1] = slotEcho[0];
            slotNear[0] = nearResidual[band];
            slotEcho[0] = shortEcho[band];
        }

        int sample;
        for (sample = 0; sample < 0x40; ++sample)
        {
            const float rising = kTmcAnalysisWindow[sample * 2];
            const float falling = kTmcAnalysisWindow[sample * 2 + 1];
            const float* const slotNear = synthesisNear + (0x7f - sample) * 3;
            const float* const slotEcho = synthesisEcho + (0x7f - sample) * 3;
            const float nearTerm = slotNear[0] * rising + slotNear[2] * falling;
            const float echoTerm = slotEcho[0] * rising + slotEcho[2] * falling;
            nearOutput[offset + sample] = overlapNear[sample] + nearTerm + nearTerm;
            echoEstimate[offset + sample] = overlapEcho[sample] + echoTerm + echoTerm;
        }
        for (sample = 0; sample < 0x40; ++sample)
        {
            const float rising = kTmcAnalysisWindow[0x7f - sample * 2];
            const float falling = kTmcAnalysisWindow[0x7e - sample * 2];
            const float* const slotNear = synthesisNear + (0x3f - sample) * 3;
            const float* const slotEcho = synthesisEcho + (0x3f - sample) * 3;
            const float nearTerm = slotNear[0] * rising + slotNear[2] * falling;
            const float echoTerm = slotEcho[0] * rising + slotEcho[2] * falling;
            overlapNear[sample] = nearTerm + nearTerm;
            overlapEcho[sample] = echoTerm + echoTerm;
        }
    }
}

void TMC_HF_FUNC_045(TmcHfDispatch* table)
{
    if (table == 0)
        return;
    const int slot = ++table->index;
    table->entry[slot & 7] = reinterpret_cast<TmcHfEntry>(&TMC_HF_FUNC_003);
    table->entry[(slot + 1) & 7] = reinterpret_cast<TmcHfEntry>(&TMC_HF_FUNC_011);
    table->entry[(slot + 2) & 7] = reinterpret_cast<TmcHfEntry>(&TMC_HF_FUNC_014);
    table->entry[(slot + 3) & 7] = reinterpret_cast<TmcHfEntry>(&TMC_HF_FUNC_002);
    table->entry[(slot + 4) & 7] = reinterpret_cast<TmcHfEntry>(&TMC_HF_FUNC_012);
    table->entry[(slot + 5) & 7] = reinterpret_cast<TmcHfEntry>(&TMC_HF_FUNC_013);
    table->entry[(slot + 6) & 7] = reinterpret_cast<TmcHfEntry>(&TMC_HF_FUNC_001);
    table->entry[(slot + 7) & 7] = reinterpret_cast<TmcHfEntry>(&TMC_HF_FUNC_004);
}

void TMC_HF_FUNC_046(TmcHfDispatch* table, void* handle, const short* micHistory, const short* referenceHistory,
                     const float* nearInput, float* nearOutput, short count, int echoMode)
{
    if (table == 0)
        return;
    typedef void (*Entry)(void*, const short*, const short*, const float*, float*, short, int);
    reinterpret_cast<Entry>(table->entry[table->index & 7])(handle, micHistory, referenceHistory, nearInput, nearOutput,
                                                            count, echoMode);
}

void TMC_HF_FUNC_047(TmcHfDispatch* table, float* state, float* unused, float* signal)
{
    if (table == 0)
        return;
    typedef void (*Entry)(float*, float*, float*);
    reinterpret_cast<Entry>(table->entry[(table->index + 1) & 7])(state, unused, signal);
}

void TMC_HF_FUNC_048(TmcHfDispatch* table, float* state, int target, short* signal)
{
    if (table == 0)
        return;
    typedef void (*Entry)(float*, int, short*);
    reinterpret_cast<Entry>(table->entry[(table->index + 2) & 7])(state, target, signal);
}

void TMC_HF_FUNC_049(TmcHfDispatch* table, void* handle, float* state, float* levels, float* mix, float* residual)
{
    if (table == 0)
        return;
    typedef void (*Entry)(void*, float*, float*, float*, float*);
    reinterpret_cast<Entry>(table->entry[(table->index + 3) & 7])(handle, state, levels, mix, residual);
}

void TMC_HF_FUNC_050(TmcHfDispatch* table, float* state, float* signal)
{
    if (table == 0)
        return;
    typedef void (*Entry)(float*, float*);
    reinterpret_cast<Entry>(table->entry[(table->index + 4) & 7])(state, signal);
}

void TMC_HF_FUNC_051(TmcHfDispatch* table, float* state, const short* source, float* destination, int count)
{
    if (table == 0)
        return;
    typedef void (*Entry)(float*, const short*, float*, int);
    reinterpret_cast<Entry>(table->entry[(table->index + 5) & 7])(state, source, destination, count);
}

void TMC_HF_FUNC_052(TmcHfDispatch* table, void* handle, float* signal)
{
    if (table == 0)
        return;
    typedef void (*Entry)(void*, float*);
    reinterpret_cast<Entry>(table->entry[(table->index + 6) & 7])(handle, signal);
}

void TMC_HF_FUNC_053(TmcHfDispatch* table, short* state, const short* source, short* destination, short* history,
                     int count)
{
    if (table == 0)
        return;
    typedef void (*Entry)(short*, const short*, short*, short*, int);
    reinterpret_cast<Entry>(table->entry[(table->index + 7) & 7])(state, source, destination, history, count);
}

struct TmcHfConfig
{
    void* workspace;
    short flags;
    int echoCancellerMode;
    short noiseSuppressorLevel;
    short agcGainIndex;
    short agcAttenIndex;
    int inputMode;
    int outputMode;
    int error;
};

}

extern "C" void* TMC_HF_Init(void* config)
{
    TmcHfConfig* const settings = static_cast<TmcHfConfig*>(config);
    void* const workspace = settings->workspace;
    if (workspace == 0)
        return 0;

    const short flags = settings->flags;
    int words = 0xe2d;
    if ((flags & 1) != 0)
        words = 0x9441;
    int* const cleared = static_cast<int*>(workspace);
    int i;
    for (i = 0; i < words; ++i)
        cleared[i] = 0;
    *tmcUShort(workspace, kTmcFlags) = static_cast<unsigned short>(flags);

    const int echoMode = settings->echoCancellerMode;
    if (echoMode != 0 && echoMode != 1 && echoMode != 2)
    {
        settings->error = 1;
        return 0;
    }
    *tmcShort(workspace, kTmcEchoMode) = static_cast<short>(echoMode);

    const short noiseLevel = settings->noiseSuppressorLevel;
    if (static_cast<unsigned int>(static_cast<int>(noiseLevel) - 6) >= 9u)
    {
        settings->error = 2;
        return 0;
    }
    *tmcShort(workspace, kTmcNoiseLevel) = noiseLevel;

    const short gainIndex = settings->agcGainIndex;
    if (static_cast<unsigned int>(static_cast<int>(gainIndex)) >= 0x1fu)
    {
        settings->error = 4;
        return 0;
    }
    *tmcUShort(workspace, kTmcAgcGainIndex) = static_cast<unsigned short>(gainIndex);

    const short attenIndex = settings->agcAttenIndex;
    if (static_cast<unsigned int>(static_cast<int>(attenIndex) + 0x27) >= 0x28u)
    {
        settings->error = 8;
        return 0;
    }
    *tmcShort(workspace, kTmcAgcAttenIndex) = attenIndex;

    if ((flags & 1) != 0)
    {
        *tmcFloat(workspace, 0x2C14) = 1.0f;
        *tmcFloat(workspace, 0x2C18) = 1.0f;
        float* const stepSize = tmcFloat(workspace, kTmcStepSize);
        for (i = 0; i < 0x41; ++i)
            stepSize[i] = 1.0f;
        float* const coherence = tmcFloat(workspace, kTmcCoherence);
        for (i = 0; i < 0x41; ++i)
            coherence[i] = 1.0f;
        short* const delayRing = tmcShort(workspace, kTmcDelayLine);
        for (i = 0; i < 0xfa; ++i)
            delayRing[i] = -1;
    }

    const int inputMode = settings->inputMode;
    if (inputMode != 0 && inputMode != 1 && inputMode != 2)
    {
        settings->error = 0x10;
        return 0;
    }
    *tmcInt(workspace, kTmcInputMode) = inputMode;

    const int outputMode = settings->outputMode;
    if (outputMode != 0 && outputMode != 1 && outputMode != 2)
    {
        settings->error = 0x20;
        return 0;
    }
    *tmcInt(workspace, kTmcOutputMode) = outputMode;

    TMC_HF_FUNC_009(tmcFloat(workspace, kTmcNoiseState));
    settings->error = 0;
    return settings->workspace;
}

extern "C" unsigned int TMC_HF_Proc(void* handle, const short* inputLeft, const short* inputRight,
                                    const short* inputReference, short* output)
{
    if (handle == 0)
        return 0x40;
    if (inputLeft == 0)
        return 0x80;
    if (inputRight == 0)
        return 0x100;
    if (inputReference == 0)
        return 0x200;
    if (output == 0)
        return 0x400;

    TmcHfDispatch* const table = tmcDispatch(handle);
    TMC_HF_FUNC_045(table);

    short resampledMic[128];
    short work[128];
    short micHistory[192];
    short referenceHistory[192];
    short leftBand[128];
    short rightBand[128];
    float mix[192];
    int i;

    float* const spectrum = tmcFloat(handle, kTmcMixSpectrum);
    float* const residual = tmcFloat(handle, kTmcResidualSpectrum);

    const int inputMode = *tmcInt(handle, kTmcInputMode);
    if (inputMode == 0)
    {
        TMC_HF_FUNC_005(tmcShort(handle, kTmcMicResampler), inputLeft, resampledMic,
                        tmcShort(handle, kTmcMicResamplerHist), 0x40);
    }
    else if (inputMode == 1)
    {
        for (i = 0; i < 0x80; ++i)
            resampledMic[i] = inputLeft[i];
    }
    else if (inputMode == 2)
    {
        TMC_HF_FUNC_053(table, tmcShort(handle, kTmcMicResampler), inputLeft, resampledMic,
                        tmcShort(handle, kTmcMicResamplerHist), 0x80);
    }
    TMC_HF_FUNC_053(table, tmcShort(handle, kTmcLeftDecimator), inputRight, leftBand,
                    tmcShort(handle, kTmcLeftDecimatorHst), 0x80);
    TMC_HF_FUNC_053(table, tmcShort(handle, kTmcRightDecimator), inputReference, rightBand,
                    tmcShort(handle, kTmcRightDecimatorHt), 0x80);
    TMC_HF_FUNC_051(table, tmcFloat(handle, kTmcMicFilter), resampledMic, spectrum, 0x80);

    unsigned short flags = *tmcUShort(handle, kTmcFlags);
    if ((flags & 3) == 0)
    {
        for (i = 0; i < 0x80; ++i)
            residual[i] = 0.0f;
        for (i = 0; i < 0x80; ++i)
        {
            work[i] = static_cast<short>(TMC_HF_FUNC_044(static_cast<int>(spectrum[i])));
        }
        flags = *tmcUShort(handle, kTmcFlags);
    }
    else
    {
        for (i = 0; i < 0x80; ++i)
            spectrum[i] = spectrum[i] * 0.5f;
        float* const previousMix = tmcFloat(handle, kTmcPreviousMix);
        short* const previousLeft = tmcShort(handle, kTmcPreviousLeft);
        short* const previousRight = tmcShort(handle, kTmcPreviousRight);
        for (i = 0; i < 0x40; ++i)
        {
            mix[i] = previousMix[i];
            micHistory[i] = previousLeft[i];
            referenceHistory[i] = previousRight[i];
        }
        for (i = 0; i < 0x80; ++i)
        {
            mix[0x40 + i] = spectrum[i];
            micHistory[0x40 + i] = leftBand[i];
            referenceHistory[0x40 + i] = rightBand[i];
        }
        for (i = 0; i < 0x40; ++i)
        {
            previousMix[i] = spectrum[0x40 + i];
            previousLeft[i] = leftBand[0x40 + i];
            previousRight[i] = rightBand[0x40 + i];
        }
        if ((flags & 1) == 0)
        {
            float* const echo = tmcFloat(handle, kTmcEchoEstimate);
            for (i = 0; i < 0x80; ++i)
                echo[i] = 0.0f;
            TMC_HF_FUNC_052(table, handle, mix);
        }
        else
        {
            TMC_HF_FUNC_046(table, handle, micHistory, referenceHistory, mix, mix, 0x80,
                            static_cast<int>(*tmcShort(handle, kTmcEchoMode)));
            TMC_HF_FUNC_052(table, handle, mix);
        }
        flags = *tmcUShort(handle, kTmcFlags);
        if ((flags & 3) != 0)
        {
            for (i = 0; i < 0x80; ++i)
                mix[i] = mix[i] + mix[i];
            TMC_HF_FUNC_050(table, tmcFloat(handle, kTmcOutputFilter), mix);
        }
        for (i = 0; i < 0x80; ++i)
        {
            work[i] = static_cast<short>(TMC_HF_FUNC_044(static_cast<int>(mix[i])));
        }
        flags = *tmcUShort(handle, kTmcFlags);
    }

    const short attenIndex = *tmcShort(handle, kTmcAgcAttenIndex);
    const short agcTarget = static_cast<short>(static_cast<int>(kTmcAgcAttenuation[-attenIndex] * 0.5f * 32768.0f));
    if ((flags & 4) != 0)
    {
        TMC_HF_FUNC_048(table, tmcFloat(handle, kTmcAgcFilter), static_cast<int>(agcTarget), work);
    }

    const float gain = kTmcAgcGain[*tmcShort(handle, kTmcAgcGainIndex)];
    for (i = 0; i < 0x80; ++i)
    {
        work[i] = static_cast<short>(
            TMC_HF_FUNC_044(static_cast<int>(static_cast<float>(static_cast<int>(work[i])) * gain)));
    }
    float* const tracker = tmcFloat(handle, kTmcLevelTracker);
    for (i = 0; i < 0x80; ++i)
    {
        *tracker = *tracker * 0.9950000047683716f + fabsf(residual[i]) * 0.004999999888241291f;
    }
    for (i = 0; i < 0x80; ++i)
    {
        const float scaled = residual[i] * gain;
        residual[i] = static_cast<float>(
            static_cast<int>(static_cast<short>(TMC_HF_FUNC_044(static_cast<int>(scaled + scaled)))));
    }
    for (i = 0; i < 0x80; ++i)
    {
        work[i] = static_cast<short>(
            TMC_HF_FUNC_044(static_cast<int>(static_cast<float>(static_cast<int>(work[i])) + residual[i])));
    }

    const int outputMode = *tmcInt(handle, kTmcOutputMode);
    if (outputMode == 0)
    {
        TMC_HF_FUNC_053(table, tmcShort(handle, kTmcOutputResampler), work, output,
                        tmcShort(handle, kTmcOutputResampHist), 0x40);
    }
    else if (outputMode == 1)
    {
        for (i = 0; i < 0x80; ++i)
            output[i] = work[i];
    }
    else if (outputMode == 2)
    {
        TMC_HF_FUNC_005(tmcShort(handle, kTmcOutputResampler), work, output, tmcShort(handle, kTmcOutputResampHist),
                        0x80);
    }
    return 0;
}

extern "C" unsigned int TMC_HF_Close(void* handle)
{
    if (handle == 0)
        return 0x40;
    return 0;
}
