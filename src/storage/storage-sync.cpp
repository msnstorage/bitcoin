// Copyright (c) 2014-2017 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <storage/storage-sync.h>

#include <activemasternode.h>
#include <checkpoints.h>
#include <governance.h>
#include <validation.h>
#include <masternode.h>
#include <masternode-payments.h>
#include <masternodeman.h>
#include <netfulfilledman.h>
#include <netmessagemaker.h>
#include <spork.h>
#include <ui_interface.h>
#include <util/system.h>

class CStorageSync;
CStorageSync storageSync;

void CStorageSync::ProcessMessage(CNode* pfrom, const std::string& strCommand, CDataStream& vRecv)
{

}

void CStorageSync::ProcessTick(CConnman& connman)
{
    static int nTick = 0;
    if(nTick++ % 5 != 0) return;
    LogPrintf("CMasternodeSync::ProcessTick -- nTick %d\n", nTick);
}

void CStorageSync::ProcessHeaders(CTransaction& tx)
{
    for (const CTxStorage &txStorage : tx.vstorage) {
        for (size_t i = 0; i < txStorage.vhead.size(); i++) {
            if (mapStorageHeaders.count(txStorage.vhead[i].hash)) {
                LogPrintf("Hash %s exist\n", txStorage.vhead[i].hash.ToString());
            } else {
                mapStorageHeaders.insert(make_pair(txStorage.vhead[i].hash, GetTime()));
            }
        }
    }
}
