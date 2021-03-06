// $Id: RlinkPacketBufRcv.cpp 1198 2019-07-27 19:08:31Z mueller $
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2014-2019 by Walter F.J. Mueller <W.F.J.Mueller@gsi.de>
// 
// Revision History: 
// Date         Rev Version  Comment
// 2019-07-27  1198   1.2.3  add Nak handling
// 2019-06-14  1163   1.2.2  ReadData(): coverity fixup (logically dead code)
// 2018-12-23  1091   1.2.1  ReadData(): remove port open check, done at caller
// 2018-12-08  1079   1.2    use ref not ptr for RlinkPort
// 2017-04-07   868   1.1.1  Dump(): add detail arg
// 2017-02-19   853   1.1    use Rtime
// 2014-12-25   621   1.0.1  Reorganize packet send/revd stats
// 2014-11-30   607   1.0    Initial version
// 2014-11-02   600   0.1    First draft (re-organize PacketBuf for rlink v4)
// ---------------------------------------------------------------------------

/*!
  \brief   Implemenation of class RlinkPacketBuf.
 */

#include <sys/time.h>

#include <iostream>

#include "RlinkPacketBufRcv.hpp"

#include "librtools/RosFill.hpp"
#include "librtools/RosPrintf.hpp"
#include "librtools/RosPrintBvi.hpp"
#include "librtools/Rexception.hpp"

using namespace std;

/*!
  \class Retro::RlinkPacketBufRcv
  \brief FIXME_docs
*/

// all method definitions in namespace Retro
namespace Retro {

//------------------------------------------+-----------------------------------
//! Default constructor

RlinkPacketBufRcv::RlinkPacketBufRcv()
  : fRawBufSize(0),
    fRawBufDone(0),
    fRcvState(kRcvIdle),
    fNDone(0),
    fEscSeen(false),
    fNakIndex(-1),
    fNakCode(0),
    fDropData()
{
  // Statistic setup
  fStats.Define(kStatNRxPktByt,    "NRxPktByt",    "Rx packet bytes rcvd");
  fStats.Define(kStatNRxDrop,      "NRxDrop",      "Rx bytes dropped");
  fStats.Define(kStatNRxSop,       "NRxSop",       "Rx SOP commas seen");
  fStats.Define(kStatNRxEop,       "NRxEop",       "Rx EOP commas seen");
  fStats.Define(kStatNRxNak,       "NRxNak",       "Rx NAK commas seen");
  fStats.Define(kStatNRxAttn,      "NRxAttn",      "Rx ATTN commas seen");
  fStats.Define(kStatNRxEsc,       "NRxEsc",       "Rx data escapes");
  fStats.Define(kStatNRxClobber,   "NRxClobber",   "Rx clobbered escapes");
  fStats.Define(kStatNRxNakCcrc,   "NRxNakCcrc",   "Rx NAK Ccrc seen");
  fStats.Define(kStatNRxNakDcrc,   "NRxNakDcrc",   "Rx NAK Dcrc seen");
  fStats.Define(kStatNRxNakFrame,  "NRxNakFrame",  "Rx NAK Frame seen");
  fStats.Define(kStatNRxNakUnused, "NRxNakUnused", "Rx NAK Unused seen");
  fStats.Define(kStatNRxNakCmd,    "NRxNakCmd",    "Rx NAK Cmd seen");
  fStats.Define(kStatNRxNakCnt,    "NRxNakCnt",    "Rx NAK Cnt seen");
  fStats.Define(kStatNRxNakRtOvlf, "NRxNakRtOvlf", "Rx NAK RtOvlf seen");
  fStats.Define(kStatNRxNakRtWblk, "NRxNakRtWblk", "Rx NAK RtWblk seen");
  fStats.Define(kStatNRxNakInval,  "NRxNakInval",  "Rx NAK invalid seen");
}

//------------------------------------------+-----------------------------------
//! Destructor

RlinkPacketBufRcv::~RlinkPacketBufRcv()
{}

//------------------------------------------+-----------------------------------
//! FIXME_docs

int RlinkPacketBufRcv::ReadData(RlinkPort& port, const Rtime& timeout, 
                                RerrMsg& emsg)
{
  if (fRawBufDone != fRawBufSize)
    throw Rexception("RlinkPacketBufRcv::ReadData()", 
                     "Bad state: called while data pending in buffer");

  fRawBufDone = 0;
  fRawBufSize = 0;

  int irc = port.Read(fRawBuf, sizeof(fRawBuf), timeout, emsg);

  if (timeout.IsZero() && irc == RlinkPort::kTout) return 0;

  if (irc <= 0) {
    if (irc == RlinkPort::kTout) {
      SetFlagBit(kFlagErrTout);
    } else {
      SetFlagBit(kFlagErrIO);
      if (irc == RlinkPort::kEof) {
        emsg.Init("RlinkPacketBuf::ReadData()", "eof on read");
      }
    }
  } else {
    fRawBufSize = size_t(irc);
  }
  
  return irc;
}

//------------------------------------------+-----------------------------------
//! FIXME_docs

bool RlinkPacketBufRcv::ProcessData()
{
  if (fRawBufDone ==fRawBufSize) return false;

  while (fRawBufDone < fRawBufSize) {
    switch (fRcvState) {
    case kRcvIdle:
      ProcessDataIdle();
      break;
      
    case kRcvFill:
      ProcessDataFill();
      break;
      
    default:
      return true;
    }
  }
  return true;
}

//------------------------------------------+-----------------------------------
//! FIXME_docs

void RlinkPacketBufRcv::AcceptPacket()
{
  fPktBuf.clear();
  fCrc.Clear();
  fFlags    = 0;
  fRcvState = kRcvIdle;
  fNDone    = 0;
  fNakIndex = -1;
  fNakCode  =  0;
  fDropData.clear();
  return;
}
  
//------------------------------------------+-----------------------------------
//! FIXME_docs

void RlinkPacketBufRcv::FlushRaw()
{
  fRawBufSize = 0;
  fRawBufDone = 0;
  fEscSeen    = false;
  return;
}
  
//------------------------------------------+-----------------------------------
//! FIXME_docs

RlinkPacketBufRcv::pkt_state RlinkPacketBufRcv::PacketState()
{
  if (fRcvState==kRcvIdle || fRcvState==kRcvFill) return kPktPend;
  if (fRcvState==kRcvDone) return TestFlag(kFlagSopSeen) ? kPktResp : kPktAttn;
  return kPktError;
}

//------------------------------------------+-----------------------------------
//! FIXME_docs

void RlinkPacketBufRcv::GetWithCrc(uint16_t* pdata, size_t count)
{
  uint16_t* pend = pdata + count;
  while (pdata < pend) GetWithCrc(*pdata++);
  return;
}

//------------------------------------------+-----------------------------------
//! FIXME_docs

void RlinkPacketBufRcv::Dump(std::ostream& os, int ind, const char* text,
                             int detail) const
{
  RosFill bl(ind);
  os << bl << (text?text:"--") << "RlinkPacketBufRcv @ " << this << endl;
  os << bl << "  fRawBufSize:   " << RosPrintf(fRawBufSize,"d",4) << endl;
  os << bl << "  fRawBufDone:   " << RosPrintf(fRawBufDone,"d",4) << endl;
  if (fRawBufDone > 0) {
    os << bl << "  fRawBuf[last]: " 
       << RosPrintBvi(fRawBuf[fRawBufDone-1],16) << endl;
  }
  os << bl << "  fRcvState:     " << RosPrintf(fRcvState,"d",4) << endl;
  os << bl << "  fNDone:        " << RosPrintf(fNDone,"d",4) << endl;
  os << bl << "  fEscSeen:      " << RosPrintf(fEscSeen) << endl;
  os << bl << "  fNakIndex:     " << RosPrintf(fNakIndex,"d",4) << endl;
  os << bl << "  fNakCode:      " << RosPrintf(fNakCode,"d",4) << endl;

  os << bl << "  fDropData.size:" << RosPrintf(fDropData.size(),"d",4);
  size_t ncol  = max(1, (80-ind-4-6)/(2+1));
  for (size_t i=0; i<fDropData.size(); i++) {
    if (i%ncol == 0) os << "\n" << bl << "    " << RosPrintf(i,"d",4) << ": ";
    os << RosPrintBvi(fDropData[i],16) << " ";
  }
  os << endl;

  RlinkPacketBuf::Dump(os, ind, " ^", detail);
  return;
}

//------------------------------------------+-----------------------------------
//! FIXME_docs

void RlinkPacketBufRcv::ProcessDataIdle()
{
  // loop over buffer
  while (fRawBufDone < fRawBufSize) {

    // handle escapes
    if (fEscSeen) {
      uint8_t ec = GetEcode();
      switch (ec) {
        
      case kEcSop:
        SetFlagBit(kFlagSopSeen);
        fRcvState = kRcvFill;
        return;
        
      case kEcAttn:
        SetFlagBit(kFlagAttnSeen);
        fRcvState = kRcvFill;
        return;

      default:
        fDropData.push_back(kSymEsc);
        fDropData.push_back(fRawBuf[fRawBufDone-1]);
        fStats.Inc(kStatNRxDrop, 2.);
        break;
      }
    } //if (fEscSeen)

    // handle plain data (till next escape)
    uint8_t* pi   = fRawBuf+fRawBufDone;
    uint8_t* pend = fRawBuf+fRawBufSize;
    
    while (pi < pend) {
      uint8_t c = *pi++;
      if (c == kSymEsc) {
        fEscSeen = true;
        break;
      }
      fDropData.push_back(c);
      fStats.Inc(kStatNRxDrop);
    }
    fRawBufDone = pi - fRawBuf;

  } // while (fRawBufDone < fRawBufSize)
  
  return;
}

//------------------------------------------+-----------------------------------
//! FIXME_docs

void RlinkPacketBufRcv::ProcessDataFill()
{
  // loop over buffer
  while (fRawBufDone < fRawBufSize) {

    // handle escapes
    if (fEscSeen) {
      uint8_t ec = GetEcode();
      switch (ec) {
        
      case kEcEop:                          // EOP seen
        SetFlagBit(kFlagEopSeen);           // -> set eop and return
        fRcvState = kRcvDone;
        fStats.Inc(kStatNRxPktByt, double(PktSize()));
        return;
        
      case kEcNak:                          // NAK seen
        if (TestFlag(kFlagAttnSeen|kFlagNakSeen)) {  // NAK after ATTN or NAK
          SetFlagBit(kFlagErrFrame);          // -> set frame error and return
          fRcvState = kRcvError;
          return;
        }                                   // else 1st NAK
        
        SetFlagBit(kFlagNakSeen);             // -> set flag and index; continue
        fNakIndex = fPktBuf.size();
        if (fRawBufDone+1 < fRawBufSize) {
          uint8_t nc    = fRawBuf[fRawBufDone];
          uint8_t ncpre =    nc      & 0xc0;
          uint8_t ncneg = ((~nc)>>3) & 0x07;
          uint8_t ncpos =    nc      & 0x07;
          fNakCode = (ncpre == 0x80 && ncneg == ncpos) ? ncpos : kNcInval;
        } else {
          fNakCode = kNcInval;
        }
        switch (fNakCode) {
        case kNcCcrc:   fStats.Inc(kStatNRxNakCcrc);   break; // NAK Ccrc   seen
        case kNcDcrc:   fStats.Inc(kStatNRxNakDcrc);   break; // NAK Dcrc   seen
        case kNcFrame:  fStats.Inc(kStatNRxNakFrame);  break; // NAK Frame  seen
        case kNcUnused: fStats.Inc(kStatNRxNakUnused); break; // NAK Unused seen
        case kNcCmd:    fStats.Inc(kStatNRxNakCmd);    break; // NAK Cmd    seen
        case kNcCnt:    fStats.Inc(kStatNRxNakCnt);    break; // NAK Cnt    seen
        case kNcRtOvlf: fStats.Inc(kStatNRxNakRtOvlf); break; // NAK RtOvlf seen
        case kNcRtWblk: fStats.Inc(kStatNRxNakRtWblk); break; // NAK RtWblk seen
        case kNcInval:  fStats.Inc(kStatNRxNakInval);  break; // NAK invalid seen
        }
 
        break;
        
      // data escapes seen: add escaped char and continue
      case kEcXon:   fPktBuf.push_back(kSymXon);  break;
      case kEcXoff:  fPktBuf.push_back(kSymXoff); break;
      case kEcFill:  fPktBuf.push_back(kSymFill); break;
      case kEcEsc:   fPktBuf.push_back(kSymEsc);  break;
        
      case kEcClobber:                      // Clobber(ed) escape seen
        SetFlagBit(kFlagErrClobber);        // -> set clobber error and return
        fRcvState = kRcvError;
        return;
        
      default:                              // unexpected escape (SOP,ATTN)
        SetFlagBit(kFlagErrFrame);          // -> set frame error and return
        fRcvState = kRcvError;
        return;
      }
    } // if (fEscSeen)

    // handle plain data (till next escape)
    uint8_t* pi   = fRawBuf+fRawBufDone;
    uint8_t* pend = fRawBuf+fRawBufSize;
    
    while (pi < pend) {
      uint8_t c = *pi++;
      if (c == kSymEsc) {
        fEscSeen = true;
        break;
      }
      fPktBuf.push_back(c);
    }
    fRawBufDone = pi - fRawBuf;

  } // while (fRawBufDone < fRawBufSize)
  
  return;
}

//------------------------------------------+-----------------------------------
//! FIXME_docs

uint8_t RlinkPacketBufRcv::GetEcode()
{
  if (!fEscSeen || fRawBufDone >= fRawBufSize)
    throw Rexception("RlinkPacketBufRcv::GetEcode()", "Bad state");

  fEscSeen = false;

  uint8_t c  = fRawBuf[fRawBufDone++];
  uint8_t ec = c & 0x7;
  if ((c & 0xC0) != kSymEdPref || (((~c)>>3)&0x7) != ec) ec = kEcClobber;

  switch (ec) {
  case kEcSop:     fStats.Inc(kStatNRxSop);     break; // SOP  comma seen
  case kEcEop:     fStats.Inc(kStatNRxEop);     break; // EOP  comma seen
  case kEcNak:     fStats.Inc(kStatNRxNak);     break; // NAK  comma seen
  case kEcAttn:    fStats.Inc(kStatNRxAttn);    break; // ATTN comma seen
  case kEcClobber: fStats.Inc(kStatNRxClobber); break; // clobbered esc seen
  default:         fStats.Inc(kStatNRxEsc);     break; // escaped data seen
  }

  return ec;
}

} // end namespace Retro
