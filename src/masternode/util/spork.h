// Copyright (c) 2014-2017 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPORK_H
#define SPORK_H

#include <hash.h>
#include <net.h>
#include <util/strencodings.h>

class CSporkDB;

/** Global variable that points to the spork database (protected by cs_main) */
extern std::unique_ptr<CSporkDB> pSporkDB;

class CSporkMessage;
class CSporkManager;

/*
    Don't ever reuse these IDs for other sporks
    - This would result in old clients getting confused about which spork is for what
*/
static const int SPORK_START                                            = 10001;
static const int SPORK_END                                              = 10006;

static const int SPORK_FXTC_START                                    = 94680010;
static const int SPORK_FXTC_END                                      = 94680015;

static const int SPORK_1_MASTERNODE_PAYMENT_ENFORCEMENT                 = 10001;
static const int SPORK_2_SUPERBLOCKS_ENABLED                            = 10002;
static const int SPORK_3_MASTERNODE_PAY_UPDATED_NODES                   = 10003;
static const int SPORK_4_RECONSIDER_BLOCKS                              = 10004;
static const int SPORK_5_OLD_SUPERBLOCK_FLAG                            = 10005;
static const int SPORK_6_REQUIRE_SENTINEL_FLAG                          = 10006;

static const int SPORK_FXTC_02_IGNORE_SLIGHTLY_HIGHER_COINBASE       = 94680010;
static const int SPORK_FXTC_02_IGNORE_FOUNDER_REWARD_VALUE           = 94680011;
static const int SPORK_FXTC_02_IGNORE_MASTERNODE_REWARD_VALUE        = 94680012;
static const int SPORK_FXTC_02_IGNORE_MASTERNODE_REWARD_PAYEE        = 94680013;

static const int SPORK_FXTC_03_BLOCK_REWARD_SMOOTH_HALVING_START     = 94680014;
static const int SPORK_FXTC_03_BLOCK_REWARD_SHAPING_START            = 94680015;

static const int64_t SPORK_1_MASTERNODE_PAYMENT_ENFORCEMENT_DEFAULT     = 4070908800ULL;// OFF
static const int64_t SPORK_2_SUPERBLOCKS_ENABLED_DEFAULT                = 4070908800ULL;// OFF
static const int64_t SPORK_3_MASTERNODE_PAY_UPDATED_NODES_DEFAULT       = 4070908800ULL;// OFF
static const int64_t SPORK_4_RECONSIDER_BLOCKS_DEFAULT                  = 0;            // 0 BLOCKS
static const int64_t SPORK_5_OLD_SUPERBLOCK_FLAG_DEFAULT                = 4070908800ULL;// OFF
static const int64_t SPORK_6_REQUIRE_SENTINEL_FLAG_DEFAULT              = 4070908800ULL;// OFF

static const int64_t SPORK_FXTC_02_IGNORE_SLIGHTLY_HIGHER_COINBASE_DEFAULT  = 4070908800ULL;// OFF
static const int64_t SPORK_FXTC_02_IGNORE_FOUNDER_REWARD_VALUE_DEFAULT      = 4070908800ULL;// OFF
static const int64_t SPORK_FXTC_02_IGNORE_MASTERNODE_REWARD_VALUE_DEFAULT   = 4070908800ULL;// OFF
static const int64_t SPORK_FXTC_02_IGNORE_MASTERNODE_REWARD_PAYEE_DEFAULT   = 4070908800ULL;// OFF

static const int64_t SPORK_FXTC_03_BLOCK_REWARD_SMOOTH_HALVING_START_DEFAULT  = 4070908800ULL;// OFF
static const int64_t SPORK_FXTC_03_BLOCK_REWARD_SHAPING_START_DEFAULT         = 4070908800ULL;// OFF

extern std::map<uint256, CSporkMessage> mapSporks;
extern CSporkManager sporkManager;

//
// Spork classes
// Keep track of all of the network spork settings
//

class CSporkMessage
{
private:
    std::vector<unsigned char> vchSig;

public:
    int nSporkID;
    int64_t nValue;
    int64_t nTimeSigned;

    CSporkMessage(int nSporkID, int64_t nValue, int64_t nTimeSigned) :
        nSporkID(nSporkID),
        nValue(nValue),
        nTimeSigned(nTimeSigned)
        {}

    CSporkMessage() :
        nSporkID(0),
        nValue(0),
        nTimeSigned(0)
        {}


    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(nSporkID);
        READWRITE(nValue);
        READWRITE(nTimeSigned);
        READWRITE(vchSig);
    }

    uint256 GetHash() const
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
        ss << nSporkID;
        ss << nValue;
        ss << nTimeSigned;
        return ss.GetHash();
    }

    bool Sign(std::string strSignKey);
    bool CheckSignature();
    void Relay(CConnman& connman);
};


class CSporkManager
{
private:
    std::vector<unsigned char> vchSig;
    std::string strMasterPrivKey;
    std::map<int, CSporkMessage> mapSporksActive;

public:

    CSporkManager() {}

    void LoadSporksFromDB();
    void ProcessSpork(CNode* pfrom, const std::string& strCommand, CDataStream& vRecv, CConnman& connman);
    void ExecuteSpork(int nSporkID, int nValue);
    bool UpdateSpork(int nSporkID, int64_t nValue, CConnman& connman);

    bool IsSporkActive(int nSporkID);
    int64_t GetSporkValue(int nSporkID);
    int GetSporkIDByName(std::string strName);
    std::string GetSporkNameByID(int nSporkID);

    bool SetPrivKey(std::string strPrivKey);
};

#endif // SPORK_H
