// Copyright (c) 2014-2017 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <storage/storageman.h>
#include <storage/storage.h>
#include <storage/storagedb.h>
#include <netmessagemaker.h>
#include <shutdown.h>

/** Storage manager */
CStorageMan storageman;

std::unique_ptr<CStorageHeadersDB> pStorageHeadersDB = NULL;
std::unique_ptr<CStorageHeadersFilesDB> pStorageHeadersFilesDB = NULL;
std::unique_ptr<CStorageFilesPartsDB> pStorageFilesPartsDB = NULL;

CStorageMan::CStorageMan()
: cs(),
  mapHeaders(),
  mapHeadersFiles(),
  mapFilesParts()
{}

bool CStorageMan::AddHeader(CStorageHead hf, bool status)
{
    LogPrint(BCLog::MASTERNODE, "CStorageMan::Add -- Adding new Storage Head: headhash=%s filehash=%s, size=%i\n", hf.headhash.ToString(), hf.filehash.ToString(), hf.size);

    {
        LOCK(cs);

        if (mapHeaders.size()<500) {
            mapHeaders[std::make_pair(hf.headhash, hf.filehash)] = std::make_pair(hf, status);
        }
        pStorageHeadersDB->WriteHead(std::make_pair(hf.headhash, hf.filehash), hf, status);
        return true;
    }

}

bool CStorageMan::AddHeaderFiles(CHeadFile hf, bool status)
{
    LogPrint(BCLog::MASTERNODE, "CStorageMan::AddHeaderFiles -- Adding new Storage Head Files: hash=%s, parts=%u\n", hf.hash.ToString(), hf.parts);

    {
        LOCK(cs);

        if (mapHeadersFiles.size()<500) {
            mapHeadersFiles[hf.hash] = std::make_pair(hf, status);
        }
        pStorageHeadersFilesDB->WriteHeadFiles(hf.hash, hf, status);
        return true;
    }

}

bool CStorageMan::AddFilesParts(std::pair<std::pair<uint256, uint256>, uint256> hashpair, std::pair<std::vector<unsigned char>, bool> data)
{
    LogPrint(BCLog::MASTERNODE, "CStorageMan::AddFilesParts -- Adding new Storage Files Parts: headerhash=%s, filehash=%s parthash=%s, size=%u\n",
             hashpair.first.first.ToString(), hashpair.first.second.ToString(), hashpair.second.ToString(), data.first.size());

    {
        LOCK(cs);

        if (mapFilesParts.size()<500) {
            mapFilesParts[std::make_pair(std::make_pair(hashpair.first.first, hashpair.first.second), hashpair.second)] = data;
        }
        pStorageFilesPartsDB->WriteFilesParts(hashpair, data);
        return true;
    }

}

void CStorageMan::LoadHeaders()
{

    LogPrintf("CStorageMan::LoadHeaders\n");

    {
        LOCK(cs);
        pStorageHeadersDB->LoadHeaders();
    }

}

void CStorageMan::LoadHeadersFiles()
{

    LogPrintf("CStorageMan::LoadHeadersFiles\n");

    {
        LOCK(cs);
        pStorageHeadersFilesDB->LoadHeadersFiles();
    }

}

void CStorageMan::LoadFilesParts()
{

    LogPrintf("CStorageMan::LoadFilesParts\n");

    {
        LOCK(cs);
        pStorageFilesPartsDB->LoadFilesParts();
    }

}

void CStorageMan::CheckAndRemove(CConnman& connman)
{

    LogPrintf("CStorageMan::CheckAndRemove\n");

    {
        // Need LOCK2 here to ensure consistent locking order because code below locks cs_main
        // in CheckMnbAndUpdateMasternodeList()
        LOCK(cs);

        pStorageHeadersDB->LoadHeaders();

        LogPrintf("CStorageMan::CheckAndRemove mapHeaders=%u\n", mapHeaders.size());

        std::map<std::pair<uint256, uint256>, std::pair<CStorageHead , bool>>::iterator pos;
        for (pos = mapHeaders.begin(); pos != mapHeaders.end(); ++pos) {
            if (!pos->second.second) {
                connman.ForEachNode(CConnman::AllNodes, [&connman, &pos](CNode* pnode) {
                    connman.PushMessage(pnode, CNetMsgMaker(pnode->GetSendVersion()).Make(NetMsgType::CHKFHEAD, pos->first)); //check storage head
                });
            } else {
                mapHeaders.erase(pos);
            }
        }

        pStorageHeadersFilesDB->LoadHeadersFiles();

        LogPrintf("CStorageMan::CheckAndRemove mapHeadersFiles=%u\n", mapHeadersFiles.size());

        std::map<uint256, std::pair<CHeadFile , bool>>::iterator posf;
        for (posf = mapHeadersFiles.begin(); posf != mapHeadersFiles.end(); ++posf) {
            if (posf->second.second) {
                pStorageHeadersFilesDB->WriteHeadFiles(posf->first, posf->second.first, true);
                mapHeadersFiles.erase(posf);
            }
        }

        pStorageFilesPartsDB->LoadFilesParts();

        LogPrintf("CStorageMan::CheckAndRemove mapFilesParts=%u\n", mapFilesParts.size());

        std::map<std::pair<std::pair<uint256, uint256>, uint256>, std::pair<std::vector<unsigned char>, bool>>::iterator itfp;
        for (itfp = mapFilesParts.begin(); itfp != mapFilesParts.end(); ++itfp) {
            if (itfp->second.second) {
                mapFilesParts.erase(itfp);
            } else if (pStorageFilesPartsDB->FilesPartsExists(itfp->first) && !itfp->second.second) {
                if (pStorageHeadersFilesDB->HeadFilesExists(itfp->first.first.second)) {
                    std::pair<CHeadFile , bool> pair;
                    if (pStorageHeadersFilesDB->ReadHeadFiles(itfp->first.first.second, pair)) {
                        for (size_t i = 0; i < pair.first.vhead.size(); i++) {
                            LogPrintf("CStorageMan::CheckAndRemove hash1=%s, hash2=%s\n", pair.first.vhead[i].hash.ToString(), itfp->first.second.ToString());
                            if (pair.first.vhead[i].hash == itfp->first.second) {
                                CHeadFilePartL fhp;
                                fhp.headhash = itfp->first.first.first;
                                fhp.filehash = itfp->first.first.second;
                                fhp.parthash = itfp->first.second;
                                fhp.part_begin = pair.first.vhead[i].part_begin;
                                fhp.part_end = pair.first.vhead[i].part_end;
                                connman.ForEachNode(CConnman::AllNodes, [&connman, &fhp](CNode* pnode) {
                                    connman.PushMessage(pnode, CNetMsgMaker(pnode->GetSendVersion()).Make(NetMsgType::FGET, fhp)); //get storage file
                                });
                                MilliSleep(100);
                                break;
                            }
                        }
                    }
                }
            }
        }

        LogPrintf("CStorageMan::CheckAndRemove storage size=%u\n", pStorageFilesPartsDB->GetSize());
        
    }

}

void CStorageMan::ProcessMessage(CNode* pfrom, const std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
    if(fLiteMode) return; // disable all Masternode specific functionality

    if (strCommand == NetMsgType::FHSTAT)
    { //file head status
        uint256 headhash;
        uint256 filehash;
        uint32_t status;
        vRecv >> headhash >> filehash >> status;
        if (status == 1) {
            connman.PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::FHGET, std::make_pair(headhash, filehash))); //get storage headers
        }
        LogPrint(BCLog::STORAGE, "CStorageMan::FHSTAT -- headhash=%s filehash=%s status=%u peer=%d\n", headhash.ToString(), filehash.ToString(), status, pfrom->GetId());
    }
    else if (strCommand == NetMsgType::FH)
    { //file head
        uint256 headhash;
        uint256 filehash;
        std::vector<unsigned char> fileData;
        vRecv >> headhash >> filehash >> fileData;
        CHeadFile head;

        CDataStream ssObj(fileData, SER_GETHASH, PROTOCOL_VERSION);
        // verify stored checksum matches input data
        uint256 hashTmp = Hash(ssObj.begin(), ssObj.end());

        if (headhash == hashTmp) {
            ssObj >> head;
            {
                LOCK(cs);
                AddHeaderFiles(head, true);
                if (pStorageHeadersDB->HeadExists(std::make_pair(headhash, filehash))) {
                    std::pair<CStorageHead , bool> pair;
                    if (pStorageHeadersDB->ReadHead(std::make_pair(headhash, filehash), pair)) {
                        pair.second = true;
                        pStorageHeadersDB->WriteHead(std::make_pair(headhash, filehash), pair.first, pair.second);
                        mapHeaders[std::make_pair(headhash, filehash)] = pair;

                        LogPrintf("CStorageMan::FH headhash=%s filehash=%s size=%i status=%d\n", pair.first.headhash.ToString(), pair.first.filehash.ToString(), pair.first.size, pair.second);

                        std::pair<std::vector<unsigned char>, bool> data;
                        for (size_t i = 0; i < head.vhead.size(); i++) {
                            data.second = false;
                            AddFilesParts(std::make_pair(std::make_pair(headhash, filehash), head.vhead[i].hash), data);
                        }
                    }
                }
            }
        }
    }
    else if (strCommand == NetMsgType::FPART)
    { //file part
        uint256 headhash;
        uint256 filehash;
        uint256 parthash;
        uint32_t part_begin;
        uint32_t part_end;
        std::vector<unsigned char> fileData;

        vRecv >> headhash >> filehash >> parthash >> part_begin >> part_end >> fileData;

        LogPrint(BCLog::STORAGE, "CStorageMan::FPART headhash=%s filehash=%s parthash=%s begin:%u end:%u size:%u\n",
                 headhash.ToString(), filehash.ToString(), parthash.ToString(), part_begin, part_end, fileData.size());

//        CDataStream ssObj(fileData, SER_GETHASH, PROTOCOL_VERSION);
//        verify stored checksum matches input data
//        uint256 hashTmp = Hash(ssObj.begin(), ssObj.end());
//        if (filehash == hashTmp) {
//        }

        if(pStorageFilesPartsDB->FilesPartsExists(std::make_pair(std::make_pair(headhash, filehash), parthash))) {
            std::pair<std::vector<unsigned char>, bool> data;
            if (pStorageFilesPartsDB->ReadFilesParts(std::make_pair(std::make_pair(headhash, filehash), parthash), data)) {
                data.first = fileData;
                data.second = true;
                pStorageFilesPartsDB->WriteFilesParts(std::make_pair(std::make_pair(headhash, filehash), parthash), data);
                pStorageFilesPartsDB->UpdateSize(data.first.size());
                mapFilesParts[std::make_pair(std::make_pair(headhash, filehash), parthash)] = data;
            }
        } else {
            std::pair<std::vector<unsigned char>, bool> data;
            data.second = false;
            pStorageFilesPartsDB->WriteFilesParts(std::make_pair(std::make_pair(headhash, filehash), parthash), data);
        }
    }
    else if (strCommand == NetMsgType::CHKFHEAD)
    { //file
        uint256 headhash;
        uint256 filehash;
        vRecv >> headhash >> filehash;

        LogPrint(BCLog::STORAGE, "CStorageMan::CHKFHEAD headhash=%s filehash=%s\n", headhash.ToString(), filehash.ToString());

        CHeadFileStatus fileHeadStatus;
        fileHeadStatus.headhash = headhash;
        fileHeadStatus.filehash = filehash;
        fileHeadStatus.status = 0;

        if (pStorageHeadersDB->HeadExists(std::make_pair(headhash, filehash))) {
            fileHeadStatus.status = 1;
        }

        connman.PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::FHSTAT, fileHeadStatus));

    }
    else if (strCommand == NetMsgType::FHGET)
    { //file
        uint256 hash;
        vRecv >> hash;

        LogPrint(BCLog::STORAGE, "CStorageMan::FHGET hash:%s\n", hash.ToString());

        if (pStorageHeadersFilesDB->HeadFilesExists(hash)) {
            std::pair<CHeadFile, bool> pair;
            if (pStorageHeadersFilesDB->ReadHeadFiles(hash, pair)) {
                connman.PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::FH, pair.first));
            }
        }
    }
    else if (strCommand == NetMsgType::FGET)
    { //file
        uint256 headhash;
        uint256 filehash;
        uint256 parthash;
        uint32_t part_begin;
        uint32_t part_end;
        vRecv >> headhash >> filehash >> parthash >> part_begin >> part_end;

        LogPrint(BCLog::STORAGE, "CStorageMan::FGET headhash=%s filehash=%s parthash=%s part_begin:%u part_end:%u\n"
                 , headhash.ToString(), filehash.ToString(), parthash.ToString(), part_begin, part_end);

        if (pStorageFilesPartsDB->FilesPartsExists(std::make_pair(std::make_pair(headhash, filehash), filehash))) {
            std::pair<std::vector<unsigned char>, bool> data;
            pStorageFilesPartsDB->ReadFilesParts(std::make_pair(std::make_pair(headhash, filehash), parthash), data);
            CFP HFP;
            HFP.headhash = headhash;
            HFP.filehash = filehash;
            HFP.parthash = parthash;
            HFP.part_begin = part_begin;
            HFP.part_end = part_end;
            HFP.data = data.first;
            connman.PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::FPART, HFP));
        }

    }
    else if (strCommand == NetMsgType::FSTAT)
    {//file head status
        uint256 hash;
        uint32_t status;
        vRecv >> hash >> status;

        LogPrint(BCLog::STORAGE, "CStorageMan::FSTAT -- hash=%s status=%u peer=%d\n", hash.ToString(), status, pfrom->GetId());
    }
    else if (strCommand == NetMsgType::CHKF)
    { //file
        uint256 headhash;
        uint256 filehash;
        vRecv >> headhash >> filehash;

        LogPrint(BCLog::STORAGE, "CStorageMan::CHKF headhash=%s filehash=%s\n", headhash.ToString(), filehash.ToString());

        CHeadFileStatus fileHeadStatus;
        fileHeadStatus.headhash = headhash;
        fileHeadStatus.filehash = filehash;
        fileHeadStatus.status = 0;

        connman.PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::FSTAT, fileHeadStatus));
    }

}

void CStorageMan::ProcesseTxStorage(const CTransaction& tx)
{
    for (const CTxStorage &txStorage : tx.vstorage) {
        for (size_t i = 0; i < txStorage.vhead.size(); i++) {
            if (pStorageHeadersDB->HeadExists(std::make_pair(txStorage.vhead[i].headhash, txStorage.vhead[i].filehash))) {
                LogPrint(BCLog::STORAGE, "HeadHash=%s FileHash=% exist\n", txStorage.vhead[i].headhash.ToString(), txStorage.vhead[i].filehash.ToString());
            } else {
                AddHeader(txStorage.vhead[i], false);
            }
        }
    }
}

void ThreadCheckStorage(CConnman& connman)
{
    if(fLiteMode) return; // disable all masternode specific functionality

    static bool fOneThread;
    if(fOneThread) return;
    fOneThread = true;

    // Make this thread recognisable as the Storage thread
    RenameThread("storage-check");

    unsigned int nTick = 0;

    while (true)
    {
        MilliSleep(1000);

        if(!ShutdownRequested()) {
            nTick++;

            if(nTick % 60 == 0) {
                storageman.CheckAndRemove(connman);
            }
        }

    }
}