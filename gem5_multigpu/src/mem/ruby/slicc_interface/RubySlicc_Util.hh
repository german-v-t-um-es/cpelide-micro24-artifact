/*
 * Copyright (c) 2020 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 1999-2008 Mark D. Hill and David A. Wood
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * These are the functions that exported to slicc from ruby.
 */

#ifndef __MEM_RUBY_SLICC_INTERFACE_RUBYSLICC_UTIL_HH__
#define __MEM_RUBY_SLICC_INTERFACE_RUBYSLICC_UTIL_HH__

#include <cassert>
#include <climits>

#include "debug/RubySlicc.hh"
#include "mem/packet.hh"
#include "mem/ruby/common/Address.hh"
#include "mem/ruby/common/BoolVec.hh"
#include "mem/ruby/common/DataBlock.hh"
#include "mem/ruby/common/TypeDefines.hh"
#include "mem/ruby/common/WriteMask.hh"
#include "mem/ruby/protocol/RubyRequestType.hh"

inline Cycles zero_time() { return Cycles(0); }

inline Cycles intToCycles(int c) { return Cycles(c); }

inline NodeID
intToID(int nodenum)
{
    NodeID id = nodenum;
    return id;
}

inline int
IDToInt(NodeID id)
{
    int nodenum = id;
    return nodenum;
}

inline int
addressToInt(Addr addr)
{
    assert(!(addr & 0xffffffff00000000));
    return addr;
}

inline Addr
intToAddress(int addr)
{
    assert(!(addr & 0xffffffff00000000));
    return addr;
}

inline int
mod(int val, int mod)
{
    return val % mod;
}

inline int max_tokens()
{
  return 1024;
}

inline bool
isWriteRequest(RubyRequestType type)
{
    if ((type == RubyRequestType_ST) ||
        (type == RubyRequestType_ATOMIC) ||
        (type == RubyRequestType_RMW_Read) ||
        (type == RubyRequestType_RMW_Write) ||
        (type == RubyRequestType_Store_Conditional) ||
        (type == RubyRequestType_Locked_RMW_Read) ||
        (type == RubyRequestType_Locked_RMW_Write) ||
        (type == RubyRequestType_FLUSH) ||
        (type == RubyRequestType_INVL2)) {
            return true;
    } else {
            return false;
    }
}

inline bool
isDataReadRequest(RubyRequestType type)
{
    if ((type == RubyRequestType_LD) ||
        (type == RubyRequestType_Load_Linked)) {
            return true;
    } else {
            return false;
    }
}

inline bool
isReadRequest(RubyRequestType type)
{
    if (isDataReadRequest(type) ||
        (type == RubyRequestType_IFETCH)) {
            return true;
    } else {
            return false;
    }
}

inline bool
isHtmCmdRequest(RubyRequestType type)
{
    if ((type == RubyRequestType_HTM_Start)  ||
        (type == RubyRequestType_HTM_Commit) ||
        (type == RubyRequestType_HTM_Cancel) ||
        (type == RubyRequestType_HTM_Abort)) {
            return true;
    } else {
            return false;
    }
}

inline RubyRequestType
htmCmdToRubyRequestType(const Packet *pkt)
{
    if (pkt->req->isHTMStart()) {
        return RubyRequestType_HTM_Start;
    } else if (pkt->req->isHTMCommit()) {
        return RubyRequestType_HTM_Commit;
    } else if (pkt->req->isHTMCancel()) {
        return RubyRequestType_HTM_Cancel;
    } else if (pkt->req->isHTMAbort()) {
        return RubyRequestType_HTM_Abort;
    }
    else {
        panic("invalid ruby packet type\n");
    }
}

inline int
addressOffset(Addr addr, Addr base)
{
    assert(addr >= base);
    Addr offset = addr - base;
    // sanity checks if fits in an int
    assert(offset < INT_MAX);
    return offset;
}

/**
 * This function accepts an address, a data block and a packet. If the address
 * range for the data block contains the address which the packet needs to
 * read, then the data from the data block is written to the packet. True is
 * returned if the data block was read, otherwise false is returned.
 *
 * This is used during a functional access "search the world" operation. The
 * functional access looks in every place that might hold a valid data block
 * and, if it finds one, checks to see if it is holding the address the access
 * is searching for. During the access check, the WriteMask could be in any
 * state, including empty.
 */
inline bool
testAndRead(Addr addr, DataBlock& blk, Packet *pkt)
{
    Addr pktLineAddr = makeLineAddress(pkt->getAddr());
    Addr lineAddr = makeLineAddress(addr);

    if (pktLineAddr == lineAddr) {
        uint8_t *data = pkt->getPtr<uint8_t>();
        unsigned int size_in_bytes = pkt->getSize();
        unsigned startByte = pkt->getAddr() - lineAddr;

        for (unsigned i = 0; i < size_in_bytes; ++i) {
            data[i] = blk.getByte(i + startByte);
        }
        return true;
    }
    return false;
}

/**
 * This function accepts an address, a data block, a write mask and a packet.
 * If the valid address range for the data block contains the address which
 * the packet needs to read, then the data from the data block is written to
 * the packet. True is returned if any part of the data block was read,
 * otherwise false is returned.
 */
inline bool
testAndReadMask(Addr addr, DataBlock& blk, WriteMask& mask, Packet *pkt)
{
    Addr pktLineAddr = makeLineAddress(pkt->getAddr());
    Addr lineAddr = makeLineAddress(addr);

    if (pktLineAddr == lineAddr) {
        uint8_t *data = pkt->getPtr<uint8_t>();
        unsigned int size_in_bytes = pkt->getSize();
        unsigned startByte = pkt->getAddr() - lineAddr;
        bool was_read = false;

        for (unsigned i = 0; i < size_in_bytes; ++i) {
            if (mask.test(i + startByte)) {
                was_read = true;
                data[i] = blk.getByte(i + startByte);
            }
        }
        return was_read;
    }
    return false;
}

/**
 * This function accepts an address, a data block and a packet. If the address
 * range for the data block contains the address which the packet needs to
 * write, then the data from the packet is written to the data block. True is
 * returned if the data block was written, otherwise false is returned.
 */
inline bool
testAndWrite(Addr addr, DataBlock& blk, Packet *pkt)
{
    Addr pktLineAddr = makeLineAddress(pkt->getAddr());
    Addr lineAddr = makeLineAddress(addr);

    if (pktLineAddr == lineAddr) {
        const uint8_t *data = pkt->getConstPtr<uint8_t>();
        unsigned int size_in_bytes = pkt->getSize();
        unsigned startByte = pkt->getAddr() - lineAddr;

        for (unsigned i = 0; i < size_in_bytes; ++i) {
            blk.setByte(i + startByte, data[i]);
        }
        return true;
    }
    return false;
}

inline int
countBoolVec(BoolVec bVec)
{
    int count = 0;
    for (const bool e: bVec) {
        if (e) {
            count++;
        }
    }
    return count;
}

#endif //__MEM_RUBY_SLICC_INTERFACE_RUBYSLICC_UTIL_HH__
