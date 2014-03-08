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

#include "qpid/linearstore/jrnl/enq_rec.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include "qpid/linearstore/jrnl/jerrno.h"
#include "qpid/linearstore/jrnl/jexception.h"
#include <sstream>

namespace qpid
{
namespace qls_jrnl
{

// Constructor used for read operations, where buf contains preallocated space to receive data.
enq_rec::enq_rec():
        jrec(), // superclass
        //_enq_hdr(QLS_ENQ_MAGIC, QLS_JRNL_VERSION, 0, 0, 0, false, false),
        _xidp(0),
        _data(0),
        _buff(0)
        //_enq_tail(_enq_hdr)
{
    ::enq_hdr_init(&_enq_hdr, QLS_ENQ_MAGIC, QLS_JRNL_VERSION, 0, 0, 0, false);
    ::rec_tail_copy(&_enq_tail, &_enq_hdr._rhdr, 0);
}

// Constructor used for transactional write operations, where dbuf contains data to be written.
enq_rec::enq_rec(const uint64_t rid, const void* const dbuf, const std::size_t dlen,
        const void* const xidp, const std::size_t xidlen, const bool transient):
        jrec(), // superclass
        //_enq_hdr(QLS_ENQ_MAGIC, QLS_JRNL_VERSION, rid, xidlen, dlen, owi, transient),
        _xidp(xidp),
        _data(dbuf),
        _buff(0)
        //_enq_tail(_enq_hdr)
{
    ::enq_hdr_init(&_enq_hdr, QLS_ENQ_MAGIC, QLS_JRNL_VERSION, 0, rid, xidlen, dlen);
    ::rec_tail_copy(&_enq_tail, &_enq_hdr._rhdr, 0);
    ::set_enq_transient(&_enq_hdr, transient);
}

enq_rec::~enq_rec()
{
    clean();
}

// Prepare instance for use in reading data from journal, where buf contains preallocated space
// to receive data.
void
enq_rec::reset()
{
    _enq_hdr._rhdr._rid = 0;
    ::set_enq_transient(&_enq_hdr, false);
    _enq_hdr._xidsize = 0;
    _enq_hdr._dsize = 0;
    _xidp = 0;
    _data = 0;
    _buff = 0;
    _enq_tail._rid = 0;
}

// Prepare instance for use in writing transactional data to journal, where dbuf contains data to
// be written.
void
enq_rec::reset(const uint64_t rid, const void* const dbuf, const std::size_t dlen,
        const void* const xidp, const std::size_t xidlen, const bool transient,
        const bool external)
{
    _enq_hdr._rhdr._rid = rid;
    ::set_enq_transient(&_enq_hdr, transient);
    ::set_enq_external(&_enq_hdr, external);
    _enq_hdr._xidsize = xidlen;
    _enq_hdr._dsize = dlen;
    _xidp = xidp;
    _data = dbuf;
    _buff = 0;
    _enq_tail._rid = rid;
}

uint32_t
enq_rec::encode(void* wptr, uint32_t rec_offs_dblks, uint32_t max_size_dblks)
{
    assert(wptr != 0);
    assert(max_size_dblks > 0);
    if (_xidp == 0)
        assert(_enq_hdr._xidsize == 0);

    std::size_t rec_offs = rec_offs_dblks * QLS_DBLK_SIZE_BYTES;
    std::size_t rem = max_size_dblks * QLS_DBLK_SIZE_BYTES;
    std::size_t wr_cnt = 0;
    if (rec_offs_dblks) // Continuation of split data record (over 2 or more pages)
    {
        if (size_dblks(rec_size()) - rec_offs_dblks > max_size_dblks) // Further split required
        {
            rec_offs -= sizeof(_enq_hdr);
            std::size_t wsize = _enq_hdr._xidsize > rec_offs ? _enq_hdr._xidsize - rec_offs : 0;
            std::size_t wsize2 = wsize;
            if (wsize)
            {
                if (wsize > rem)
                    wsize = rem;
                std::memcpy(wptr, (const char*)_xidp + rec_offs, wsize);
                wr_cnt = wsize;
                rem -= wsize;
            }
            rec_offs -= _enq_hdr._xidsize - wsize2;
            if (rem && !::is_enq_external(&_enq_hdr))
            {
                wsize = _enq_hdr._dsize > rec_offs ? _enq_hdr._dsize - rec_offs : 0;
                wsize2 = wsize;
                if (wsize)
                {
                    if (wsize > rem)
                        wsize = rem;
                    std::memcpy((char*)wptr + wr_cnt, (const char*)_data + rec_offs, wsize);
                    wr_cnt += wsize;
                    rem -= wsize;
                }
                rec_offs -= _enq_hdr._dsize - wsize2;
            }
            if (rem)
            {
                wsize = sizeof(_enq_tail) > rec_offs ? sizeof(_enq_tail) - rec_offs : 0;
                wsize2 = wsize;
                if (wsize)
                {
                    if (wsize > rem)
                        wsize = rem;
                    std::memcpy((char*)wptr + wr_cnt, (char*)&_enq_tail + rec_offs, wsize);
                    wr_cnt += wsize;
                    rem -= wsize;
                }
                rec_offs -= sizeof(_enq_tail) - wsize2;
            }
            assert(rem == 0);
            assert(rec_offs == 0);
        }
        else // No further split required
        {
            rec_offs -= sizeof(_enq_hdr);
            std::size_t wsize = _enq_hdr._xidsize > rec_offs ? _enq_hdr._xidsize - rec_offs : 0;
            if (wsize)
            {
                std::memcpy(wptr, (const char*)_xidp + rec_offs, wsize);
                wr_cnt += wsize;
            }
            rec_offs -= _enq_hdr._xidsize - wsize;
            wsize = _enq_hdr._dsize > rec_offs ? _enq_hdr._dsize - rec_offs : 0;
            if (wsize && !::is_enq_external(&_enq_hdr))
            {
                std::memcpy((char*)wptr + wr_cnt, (const char*)_data + rec_offs, wsize);
                wr_cnt += wsize;
            }
            rec_offs -= _enq_hdr._dsize - wsize;
            wsize = sizeof(_enq_tail) > rec_offs ? sizeof(_enq_tail) - rec_offs : 0;
            if (wsize)
            {
                std::memcpy((char*)wptr + wr_cnt, (char*)&_enq_tail + rec_offs, wsize);
                wr_cnt += wsize;
#ifdef QLS_CLEAN
                std::size_t rec_offs = rec_offs_dblks * QLS_DBLK_SIZE_BYTES;
                std::size_t dblk_rec_size = size_dblks(rec_size() - rec_offs) * QLS_DBLK_SIZE_BYTES;
                std::memset((char*)wptr + wr_cnt, QLS_CLEAN_CHAR, dblk_rec_size - wr_cnt);
#endif
            }
            rec_offs -= sizeof(_enq_tail) - wsize;
            assert(rec_offs == 0);
        }
    }
    else // Start at beginning of data record
    {
        // Assumption: the header will always fit into the first dblk
        std::memcpy(wptr, (void*)&_enq_hdr, sizeof(_enq_hdr));
        wr_cnt = sizeof(_enq_hdr);
        if (size_dblks(rec_size()) > max_size_dblks) // Split required
        {
            std::size_t wsize;
            rem -= sizeof(_enq_hdr);
            if (rem)
            {
                wsize = rem >= _enq_hdr._xidsize ? _enq_hdr._xidsize : rem;
                std::memcpy((char*)wptr + wr_cnt,  _xidp, wsize);
                wr_cnt += wsize;
                rem -= wsize;
            }
            if (rem && !::is_enq_external(&_enq_hdr))
            {
                wsize = rem >= _enq_hdr._dsize ? _enq_hdr._dsize : rem;
                std::memcpy((char*)wptr + wr_cnt, _data, wsize);
                wr_cnt += wsize;
                rem -= wsize;
            }
            if (rem)
            {
                wsize = rem >= sizeof(_enq_tail) ? sizeof(_enq_tail) : rem;
                std::memcpy((char*)wptr + wr_cnt, (void*)&_enq_tail, wsize);
                wr_cnt += wsize;
                rem -= wsize;
            }
            assert(rem == 0);
        }
        else // No split required
        {
            if (_enq_hdr._xidsize)
            {
                std::memcpy((char*)wptr + wr_cnt, _xidp, _enq_hdr._xidsize);
                wr_cnt += _enq_hdr._xidsize;
            }
            if (!::is_enq_external(&_enq_hdr))
            {
                std::memcpy((char*)wptr + wr_cnt, _data, _enq_hdr._dsize);
                wr_cnt += _enq_hdr._dsize;
            }
            std::memcpy((char*)wptr + wr_cnt, (void*)&_enq_tail, sizeof(_enq_tail));
            wr_cnt += sizeof(_enq_tail);
#ifdef QLS_CLEAN
            std::size_t dblk_rec_size = size_dblks(rec_size()) * QLS_DBLK_SIZE_BYTES;
            std::memset((char*)wptr + wr_cnt, QLS_CLEAN_CHAR, dblk_rec_size - wr_cnt);
#endif
        }
    }
    return size_dblks(wr_cnt);
}

uint32_t
enq_rec::decode(rec_hdr_t& h, void* rptr, uint32_t rec_offs_dblks, uint32_t max_size_dblks)
{
    assert(rptr != 0);
    assert(max_size_dblks > 0);

    std::size_t rd_cnt = 0;
    if (rec_offs_dblks) // Continuation of record on new page
    {
        const uint32_t hdr_xid_data_size = sizeof(enq_hdr_t) + _enq_hdr._xidsize +
                (::is_enq_external(&_enq_hdr) ? 0 : _enq_hdr._dsize);
        const uint32_t hdr_xid_data_tail_size = hdr_xid_data_size + sizeof(rec_tail_t);
        const uint32_t hdr_data_dblks = size_dblks(hdr_xid_data_size);
        const uint32_t hdr_tail_dblks = size_dblks(hdr_xid_data_tail_size);
        const std::size_t rec_offs = rec_offs_dblks * QLS_DBLK_SIZE_BYTES;
        const std::size_t offs = rec_offs - sizeof(enq_hdr_t);

        if (hdr_tail_dblks - rec_offs_dblks <= max_size_dblks)
        {
            // Remainder of record fits within this page
            if (offs < _enq_hdr._xidsize)
            {
                // some XID still outstanding, copy remainder of XID, data and tail
                const std::size_t rem = _enq_hdr._xidsize + _enq_hdr._dsize - offs;
                std::memcpy((char*)_buff + offs, rptr, rem);
                rd_cnt += rem;
                std::memcpy((void*)&_enq_tail, ((char*)rptr + rd_cnt), sizeof(_enq_tail));
                chk_tail();
                rd_cnt += sizeof(_enq_tail);
            }
            else if (offs < _enq_hdr._xidsize + _enq_hdr._dsize && !::is_enq_external(&_enq_hdr))
            {
                // some data still outstanding, copy remainder of data and tail
                const std::size_t data_offs = offs - _enq_hdr._xidsize;
                const std::size_t data_rem = _enq_hdr._dsize - data_offs;
                std::memcpy((char*)_buff + offs, rptr, data_rem);
                rd_cnt += data_rem;
                std::memcpy((void*)&_enq_tail, ((char*)rptr + rd_cnt), sizeof(_enq_tail));
                chk_tail();
                rd_cnt += sizeof(_enq_tail);
            }
            else
            {
                // Tail or part of tail only outstanding, complete tail
                const std::size_t tail_offs = rec_offs - sizeof(enq_hdr_t) - _enq_hdr._xidsize -
                        _enq_hdr._dsize;
                const std::size_t tail_rem = sizeof(rec_tail_t) - tail_offs;
                std::memcpy((char*)&_enq_tail + tail_offs, rptr, tail_rem);
                chk_tail();
                rd_cnt = tail_rem;
            }
        }
        else if (hdr_data_dblks - rec_offs_dblks <= max_size_dblks)
        {
            // Remainder of xid & data fits within this page; tail split

            /*
             * TODO: This section needs revision. Since it is known that the end of the page falls within the
             * tail record, it is only necessary to write from the current offset to the end of the page under
             * all circumstances. The multiple if/else combinations may be eliminated, as well as one memcpy()
             * operation.
             *
             * Also note that Coverity has detected a possible memory overwrite in this block. It occurs if
             * both the following two if() stmsts (numbered) are false. With rd_cnt = 0, this would result in
             * the value of tail_rem > sizeof(tail_rec). Practically, this could only happen if the start and
             * end of a page both fall within the same tail record, in which case the tail would have to be
             * (much!) larger. However, the logic here does not account for this possibility.
             *
             * If the optimization above is undertaken, this code would probably be removed.
             */
            if (offs < _enq_hdr._xidsize) // 1
            {
                // some XID still outstanding, copy remainder of XID and data
                const std::size_t rem = _enq_hdr._xidsize + _enq_hdr._dsize - offs;
                std::memcpy((char*)_buff + offs, rptr, rem);
                rd_cnt += rem;
            }
            else if (offs < _enq_hdr._xidsize + _enq_hdr._dsize && !::is_enq_external(&_enq_hdr)) // 2
            {
                // some data still outstanding, copy remainder of data
                const std::size_t data_offs = offs - _enq_hdr._xidsize;
                const std::size_t data_rem = _enq_hdr._dsize - data_offs;
                std::memcpy((char*)_buff + offs, rptr, data_rem);
                rd_cnt += data_rem;
            }
            const std::size_t tail_rem = (max_size_dblks * QLS_DBLK_SIZE_BYTES) - rd_cnt;
            if (tail_rem)
            {
                std::memcpy((void*)&_enq_tail, ((char*)rptr + rd_cnt), tail_rem);
                rd_cnt += tail_rem;
            }
        }
        else
        {
            // Since xid and data are contiguous, both fit within current page - copy whole page
            const std::size_t data_cp_size = (max_size_dblks * QLS_DBLK_SIZE_BYTES);
            std::memcpy((char*)_buff + offs, rptr, data_cp_size);
            rd_cnt += data_cp_size;
        }
    }
    else // Start of record
    {
        // Get and check header
        //_enq_hdr.hdr_copy(h);
        ::rec_hdr_copy(&_enq_hdr._rhdr, &h);
        rd_cnt = sizeof(rec_hdr_t);
        std::memcpy(&_enq_hdr._xidsize, (char*)rptr + rd_cnt, sizeof(std::size_t));
        rd_cnt += sizeof(std::size_t);
#if defined(JRNL_32_BIT)
        rd_cnt += sizeof(uint32_t); // Filler 0
#endif
        std::memcpy(&_enq_hdr._dsize, (char*)rptr + rd_cnt, sizeof(std::size_t));
        rd_cnt = sizeof(enq_hdr_t);
        chk_hdr();
        if (_enq_hdr._xidsize + (::is_enq_external(&_enq_hdr) ? 0 : _enq_hdr._dsize))
        {
            _buff = std::malloc(_enq_hdr._xidsize + (::is_enq_external(&_enq_hdr) ? 0 : _enq_hdr._dsize));
            MALLOC_CHK(_buff, "_buff", "enq_rec", "decode");

            const uint32_t hdr_xid_size = sizeof(enq_hdr_t) + _enq_hdr._xidsize;
            const uint32_t hdr_xid_data_size = hdr_xid_size + (::is_enq_external(&_enq_hdr) ? 0 : _enq_hdr._dsize);
            const uint32_t hdr_xid_data_tail_size = hdr_xid_data_size + sizeof(rec_tail_t);
            const uint32_t hdr_xid_dblks  = size_dblks(hdr_xid_size);
            const uint32_t hdr_data_dblks = size_dblks(hdr_xid_data_size);
            const uint32_t hdr_tail_dblks = size_dblks(hdr_xid_data_tail_size);
            // Check if record (header + data + tail) fits within this page, we can check the
            // tail before the expense of copying data to memory
            if (hdr_tail_dblks <= max_size_dblks)
            {
                // Header, xid, data and tail fits within this page
                if (_enq_hdr._xidsize)
                {
                    std::memcpy(_buff, (char*)rptr + rd_cnt, _enq_hdr._xidsize);
                    rd_cnt += _enq_hdr._xidsize;
                }
                if (_enq_hdr._dsize && !::is_enq_external(&_enq_hdr))
                {
                    std::memcpy((char*)_buff + _enq_hdr._xidsize, (char*)rptr + rd_cnt,
                            _enq_hdr._dsize);
                    rd_cnt += _enq_hdr._dsize;
                }
                std::memcpy((void*)&_enq_tail, (char*)rptr + rd_cnt, sizeof(_enq_tail));
                chk_tail();
                rd_cnt += sizeof(_enq_tail);
            }
            else if (hdr_data_dblks <= max_size_dblks)
            {
                // Header, xid and data fit within this page, tail split or separated
                if (_enq_hdr._xidsize)
                {
                    std::memcpy(_buff, (char*)rptr + rd_cnt, _enq_hdr._xidsize);
                    rd_cnt += _enq_hdr._xidsize;
                }
                if (_enq_hdr._dsize && !::is_enq_external(&_enq_hdr))
                {
                    std::memcpy((char*)_buff + _enq_hdr._xidsize, (char*)rptr + rd_cnt,
                            _enq_hdr._dsize);
                    rd_cnt += _enq_hdr._dsize;
                }
                const std::size_t tail_rem = (max_size_dblks * QLS_DBLK_SIZE_BYTES) - rd_cnt;
                if (tail_rem)
                {
                    std::memcpy((void*)&_enq_tail, (char*)rptr + rd_cnt, tail_rem);
                    rd_cnt += tail_rem;
                }
            }
            else if (hdr_xid_dblks <= max_size_dblks)
            {
                // Header and xid fits within this page, data split or separated
                if (_enq_hdr._xidsize)
                {
                    std::memcpy(_buff, (char*)rptr + rd_cnt, _enq_hdr._xidsize);
                    rd_cnt += _enq_hdr._xidsize;
                }
                if (_enq_hdr._dsize && !::is_enq_external(&_enq_hdr))
                {
                    const std::size_t data_cp_size = (max_size_dblks * QLS_DBLK_SIZE_BYTES) - rd_cnt;
                    std::memcpy((char*)_buff + _enq_hdr._xidsize, (char*)rptr + rd_cnt, data_cp_size);
                    rd_cnt += data_cp_size;
                }
            }
            else
            {
                // Header fits within this page, xid split or separated
                const std::size_t data_cp_size = (max_size_dblks * QLS_DBLK_SIZE_BYTES) - rd_cnt;
                std::memcpy(_buff, (char*)rptr + rd_cnt, data_cp_size);
                rd_cnt += data_cp_size;
            }
        }
    }
    return size_dblks(rd_cnt);
}

bool
enq_rec::rcv_decode(rec_hdr_t h, std::ifstream* ifsp, std::size_t& rec_offs)
{
    if (rec_offs == 0)
    {
        // Read header, allocate (if req'd) for xid
        //_enq_hdr.hdr_copy(h);
        ::rec_hdr_copy(&_enq_hdr._rhdr, &h);
        ifsp->read((char*)&_enq_hdr._xidsize, sizeof(std::size_t));
#if defined(JRNL_32_BIT)
        ifsp->ignore(sizeof(uint32_t)); // _filler0
#endif
        ifsp->read((char*)&_enq_hdr._dsize, sizeof(std::size_t));
#if defined(JRNL_32_BIT)
        ifsp->ignore(sizeof(uint32_t)); // _filler1
#endif
        rec_offs = sizeof(_enq_hdr);
        if (_enq_hdr._xidsize)
        {
            _buff = std::malloc(_enq_hdr._xidsize);
            MALLOC_CHK(_buff, "_buff", "enq_rec", "rcv_decode");
        }
    }
    if (rec_offs < sizeof(_enq_hdr) + _enq_hdr._xidsize)
    {
        // Read xid (or continue reading xid)
        std::size_t offs = rec_offs - sizeof(_enq_hdr);
        ifsp->read((char*)_buff + offs, _enq_hdr._xidsize - offs);
        std::size_t size_read = ifsp->gcount();
        rec_offs += size_read;
        if (size_read < _enq_hdr._xidsize - offs)
        {
            assert(ifsp->eof());
            // As we may have read past eof, turn off fail bit
            ifsp->clear(ifsp->rdstate()&(~std::ifstream::failbit));
            assert(!ifsp->fail() && !ifsp->bad());
            return false;
        }
    }
    if (!::is_enq_external(&_enq_hdr))
    {
        if (rec_offs < sizeof(_enq_hdr) + _enq_hdr._xidsize +  _enq_hdr._dsize)
        {
            // Ignore data (or continue ignoring data)
            std::size_t offs = rec_offs - sizeof(_enq_hdr) - _enq_hdr._xidsize;
            ifsp->ignore(_enq_hdr._dsize - offs);
            std::size_t size_read = ifsp->gcount();
            rec_offs += size_read;
            if (size_read < _enq_hdr._dsize - offs)
            {
                assert(ifsp->eof());
                // As we may have read past eof, turn off fail bit
                ifsp->clear(ifsp->rdstate()&(~std::ifstream::failbit));
                assert(!ifsp->fail() && !ifsp->bad());
                return false;
            }
        }
    }
    if (rec_offs < sizeof(_enq_hdr) + _enq_hdr._xidsize +
            (::is_enq_external(&_enq_hdr) ? 0 : _enq_hdr._dsize) + sizeof(rec_tail_t))
    {
        // Read tail (or continue reading tail)
        std::size_t offs = rec_offs - sizeof(_enq_hdr) - _enq_hdr._xidsize;
        if (!::is_enq_external(&_enq_hdr))
            offs -= _enq_hdr._dsize;
        ifsp->read((char*)&_enq_tail + offs, sizeof(rec_tail_t) - offs);
        std::size_t size_read = ifsp->gcount();
        rec_offs += size_read;
        if (size_read < sizeof(rec_tail_t) - offs)
        {
            assert(ifsp->eof());
            // As we may have read past eof, turn off fail bit
            ifsp->clear(ifsp->rdstate()&(~std::ifstream::failbit));
            assert(!ifsp->fail() && !ifsp->bad());
            return false;
        }
    }
    ifsp->ignore(rec_size_dblks() * QLS_DBLK_SIZE_BYTES - rec_size());
    chk_tail(); // Throws if tail invalid or record incomplete
    assert(!ifsp->fail() && !ifsp->bad());
    return true;
}

std::size_t
enq_rec::get_xid(void** const xidpp)
{
    if (!_buff || !_enq_hdr._xidsize)
    {
        *xidpp = 0;
        return 0;
    }
    *xidpp = _buff;
    return _enq_hdr._xidsize;
}

std::size_t
enq_rec::get_data(void** const datapp)
{
    if (!_buff)
    {
        *datapp = 0;
        return 0;
    }
    if (::is_enq_external(&_enq_hdr))
        *datapp = 0;
    else
        *datapp = (void*)((char*)_buff + _enq_hdr._xidsize);
    return _enq_hdr._dsize;
}

std::string&
enq_rec::str(std::string& str) const
{
    std::ostringstream oss;
    oss << "enq_rec: m=" << _enq_hdr._rhdr._magic;
    oss << " v=" << (int)_enq_hdr._rhdr._version;
    oss << " rid=" << _enq_hdr._rhdr._rid;
    if (_xidp)
        oss << " xid=\"" << _xidp << "\"";
    oss << " len=" << _enq_hdr._dsize;
    str.append(oss.str());
    return str;
}

std::size_t
enq_rec::rec_size() const
{
    return rec_size(_enq_hdr._xidsize, _enq_hdr._dsize, ::is_enq_external(&_enq_hdr));
}

std::size_t
enq_rec::rec_size(const std::size_t xidsize, const std::size_t dsize, const bool external)
{
    if (external)
        return sizeof(enq_hdr_t) + xidsize + sizeof(rec_tail_t);
    return sizeof(enq_hdr_t) + xidsize + dsize + sizeof(rec_tail_t);
}

void
enq_rec::set_rid(const uint64_t rid)
{
    _enq_hdr._rhdr._rid = rid;
    _enq_tail._rid = rid;
}

void
enq_rec::chk_hdr() const
{
    jrec::chk_hdr(_enq_hdr._rhdr);
    if (_enq_hdr._rhdr._magic != QLS_ENQ_MAGIC)
    {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        oss << "enq magic: rid=0x" << std::setw(16) << _enq_hdr._rhdr._rid;
        oss << ": expected=0x" << std::setw(8) << QLS_ENQ_MAGIC;
        oss << " read=0x" << std::setw(2) << (int)_enq_hdr._rhdr._magic;
        throw jexception(jerrno::JERR_JREC_BADRECHDR, oss.str(), "enq_rec", "chk_hdr");
    }
}

void
enq_rec::chk_hdr(uint64_t rid) const
{
    chk_hdr();
    jrec::chk_rid(_enq_hdr._rhdr, rid);
}

void
enq_rec::chk_tail() const
{
    jrec::chk_tail(_enq_tail, _enq_hdr._rhdr);
}

void
enq_rec::clean()
{
    // clean up allocated memory here
}

}}
