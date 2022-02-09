// Copyright (c) 2014-2017 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <storage/storage-sync.h>

#include <netmessagemaker.h>
#include <util/system.h>

class CStorageSync;
CStorageSync storageSync;

void CStorageSync::ProcessMessage(CNode* pfrom, const std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
    if (strCommand == NetMsgType::FHSTAT)
    { //file head status

        uint256 hash;
        uint32_t status;
        vRecv >> hash >> status;
        if (status == 1) {
            connman.PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::FHGET, hash)); //get storage headers
        }
        LogPrint(BCLog::STORAGE, "CStorageSync::ProcessMessage FHSTAT -- hash=%s status=%u peer=%d\n", hash.ToString(), status, pfrom->GetId());
    }
    else if (strCommand == NetMsgType::FH)
    { //file head

        uint256 hash;
        uint256 filehash;
        std::vector<unsigned char> fileData;
        vRecv >> hash >> filehash >> fileData;
        CHeadFile head;

        fs::path HeadFile = GetStorageDir() / "headers" / hash.ToString();
        // open temp output file, and associate with CAutoFile
        const char* c = HeadFile.c_str();

        CDataStream stream(fileData, SER_GETHASH, PROTOCOL_VERSION);
        uint256 tmphash = SerializeHash(stream);

        if (hash == tmphash) {
            if(!boost::filesystem::exists(c)) {

                std::ofstream out(c, std::ios::out | std::ios::binary | std::ios::app);

                std::string binaryHeader = stream.str();

                out << binaryHeader;
                out.close();

                stream >> head;

                std::vector<CHeadFilePartL> vheadl;
                for (size_t i = 0; i < head.vhead.size(); i++) {
                    CHeadFilePartL fhp;
                    fhp.hash = head.vhead[i].hash;
                    fhp.part_begin = head.vhead[i].part_begin;
                    fhp.part_end = head.vhead[i].part_end;
                    fhp.loaded = false;
                    fhp.lasttime = 0;
                    fhp.filehash = filehash;
                    vheadl.push_back(fhp);
                }

                mapFilesPartsForDownoads.insert(std::make_pair(filehash, vheadl));

                mapStorageHeaders.erase(hash);
            } else {
                mapStorageHeaders.erase(hash);
            }
        } else {
            LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- FH error calc expected hash:%s calculate hash:%s\n", hash.ToString(), tmphash.ToString());
        }
    }
    else if (strCommand == NetMsgType::FPART)
    { //file
        try {
            uint256 hash;
            uint256 filehash;
            uint32_t part_begin;
            uint32_t part_end;
            std::vector<unsigned char> vData;

            vRecv >> hash >> filehash >> part_begin >> part_end >> vData;

            LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- FPART filehash:%s hash:%s begin:%u end:%u size:%u\n",
                     filehash.ToString(), hash.ToString(), part_begin, part_end, vData.size());

            fs::path file = GetStorageDir() / "files" / filehash.ToString();
            // open temp output file, and associate with CAutoFile
            const char* c = file.c_str();

            CDataStream stream(vData, SER_GETHASH, PROTOCOL_VERSION);
            uint256 tmphash = SerializeHash(stream);

            if (hash == tmphash) {
                std::ofstream out(c, std::ios::out | std::ios::binary | std::ios::app);
                out.seekp(part_begin, std::ios::beg);
                out.write((const char*)&vData[0], vData.size());
                out.close();

                CHeadFilePartL filePartL;
                filePartL.hash = hash;
                filePartL.filehash = filehash;
                filePartL.part_begin = part_begin;
                filePartL.part_end = part_end;

                if(mapFilesPartsForDownoads.count(filePartL.filehash)) {
                    std::vector<CHeadFilePartL>::iterator it = std::find(mapFilesPartsForDownoads[filehash].begin(), mapFilesPartsForDownoads[filehash].end(), filePartL);
                    LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- FPART downloaded successfully hash:%s size=%u del:%s\n", filePartL.hash.ToString(), mapFilesPartsForDownoads[filePartL.filehash].size(), it->hash.ToString());
                    int index = std::distance(mapFilesPartsForDownoads[filehash].begin(), it);
                    mapFilesPartsForDownoads[filePartL.filehash].erase(mapFilesPartsForDownoads[filePartL.filehash].begin() + index);
                    //vec.erase(find(vec.begin(),vec.end(),value));
                    if (mapFilesPartsForDownoads[filePartL.filehash].empty()) {
                        mapFilesPartsForDownoads.erase(filePartL.filehash);
                        LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- FPART all part downloaded successfully filehash:%s\n", filePartL.filehash.ToString());
                    }
                } else {
                    LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- FPART not found filehash:%s\n", filePartL.filehash.ToString());
                }
            } else {
                LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- FPART error calc expected hash:%s calculate hash:%s\n", hash.ToString(), tmphash.ToString());
            }
        } catch (const std::exception& e) {
            LogPrint(BCLog::STORAGE, "ERROR Exception '%s' \n", e.what());
        }
    }
    else if (strCommand == NetMsgType::CHKFHEAD)
    { //file
        uint256 hash;
        vRecv >> hash;

        LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- CHKFHEAD hash:%s\n",
                 hash.ToString());

        CHeadFileStatus fileHeadStatus;
        fileHeadStatus.hash = hash;
        fileHeadStatus.status = 0;

        fs::path HeadFile = GetStorageDir() / "headers" / hash.ToString();
        // open temp output file, and associate with CAutoFile
        const char* c = HeadFile.c_str();

        if(boost::filesystem::exists(c)) {
            fileHeadStatus.status = 1;
        } else {
            fileHeadStatus.status = 0;
        }

        connman.PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::FHSTAT, fileHeadStatus));
    }
    else if (strCommand == NetMsgType::FHGET)
    { //file
        uint256 hash;
        vRecv >> hash;

        CFH fh;

        LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- FHGET hash:%s\n", hash.ToString());

        fs::path HeadFile = GetStorageDir() / "headers" / hash.ToString();
        // open temp output file, and associate with CAutoFile
        const char* c = HeadFile.c_str();

        if(boost::filesystem::exists(c)) {

            // open the file:
            std::streampos fileSize;
            std::ifstream file(c, std::ios::binary | std::ios::app);

            // get its size:
            file.seekg(0, std::ios::end);
            fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            // read the data:
            std::vector<unsigned char> fileData(fileSize);
            file.read((char*) &fileData[0], fileSize);

            CDataStream stream(fileData, SER_NETWORK, PROTOCOL_VERSION);
            CHeadFile head;
            stream >> head;

            fh.hash = hash;
            fh.data = fileData;
            fh.filehash = head.hash;

            LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- FHGET hash:%s filehash:%s size:%u\n", fh.hash.ToString(), fh.filehash.ToString(), fh.data.size());

            connman.PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::FH, fh));
        }


    }
    else if (strCommand == NetMsgType::FGET)
    { //file
        uint256 hash;
        uint256 filehash;
        uint32_t part_begin;
        uint32_t part_end;
        vRecv >> hash >> part_begin >> part_end >> filehash;

        CFP HFP;

        HFP.hash = hash;
        HFP.filehash = filehash;
        HFP.part_begin = part_begin;
        HFP.part_end = part_end;

        LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- FGET hash:%s filehash:%s part_begin:%u part_end:%u\n", hash.ToString(), filehash.ToString(), part_begin, part_end);

        fs::path HeadFile = GetStorageDir() / "files" / filehash.ToString();
        // open temp output file, and associate with CAutoFile
        const char* c = HeadFile.c_str();

        if(boost::filesystem::exists(c)) {

            // open the file:
            std::streampos fileSize;
            std::ifstream file(c, std::ios::binary);

            // get its size:
            fileSize = HFP.part_end - HFP.part_begin;
            file.seekg(HFP.part_begin, std::ios::beg);

            // read the data:
            std::vector<unsigned char> fileData(fileSize);
            file.read((char*) &fileData[0], fileSize);

            HFP.hash = hash;
            HFP.part_begin = part_begin;
            HFP.part_end = part_end;
            HFP.data = fileData;

            connman.PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::FPART, HFP));
        }

    }
}

void CStorageSync::ProcessTick(CConnman& connman)
{
    static int nTick = 0;

    nTick++;

    if(nTick % 60 == 0) {

        LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- nTick %d waiting headers %d\n", nTick, mapStorageHeaders.size());

        for(std::map<uint256, int64_t>::iterator it = mapStorageHeaders.begin(); it != mapStorageHeaders.end(); ++it) {
            if(it->second + 60 < GetTime()) {
                connman.ForEachNode(CConnman::AllNodes, [&connman, &it](CNode* pnode) {
                    connman.PushMessage(pnode, CNetMsgMaker(pnode->GetSendVersion()).Make(NetMsgType::CHKFHEAD, it->first)); //check storage head
                });
                mapStorageHeaders[it->first] = GetTime();
            }
        }
    }

    if(nTick % 30 == 0) {

        LogPrint(BCLog::STORAGE, "CStorageSync::ProcessTick -- nTick %d waiting files parts %d\n", nTick, mapFilesPartsForDownoads.size());

        for(std::map<uint256, std::vector<CHeadFilePartL>>::iterator it = mapFilesPartsForDownoads.begin(); it != mapFilesPartsForDownoads.end(); ++it) {
            for (size_t i = 0; i < it->second.size(); i++) {
                CHeadFilePartL fhp;
                fhp.hash = it->second[i].hash;
                fhp.filehash = it->first;
                fhp.part_begin = it->second[i].part_begin;
                fhp.part_end = it->second[i].part_end;
                if(!fhp.loaded && fhp.lasttime + 30 < GetTime()) {
                    it->second[i].lasttime = GetTime();
                    connman.ForEachNode(CConnman::AllNodes, [&connman, &fhp](CNode* pnode) {
                        connman.PushMessage(pnode, CNetMsgMaker(pnode->GetSendVersion()).Make(NetMsgType::FGET, fhp)); //get storage file
                    });
                    MilliSleep(100);
                }
            }
        }
    }

}

void CStorageSync::ProcessHeaders(const CTransaction& tx)
{
    for (const CTxStorage &txStorage : tx.vstorage) {
        for (size_t i = 0; i < txStorage.vhead.size(); i++) {
            if (mapStorageHeaders.count(txStorage.vhead[i].hash)) {
                LogPrint(BCLog::STORAGE, "Hash %s exist\n", txStorage.vhead[i].hash.ToString());
            } else {
                mapStorageHeaders.insert(std::make_pair(txStorage.vhead[i].hash, GetTime()));
            }
        }
    }
}
