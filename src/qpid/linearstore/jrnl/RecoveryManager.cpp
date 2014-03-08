/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

#include "qpid/linearstore/jrnl/RecoveryManager.h"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include "qpid/linearstore/jrnl/data_tok.h"
#include "qpid/linearstore/jrnl/deq_rec.h"
#include "qpid/linearstore/jrnl/EmptyFilePoolManager.h"
#include "qpid/linearstore/jrnl/enq_map.h"
#include "qpid/linearstore/jrnl/enq_rec.h"
#include "qpid/linearstore/jrnl/jcfg.h"
#include "qpid/linearstore/jrnl/jdir.h"
#include "qpid/linearstore/jrnl/JournalFile.h"
#include "qpid/linearstore/jrnl/JournalLog.h"
#include "qpid/linearstore/jrnl/jrec.h"
#include "qpid/linearstore/jrnl/LinearFileController.h"
#include "qpid/linearstore/jrnl/txn_map.h"
#include "qpid/linearstore/jrnl/txn_rec.h"
#include "qpid/linearstore/jrnl/utils/enq_hdr.h"
#include "qpid/linearstore/jrnl/utils/file_hdr.h"
#include <sstream>
#include <string>
#include <vector>

namespace qpid {
namespace qls_jrnl
{

RecoveryManager::RecoveryManager(const std::string& journalDirectory,
                                 const std::string& queuename,
                                 enq_map& enqueueMapRef,
                                 txn_map& transactionMapRef,
                                 JournalLog& journalLogRef) :
                                                 journalDirectory_(journalDirectory),
                                                 queueName_(queuename),
                                                 enqueueMapRef_(enqueueMapRef),
                                                 transactionMapRef_(transactionMapRef),
                                                 journalLogRef_(journalLogRef),
                                                 journalEmptyFlag_(false),
                                                 firstRecordOffset_(0),
                                                 endOffset_(0),
                                                 highestRecordId_(0ULL),
                                                 highestFileNumber_(0ULL),
                                                 lastFileFullFlag_(false),
                                                 efpFileSize_kib_(0)
{}

RecoveryManager::~RecoveryManager() {}

void RecoveryManager::analyzeJournals(const std::vector<std::string>* preparedTransactionListPtr,
                                      EmptyFilePoolManager* emptyFilePoolManager,
                                      EmptyFilePool** emptyFilePoolPtrPtr) {
    // Analyze file headers of existing journal files
    efpIdentity_t efpIdentity;
    analyzeJournalFileHeaders(efpIdentity);
    *emptyFilePoolPtrPtr = emptyFilePoolManager->getEmptyFilePool(efpIdentity);
    efpFileSize_kib_ = (*emptyFilePoolPtrPtr)->fileSize_kib();

    // Check for file full condition
    lastFileFullFlag_ = endOffset_ == (std::streamoff)(*emptyFilePoolPtrPtr)->fileSize_kib() * 1024;

    if (!journalEmptyFlag_) {

        // Read all records, establish remaining enqueued records
        while (getNextRecordHeader()) {}
        if (inFileStream_.is_open()) {
            inFileStream_.close();
        }

        // Remove leading files which have no enqueued records
        removeEmptyFiles(*emptyFilePoolPtrPtr);

        // Remove all txns from tmap that are not in the prepared list
        if (preparedTransactionListPtr) {
            std::vector<std::string> xidList;
            transactionMapRef_.xid_list(xidList);
            for (std::vector<std::string>::iterator itr = xidList.begin(); itr != xidList.end(); itr++) {
                std::vector<std::string>::const_iterator pitr =
                        std::find(preparedTransactionListPtr->begin(), preparedTransactionListPtr->end(), *itr);
                if (pitr == preparedTransactionListPtr->end()) { // not found in prepared list
                    txn_data_list tdl = transactionMapRef_.get_remove_tdata_list(*itr); // tdl will be empty if xid not found
                    // Unlock any affected enqueues in emap
                    for (tdl_itr i=tdl.begin(); i<tdl.end(); i++) {
                        if (i->enq_flag_) { // enq op - decrement enqueue count
                            fileNumberMap_[i->pfid_]->decrEnqueuedRecordCount();
                        } else if (enqueueMapRef_.is_enqueued(i->drid_, true)) { // deq op - unlock enq record
                            int16_t ret = enqueueMapRef_.unlock(i->drid_);
                            if (ret < enq_map::EMAP_OK) { // fail
                                // enq_map::unlock()'s only error is enq_map::EMAP_RID_NOT_FOUND
                                std::ostringstream oss;
                                oss << std::hex << "_emap.unlock(): drid=0x\"" << i->drid_;
                                throw jexception(jerrno::JERR_MAP_NOTFOUND, oss.str(), "RecoveryManager", "analyzeJournals");
                            }
                        }
                    }
                }
            }
        }

        // Set up recordIdList_ from enqueue map
        enqueueMapRef_.rid_list(recordIdList_);

        recordIdListConstItr_ = recordIdList_.begin();
    }
}

std::streamoff RecoveryManager::getEndOffset() const {
    return endOffset_;
}

uint64_t RecoveryManager::getHighestFileNumber() const {
    return highestFileNumber_;
}

uint64_t RecoveryManager::getHighestRecordId() const {
    return highestRecordId_;
}

bool RecoveryManager::isLastFileFull() const {
    return lastFileFullFlag_;
}

bool RecoveryManager::readNextRemainingRecord(void** const dataPtrPtr,
                                              std::size_t& dataSize,
                                              void** const xidPtrPtr,
                                              std::size_t& xidSize,
                                              bool& transient,
                                              bool& external,
                                              data_tok* const dtokp,
                                              bool /*ignore_pending_txns*/) {
    if (recordIdListConstItr_ == recordIdList_.end()) {
        return false;
    }
    enq_map::emap_data_struct_t eds;
    enqueueMapRef_.get_data(*recordIdListConstItr_, eds);
    uint64_t fileNumber = eds._pfid;
    currentJournalFileConstItr_ = fileNumberMap_.find(fileNumber);
    getNextFile(false);

    inFileStream_.seekg(eds._file_posn, std::ifstream::beg);
    if (!inFileStream_.good()) {
        std::ostringstream oss;
        oss << "Could not find offset 0x" << std::hex << eds._file_posn << " in file " << getCurrentFileName();
        throw jexception(jerrno::JERR__FILEIO, oss.str(), "RecoveryManager", "readNextRemainingRecord");
    }
    ::enq_hdr_t enqueueHeader;
    inFileStream_.read((char*)&enqueueHeader, sizeof(::enq_hdr_t));
    if (inFileStream_.gcount() != sizeof(::enq_hdr_t)) {
        std::ostringstream oss;
        oss << "Could not read enqueue header from file " << getCurrentFileName() << " at offset 0x" << std::hex << eds._file_posn;
        throw jexception(jerrno::JERR__FILEIO, oss.str(), "RecoveryManager", "readNextRemainingRecord");
    }
    // check flags
    transient = ::is_enq_transient(&enqueueHeader);
    external = ::is_enq_external(&enqueueHeader);

    // read xid
    xidSize = enqueueHeader._xidsize;
    *xidPtrPtr = ::malloc(xidSize);
    if (*xidPtrPtr == 0) {
        std::ostringstream oss;
        oss << "xidPtr, size=0x" << std::hex << xidSize;
        throw jexception(jerrno::JERR__MALLOC, oss.str(), "RecoveryManager", "readNextRemainingRecord");
    }
    readJournalData((char*)*xidPtrPtr, xidSize);

    // read data
    dataSize = enqueueHeader._dsize;
    *dataPtrPtr = ::malloc(dataSize);
    if (*xidPtrPtr == 0) {
        std::ostringstream oss;
        oss << "dataPtr, size=0x" << std::hex << dataSize;
        throw jexception(jerrno::JERR__MALLOC, oss.str(), "RecoveryManager", "readNextRemainingRecord");
    }
    readJournalData((char*)*dataPtrPtr, dataSize);

    // Set data token
    dtokp->set_wstate(data_tok::ENQ);
    dtokp->set_rid(enqueueHeader._rhdr._rid);
    dtokp->set_dsize(dataSize);
    if (xidSize) {
        dtokp->set_xid(*xidPtrPtr, xidSize);
    }

    ++recordIdListConstItr_;
    return true;
}

void RecoveryManager::setLinearFileControllerJournals(lfcAddJournalFileFn fnPtr,
                                                      LinearFileController* lfcPtr) {
    for (fileNumberMapConstItr_t i = fileNumberMap_.begin(); i != fileNumberMap_.end(); ++i) {
        uint32_t fileDblkCount = i->first == highestFileNumber_ ?               // Is this this last file?
                                 endOffset_ / QLS_DBLK_SIZE_BYTES :             // Last file uses _endOffset
                                 efpFileSize_kib_ * 1024 / QLS_DBLK_SIZE_BYTES; // All others use file size to make them full
        (lfcPtr->*fnPtr)(i->second, fileDblkCount);
    }
}

std::string RecoveryManager::toString(const std::string& jid,
                                      bool compact) {
    std::ostringstream oss;
    if (compact) {
        oss << "Recovery journal analysis (jid=\"" << jid << "\"):";
        oss << " jfl=[";
        for (fileNumberMapConstItr_t i=fileNumberMap_.begin(); i!=fileNumberMap_.end(); ++i) {
            if (i!=fileNumberMap_.begin()) {
                oss << " ";
            }
            std::string fqFileName = i->second->getFqFileName();
            oss << i->first << ":" << fqFileName.substr(fqFileName.rfind('/')+1);
        }
        oss << "] ecl=[ ";
        for (fileNumberMapConstItr_t j=fileNumberMap_.begin(); j!=fileNumberMap_.end(); ++j) {
            if (j!=fileNumberMap_.begin()) {
                oss << " ";
            }
            oss << j->second->getEnqueuedRecordCount();
        }
        oss << " ] empty=" << (journalEmptyFlag_ ? "T" : "F");
        oss << " fro=0x" << std::hex << firstRecordOffset_ << std::dec << " (" << (firstRecordOffset_/QLS_DBLK_SIZE_BYTES) << " dblks)";
        oss << " eo=0x" << std::hex << endOffset_ << std::dec << " ("  << (endOffset_/QLS_DBLK_SIZE_BYTES) << " dblks)";
        oss << " hrid=0x" << std::hex << highestRecordId_ << std::dec;
        oss << " hfnum=0x" << std::hex << highestFileNumber_ << std::dec;
        oss << " lffull=" << (lastFileFullFlag_ ? "T" : "F");
    } else {
        oss << "Recovery journal analysis (jid=\"" << jid << "\"):" << std::endl;
        oss << "  Number of journal files = " << fileNumberMap_.size() << std::endl;
        oss << "  Journal File List:" << std::endl;
        for (fileNumberMapConstItr_t k=fileNumberMap_.begin(); k!=fileNumberMap_.end(); ++k) {
            std::string fqFileName = k->second->getFqFileName();
            oss << "    " << k->first << ": " << fqFileName.substr(fqFileName.rfind('/')+1) << std::endl;
        }
        oss << "  Enqueue Counts: [ " << std::endl;
        for (fileNumberMapConstItr_t l=fileNumberMap_.begin(); l!=fileNumberMap_.end(); ++l) {
            if (l != fileNumberMap_.begin()) {
                oss << ", ";
            }
            oss << l->second->getEnqueuedRecordCount();
        }
        oss << " ]" << std::endl;
        oss << "  Journal empty = " << (journalEmptyFlag_ ? "TRUE" : "FALSE") << std::endl;
        oss << "  First record offset in first file = 0x" << std::hex << firstRecordOffset_ <<
                std::dec << " (" << (firstRecordOffset_/QLS_DBLK_SIZE_BYTES) << " dblks)" << std::endl;
        oss << "  End offset = 0x" << std::hex << endOffset_ << std::dec << " ("  <<
                (endOffset_/QLS_DBLK_SIZE_BYTES) << " dblks)" << std::endl;
        oss << "  Highest rid = 0x" << std::hex << highestRecordId_ << std::dec << std::endl;
        oss << "  Highest file number = 0x" << std::hex << highestFileNumber_ << std::dec << std::endl;
        oss << "  Last file full = " << (lastFileFullFlag_ ? "TRUE" : "FALSE") << std::endl;
        oss << "  Enqueued records (txn & non-txn):" << std::endl;
    }
    return oss.str();
}

// --- protected functions ---

void RecoveryManager::analyzeJournalFileHeaders(efpIdentity_t& efpIdentity) {
    std::string headerQueueName;
    ::file_hdr_t fileHeader;
    directoryList_t directoryList;
    jdir::read_dir(journalDirectory_, directoryList, false, true, false, true);
    for (directoryListConstItr_t i = directoryList.begin(); i != directoryList.end(); ++i) {
        readJournalFileHeader(*i, fileHeader, headerQueueName);
        if (headerQueueName.compare(queueName_) != 0) {
            std::ostringstream oss;
            oss << "Journal file " << (*i) << " belongs to queue \"" << headerQueueName << "\": ignoring";
            journalLogRef_.log(JournalLog::LOG_WARN, queueName_, oss.str());
        } else {
            JournalFile* jfp = new JournalFile(*i, fileHeader);
            fileNumberMap_[fileHeader._file_number] = jfp;
            if (fileHeader._file_number > highestFileNumber_) {
                highestFileNumber_ = fileHeader._file_number;
            }
        }
    }
    efpIdentity.pn_ = fileHeader._efp_partition;
    efpIdentity.ds_ = fileHeader._data_size_kib;
    currentJournalFileConstItr_ = fileNumberMap_.begin();
}

void RecoveryManager::checkFileStreamOk(bool checkEof) {
    if (inFileStream_.fail() || inFileStream_.bad() || checkEof ? inFileStream_.eof() : false) {
        std::ostringstream oss;
        oss << "Stream status: fail=" << (inFileStream_.fail()?"T":"F") << " bad=" << (inFileStream_.bad()?"T":"F");
        if (checkEof) {
            oss << " eof=" << (inFileStream_.eof()?"T":"F");
        }
        throw jexception(jerrno::JERR_RCVM_STREAMBAD, oss.str(), "RecoveryManager", "checkFileStreamOk");
    }
}

void RecoveryManager::checkJournalAlignment(const std::streampos recordPosition) {
    std::streampos currentPosn = recordPosition;
    unsigned sblkOffset = currentPosn % QLS_SBLK_SIZE_BYTES;
    if (sblkOffset)
    {
        std::ostringstream oss1;
        oss1 << std::hex << "Bad record alignment found at fid=0x" << getCurrentFileNumber();
        oss1 << " offs=0x" << currentPosn << " (likely journal overwrite boundary); " << std::dec;
        oss1 << (QLS_SBLK_SIZE_DBLKS - (sblkOffset/QLS_DBLK_SIZE_BYTES)) << " filler record(s) required.";
        journalLogRef_.log(JournalLog::LOG_WARN, queueName_, oss1.str());

        std::ofstream outFileStream(getCurrentFileName().c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::binary);
        if (!outFileStream.good()) {
            throw jexception(jerrno::JERR__FILEIO, getCurrentFileName(), "RecoveryManager", "checkJournalAlignment");
        }
        outFileStream.seekp(currentPosn);

        // Prepare write buffer containing a single empty record (1 dblk)
        void* writeBuffer = std::malloc(QLS_DBLK_SIZE_BYTES);
        if (writeBuffer == 0) {
            throw jexception(jerrno::JERR__MALLOC, "RecoveryManager", "checkJournalAlignment");
        }
        const uint32_t xmagic = QLS_EMPTY_MAGIC;
        ::memcpy(writeBuffer, (const void*)&xmagic, sizeof(xmagic));
        ::memset((char*)writeBuffer + sizeof(xmagic), QLS_CLEAN_CHAR, QLS_DBLK_SIZE_BYTES - sizeof(xmagic));

        // Write as many empty records as are needed to get to sblk boundary
        while (currentPosn % QLS_SBLK_SIZE_BYTES) {
            outFileStream.write((const char*)writeBuffer, QLS_DBLK_SIZE_BYTES);
            if (outFileStream.fail()) {
                throw jexception(jerrno::JERR_RCVM_WRITE, "RecoveryManager", "checkJournalAlignment");
            }
            std::ostringstream oss2;
            oss2 << std::hex << "Recover phase write: Wrote filler record: fid=0x" << getCurrentFileNumber();
            oss2 << " offs=0x" << currentPosn;
            journalLogRef_.log(JournalLog::LOG_NOTICE, queueName_, oss2.str());
            currentPosn = outFileStream.tellp();
        }
        outFileStream.close();
        std::free(writeBuffer);
        journalLogRef_.log(JournalLog::LOG_INFO, queueName_, "Bad record alignment fixed.");
    }
    endOffset_ = currentPosn;
}

bool RecoveryManager::decodeRecord(jrec& record,
                              std::size_t& cumulativeSizeRead,
                              ::rec_hdr_t& headerRecord,
                              std::streampos& fileOffset)
{
    std::streampos start_file_offs = fileOffset;

    if (highestRecordId_ == 0) {
        highestRecordId_ = headerRecord._rid;
    } else if (headerRecord._rid - highestRecordId_ < 0x8000000000000000ULL) { // RFC 1982 comparison for unsigned 64-bit
        highestRecordId_ = headerRecord._rid;
    }

    bool done = false;
    while (!done) {
        try {
            done = record.rcv_decode(headerRecord, &inFileStream_, cumulativeSizeRead);
        }
        catch (const jexception& e) {
            checkJournalAlignment(start_file_offs);
            return false;
        }
        if (!done && !getNextFile(false)) {
            checkJournalAlignment(start_file_offs);
            return false;
        }
    }
    return true;
}

std::string RecoveryManager::getCurrentFileName() const {
    return currentJournalFileConstItr_->second->getFqFileName();
}

uint64_t RecoveryManager::getCurrentFileNumber() const {
    return currentJournalFileConstItr_->first;
}

bool RecoveryManager::getNextFile(bool jumpToFirstRecordOffsetFlag) {
    if (inFileStream_.is_open()) {
        if (inFileStream_.eof() || !inFileStream_.good()) {
            inFileStream_.clear();
            endOffset_ = inFileStream_.tellg(); // remember file offset before closing
            if (endOffset_ == -1) {
                std::ostringstream oss;
                oss << "tellg() failure: fail=" << (inFileStream_.fail()?"T":"F") << " bad=" << (inFileStream_.bad()?"T":"F");
                throw jexception(jerrno::JERR_RCVM_STREAMBAD, oss.str(), "RecoveryManager", "getNextFile");
            }
            inFileStream_.close();
            if (++currentJournalFileConstItr_ == fileNumberMap_.end()) {
                return false;
            }
        }
    }
    if (!inFileStream_.is_open())  {
        inFileStream_.clear(); // clear eof flag, req'd for older versions of c++
        inFileStream_.open(getCurrentFileName().c_str(), std::ios_base::in | std::ios_base::binary);
        if (!inFileStream_.good()) {
            throw jexception(jerrno::JERR__FILEIO, getCurrentFileName(), "RecoveryManager", "getNextFile");
        }

        // Read file header
        file_hdr_t fhdr;
        inFileStream_.read((char*)&fhdr, sizeof(fhdr));
        checkFileStreamOk(true);
        if (fhdr._rhdr._magic == QLS_FILE_MAGIC) {
            firstRecordOffset_ = fhdr._fro;
            std::streamoff foffs = jumpToFirstRecordOffsetFlag ? firstRecordOffset_ : QLS_SBLK_SIZE_BYTES;
            inFileStream_.seekg(foffs);
        } else {
            inFileStream_.close();
            if (currentJournalFileConstItr_ == fileNumberMap_.begin()) {
                journalEmptyFlag_ = true;
            }
            return false;
        }
    }
    return true;
}

bool RecoveryManager::getNextRecordHeader()
{
    std::size_t cum_size_read = 0;
    void* xidp = 0;
    rec_hdr_t h;

    bool hdr_ok = false;
    std::streampos file_pos;
    while (!hdr_ok) {
        if (!inFileStream_.is_open()) {
            if (!getNextFile(true)) {
                return false;
            }
        }
        file_pos = inFileStream_.tellg();
        if (file_pos == std::streampos(-1)) {
            std::ostringstream oss;
            oss << "tellg() failure: fail=" << (inFileStream_.fail()?"T":"F") << " bad=" << (inFileStream_.bad()?"T":"F");
            throw jexception(jerrno::JERR_RCVM_STREAMBAD, oss.str(), "RecoveryManager", "getNextRecordHeader");
        }
        inFileStream_.read((char*)&h, sizeof(rec_hdr_t));
        if (inFileStream_.gcount() == sizeof(rec_hdr_t)) {
            hdr_ok = true;
        } else {
            if (!getNextFile(true)) {
                return false;
            }
        }
    }

    switch(h._magic) {
        case QLS_ENQ_MAGIC:
            {
//std::cout << " 0x" << std::hex << file_pos << ".e.0x" << h._rid << std::dec << std::flush; // DEBUG
                enq_rec er;
                uint64_t start_fid = getCurrentFileNumber(); // fid may increment in decode() if record folds over file boundary
                if (!decodeRecord(er, cum_size_read, h, file_pos)) {
                    return false;
                }
                if (!er.is_transient()) { // Ignore transient msgs
                    fileNumberMap_[start_fid]->incrEnqueuedRecordCount();
                    if (er.xid_size()) {
                        er.get_xid(&xidp);
                        if (xidp == 0) {
                            throw jexception(jerrno::JERR_RCVM_NULLXID, "ENQ", "RecoveryManager", "getNextRecordHeader");
                        }
                        std::string xid((char*)xidp, er.xid_size());
                        transactionMapRef_.insert_txn_data(xid, txn_data_t(h._rid, 0, start_fid, file_pos, true));
                        if (transactionMapRef_.set_aio_compl(xid, h._rid) < txn_map::TMAP_OK) { // fail - xid or rid not found
                            std::ostringstream oss;
                            oss << std::hex << "_tmap.set_aio_compl: txn_enq xid=\"" << xid << "\" rid=0x" << h._rid;
                            throw jexception(jerrno::JERR_MAP_NOTFOUND, oss.str(), "RecoveryManager", "getNextRecordHeader");
                        }
                        std::free(xidp);
                    } else {
                        if (enqueueMapRef_.insert_pfid(h._rid, start_fid, file_pos) < enq_map::EMAP_OK) { // fail
                            // The only error code emap::insert_pfid() returns is enq_map::EMAP_DUP_RID.
                            std::ostringstream oss;
                            oss << std::hex << "rid=0x" << h._rid << " _pfid=0x" << start_fid;
                            throw jexception(jerrno::JERR_MAP_DUPLICATE, oss.str(), "RecoveryManager", "getNextRecordHeader");
                        }
                    }
                }
            }
            break;
        case QLS_DEQ_MAGIC:
            {
//std::cout << " 0x" << std::hex << file_pos << ".d.0x" << h._rid << std::dec << std::flush; // DEBUG
                deq_rec dr;
                uint16_t start_fid = getCurrentFileNumber(); // fid may increment in decode() if record folds over file boundary
                if (!decodeRecord(dr, cum_size_read, h, file_pos)) {
                    return false;
                }
                if (dr.xid_size()) {
                    // If the enqueue is part of a pending txn, it will not yet be in emap
                    enqueueMapRef_.lock(dr.deq_rid()); // ignore not found error
                    dr.get_xid(&xidp);
                    if (xidp == 0) {
                        throw jexception(jerrno::JERR_RCVM_NULLXID, "DEQ", "RecoveryManager", "getNextRecordHeader");
                    }
                    std::string xid((char*)xidp, dr.xid_size());
                    transactionMapRef_.insert_txn_data(xid, txn_data_t(dr.rid(), dr.deq_rid(), start_fid, file_pos,
                                                       false, dr.is_txn_coml_commit()));
                    if (transactionMapRef_.set_aio_compl(xid, dr.rid()) < txn_map::TMAP_OK) { // fail - xid or rid not found
                        std::ostringstream oss;
                        oss << std::hex << "_tmap.set_aio_compl: txn_deq xid=\"" << xid << "\" rid=0x" << dr.rid();
                        throw jexception(jerrno::JERR_MAP_NOTFOUND, oss.str(), "RecoveryManager", "getNextRecordHeader");
                    }
                    std::free(xidp);
                } else {
                    uint64_t enq_fid;
                    if (enqueueMapRef_.get_remove_pfid(dr.deq_rid(), enq_fid, true) == enq_map::EMAP_OK) { // ignore not found error
                        fileNumberMap_[enq_fid]->decrEnqueuedRecordCount();
                    }
                }
            }
            break;
        case QLS_TXA_MAGIC:
            {
//std::cout << " 0x" << std::hex << file_pos << ".a.0x" << h._rid << std::dec << std::flush; // DEBUG
                txn_rec ar;
                if (!decodeRecord(ar, cum_size_read, h, file_pos)) {
                    return false;
                }
                // Delete this txn from tmap, unlock any locked records in emap
                ar.get_xid(&xidp);
                if (xidp == 0) {
                    throw jexception(jerrno::JERR_RCVM_NULLXID, "ABT", "RecoveryManager", "getNextRecordHeader");
                }
                std::string xid((char*)xidp, ar.xid_size());
                txn_data_list tdl = transactionMapRef_.get_remove_tdata_list(xid); // tdl will be empty if xid not found
                for (tdl_itr itr = tdl.begin(); itr != tdl.end(); itr++) {
                    if (itr->enq_flag_) {
                        fileNumberMap_[itr->pfid_]->decrEnqueuedRecordCount();
                    } else {
                        enqueueMapRef_.unlock(itr->drid_); // ignore not found error
                    }
                }
                std::free(xidp);
            }
            break;
        case QLS_TXC_MAGIC:
            {
//std::cout << " 0x" << std::hex << file_pos << ".c.0x" << h._rid << std::dec << std::flush; // DEBUG
                txn_rec cr;
                if (!decodeRecord(cr, cum_size_read, h, file_pos)) {
                    return false;
                }
                // Delete this txn from tmap, process records into emap
                cr.get_xid(&xidp);
                if (xidp == 0) {
                    throw jexception(jerrno::JERR_RCVM_NULLXID, "CMT", "RecoveryManager", "getNextRecordHeader");
                }
                std::string xid((char*)xidp, cr.xid_size());
                txn_data_list tdl = transactionMapRef_.get_remove_tdata_list(xid); // tdl will be empty if xid not found
                for (tdl_itr itr = tdl.begin(); itr != tdl.end(); itr++) {
                    if (itr->enq_flag_) { // txn enqueue
//std::cout << "[rid=0x" << std::hex << itr->rid_ << std::dec << " fid=" << itr->pfid_ << " fpos=0x" << std::hex << itr->foffs_ << "]" << std::dec << std::flush; // DEBUG
                        if (enqueueMapRef_.insert_pfid(itr->rid_, itr->pfid_, itr->foffs_) < enq_map::EMAP_OK) { // fail
                            // The only error code emap::insert_pfid() returns is enq_map::EMAP_DUP_RID.
                            std::ostringstream oss;
                            oss << std::hex << "rid=0x" << itr->rid_ << " _pfid=0x" << itr->pfid_;
                            throw jexception(jerrno::JERR_MAP_DUPLICATE, oss.str(), "RecoveryManager", "getNextRecordHeader");
                        }
                    } else { // txn dequeue
                        uint64_t enq_fid;
                        if (enqueueMapRef_.get_remove_pfid(itr->drid_, enq_fid, true) == enq_map::EMAP_OK) // ignore not found error
                            fileNumberMap_[enq_fid]->decrEnqueuedRecordCount();
                    }
                }
                std::free(xidp);
            }
            break;
        case QLS_EMPTY_MAGIC:
            {
//std::cout << ".x" << std::flush; // DEBUG
                uint32_t rec_dblks = jrec::size_dblks(sizeof(::rec_hdr_t));
                inFileStream_.ignore(rec_dblks * QLS_DBLK_SIZE_BYTES - sizeof(::rec_hdr_t));
                checkFileStreamOk(false);
                if (!getNextFile(false)) {
                    return false;
                }
            }
            break;
        case 0:
//std::cout << " 0x" << std::hex << file_pos << ".0" << std::dec << std::endl << std::flush; // DEBUG
            checkJournalAlignment(file_pos);
            return false;
        default:
//std::cout << " 0x" << std::hex << file_pos << ".?" << std::dec << std::endl << std::flush; // DEBUG
            // Stop as this is the overwrite boundary.
            checkJournalAlignment(file_pos);
            return false;
    }
    return true;
}

void RecoveryManager::readJournalData(char* target,
                                      const std::streamsize readSize) {
    std::streamoff bytesRead = 0;
    while (bytesRead < readSize) {
        std::streampos file_pos = inFileStream_.tellg();
        if (file_pos == std::streampos(-1)) {
            std::ostringstream oss;
            oss << "tellg() failure: fail=" << (inFileStream_.fail()?"T":"F") << " bad=" << (inFileStream_.bad()?"T":"F");
            throw jexception(jerrno::JERR_RCVM_STREAMBAD, oss.str(), "RecoveryManager", "readJournalData");
        }
        inFileStream_.read(target + bytesRead, readSize - bytesRead);
        std::streamoff thisReadSize = inFileStream_.gcount();
        if (thisReadSize < readSize) {
            getNextFile(false);
            file_pos = inFileStream_.tellg();
            if (file_pos == std::streampos(-1)) {
                std::ostringstream oss;
                oss << "tellg() failure: fail=" << (inFileStream_.fail()?"T":"F") << " bad=" << (inFileStream_.bad()?"T":"F");
                throw jexception(jerrno::JERR_RCVM_STREAMBAD, oss.str(), "RecoveryManager", "readJournalData");
            }
        }
        bytesRead += thisReadSize;
    }
}

// static private
void RecoveryManager::readJournalFileHeader(const std::string& journalFileName,
                                            ::file_hdr_t& fileHeaderRef,
                                            std::string& queueName) {
    const std::size_t headerBlockSize = QLS_JRNL_FHDR_RES_SIZE_SBLKS * QLS_SBLK_SIZE_KIB * 1024;
    char buffer[headerBlockSize];
    std::ifstream ifs(journalFileName.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!ifs.good()) {
        std::ostringstream oss;
        oss << "File=" << journalFileName;
        throw jexception(jerrno::JERR_RCVM_OPENRD, oss.str(), "RecoveryManager", "readJournalFileHeader");
    }
    ifs.read(buffer, headerBlockSize);
    if (!ifs) {
        std::streamsize s = ifs.gcount();
        ifs.close();
        std::ostringstream oss;
        oss << "File=" << journalFileName << "; attempted_read_size=" << headerBlockSize << "; actual_read_size=" << s;
        throw jexception(jerrno::JERR_RCVM_READ, oss.str(), "RecoveryManager", "readJournalFileHeader");
    }
    ifs.close();
    ::memcpy(&fileHeaderRef, buffer, sizeof(::file_hdr_t));
    queueName.assign(buffer + sizeof(::file_hdr_t), fileHeaderRef._queue_name_len);

}

void RecoveryManager::removeEmptyFiles(EmptyFilePool* emptyFilePoolPtr) {
    while (fileNumberMap_.begin()->second->getEnqueuedRecordCount() == 0 && fileNumberMap_.size() > 1) {
//std::cout << "*** File " << i->first << ": " << i->second << " is empty." << std::endl; // DEBUG
        emptyFilePoolPtr->returnEmptyFile(fileNumberMap_.begin()->second->getFqFileName());
        fileNumberMap_.erase(fileNumberMap_.begin()->first);
    }
}

}} // namespace qpid::qls_jrnl
