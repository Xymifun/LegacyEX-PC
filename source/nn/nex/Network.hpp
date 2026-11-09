#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NETWORK_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NETWORK_HPP

#include <utility>

#include "nn/nex/ConnectivityManager.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/InstanceTable.hpp"
#include "nn/nex/LiteBuffer.hpp"
#include "nn/nex/PRUDPMessageSelector.hpp"
#include "nn/nex/PacketBufferManager.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/RC4Encryption.hpp"
#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/ThreadVariable.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qProtectedList.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class CompressionAlgorithm;
class HMACChecksum;
class KeyedChecksumAlgorithm;
class MD5ChecksumWithKey;
class NATTraversalEngine;
class PRUDPEndPoint;
class RootTransport;
class SlidingWindow;
class TransportAdapter;

class Network : public RefCountedObject
{
public:
    enum
    {
        ClassID = 1
    };

    enum ECompAlgo
    {
        COMP_ALGO_NONE = 0,
        COMP_ALGO_ZLIB = 1
    };

    Network();
    virtual ~Network();

    static Network* GetInstance()
    {
        unsigned int uiContextId = g_usingInstantiationContext;
        if (g_usingInstantiationContext)
            uiContextId = g_instantiationContextVariable->GetRef();

        InstanceControl* pInstanceControl = g_instanceTable.FindInstance(ClassID, uiContextId);
        void* pInstance = pInstanceControl;
        if (pInstance != 0)
            pInstance = pInstanceControl->GetDelegatorInstance();

        return static_cast<Network*>(pInstance);
    }

    static void AcquireInstance();
    static void ReleaseInstance();

    static bool RegisterTransportAdapter(TransportAdapter* pAdapter);
    static bool UnregisterTransportAdapter();

    static void SetP2PDataPacketDefaultSignatureKey(const unsigned char* pKey, unsigned int uiLength);
    static void SetP2PChecksumAlgorithm(KeyedChecksumAlgorithm* pAlgorithm);

    RootTransport* GetTransport() const { return m_pTransport; }

    PRUDPMessageSelector& GetMessageSelector() { return m_messageSelector; }

    ConnectivityManager& GetConnectivityManager() { return m_connectivityManager; }

    NATTraversalEngine* GetNATTraversalEngine();

    qProtectedList<StationURL>& GetLocalURLs();
    const qProtectedList<StationURL>& GetLocalURLs() const;
    void GetLocalURLs(qList<StationURL>* pURLs) const;

    void AddLocalURL(const StationURL& rURL);
    bool RemoveLocalURL(const StationURL& rURL);
    bool ReplaceLocalURL(const StationURL& rURL);
    bool ReplaceLocalURL(const StationURL& rOldURL, const StationURL& rNewURL);
    void SortAndFilterTargetURLs(qList<StationURL>* pURLs);

    void InitCompression(ECompAlgo algorithm);
    void SetRC4Encryption();
    void InitChecksumAlgorithm();
    void InitP2PChecksumAlgorithmKey();
    void SetP2PDataPacketSessionSignatureKey(const qVector<unsigned char>& rKey);
    void ClearP2PDataPacketSessionSignatureKey();

private:
    friend class PRUDPEndPoint;
    friend class SlidingWindow;

    enum
    {
        DefaultChecksumKeyLength = 16,
        DefaultSignatureKeyCapacity = 32,
        SignatureKeyMinLength = 8,
        SignatureKeyMaxLength = 32
    };

    unsigned int m_unknown10;
    CriticalSection m_criticalSection;
    PseudoSingleton m_instanceControl;
    qProtectedList<StationURL> m_localURLs;
    RootTransport* m_pTransport;
    PacketBufferManager m_packetBufferManager;
    ConnectivityManager m_connectivityManager;
    NATTraversalEngine* m_pNATTraversalEngine;
    PRUDPMessageSelector m_messageSelector;
    RC4Encryption m_rc4Encryption;
    CompressionAlgorithm* m_pCompression;
    HMACChecksum* m_pHMACChecksum;
    MD5ChecksumWithKey* m_pMD5Checksum;
    LiteBuffer<DefaultSignatureKeyCapacity> m_p2pSignatureKey;
    qVector<unsigned int> m_unknown3d0;
    CriticalSection m_unknown3e0;
    qVector<std::pair<void*, unsigned int> > m_unknown42c;
    CriticalSection m_unknown43c;

    static ECompAlgo s_eCompAlgo;
    static int s_iZLibWindowBits;
    static int s_iZLibMemLevel;
    static unsigned int s_uiP2PDataPacketDefaultSignatureKeyLength;
    static unsigned char s_aP2PDataPacketDefaultSignatureKey[DefaultSignatureKeyCapacity];
    static unsigned char s_aDefaultChecksumKey[DefaultChecksumKeyLength];
    static bool s_bEncryptionEnabled;
    static bool s_bHMACChecksumEnabled;
    static bool s_bMD5ChecksumEnabled;
};

}
}

#endif
