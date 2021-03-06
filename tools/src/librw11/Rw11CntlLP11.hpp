// $Id: Rw11CntlLP11.hpp 1185 2019-07-12 17:29:12Z mueller $
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2013-2019 by Walter F.J. Mueller <W.F.J.Mueller@gsi.de>
// 
// Revision History:
// Date         Rev Version  Comment
// 2019-05-30  1155   1.2.3  size->fuse rename
// 2019-04-14  1131   1.2.2  remove SetOnline(), use UnitSetup()
// 2019-04-07  1127   1.2.1  add fQueBusy and queue protection
// 2019-03-17  1123   1.2    buf.val in msb; add lp11_buf readout
// 2017-04-02   865   1.1.1  Dump(): add detail arg
// 2014-12-29   623   1.1    adopt to Rlink V4 attn logic
// 2013-05-01   513   1.0    Initial version
// ---------------------------------------------------------------------------


/*!
  \brief   Declaration of class Rw11CntlLP11.
*/

#ifndef included_Retro_Rw11CntlLP11
#define included_Retro_Rw11CntlLP11 1

#include "Rw11CntlBase.hpp"
#include "Rw11UnitLP11.hpp"

namespace Retro {

  class Rw11CntlLP11 : public Rw11CntlBase<Rw11UnitLP11,1> {
    public:

                    Rw11CntlLP11();
                   ~Rw11CntlLP11();

      void          Config(const std::string& name, uint16_t base, int lam);

      virtual void  Start();

      virtual void  UnitSetup(size_t ind);

      void          SetRlim(uint16_t rlim);
      uint16_t      Rlim() const;
    
      uint16_t      Itype() const;
      bool          Buffered() const;
      uint16_t      FifoSize() const;

      virtual void  Dump(std::ostream& os, int ind=0, const char* text=0,
                         int detail=0) const;

    // some constants (also defined in cpp)
      static const uint16_t kIbaddr = 0177514; //!< LP11 default address
      static const int      kLam    = 8;       //!< LP11 default lam 

      static const uint16_t kCSR = 000;  //!< CSR reg offset
      static const uint16_t kBUF = 002;  //!< BUF reg offset

      static const uint16_t kProbeOff = kCSR;  //!< probe address offset (rcsr)
      static const bool     kProbeInt = true;  //!< probe int active
      static const bool     kProbeRem = true;  //!< probr rem active
 
      static const uint16_t kFifoMaxSize  = 127;     //!< maximal fifo size
    
      static const uint16_t kCSR_M_ERROR = kWBit15; //!< csr.err mask
      static const uint16_t kCSR_V_RLIM  = 12;      //!< csr.rlim shift 
      static const uint16_t kCSR_B_RLIM  = 007;     //!< csr.rlim bit mask
      static const uint16_t kCSR_V_TYPE  =  8;      //!< csr.type shift
      static const uint16_t kCSR_B_TYPE  = 0007;    //!< csr.type bit mask
      static const uint16_t kBUF_M_VAL   = kWBit15; //!< buf.val mask
      static const uint16_t kBUF_V_FUSE  =  8;      //!< buf.fuse shift
      static const uint16_t kBUF_B_FUSE  = 0177;    //!< buf.fuse bit mask
      static const uint16_t kBUF_M_DATA  = 0177;    //!< buf data mask

    // statistics counter indices
      enum stats {
        kStatNQue= Rw11Cntl::kDimStat,      //!< queue rblk
        kStatNNull,                         //!< send null char
        kStatNChar,                         //!< send char
        kStatNLine,                         //!< send lines
        kStatNPage,                         //!< send pages
        kDimStat
      };    

    protected:
      int           AttnHandler(RlinkServer::AttnArgs& args);
      void          ProcessUnbuf(uint16_t buf);
      void          WriteChar(uint8_t ochr);
      void          ProcessBuf(const RlinkCommand& cmd, bool prim);
      int           RcvHandler();

    protected:
      size_t        fPC_buf;                //!< PrimClist: buf index
      uint16_t      fRlim;                  //!< interrupt rate limit
      uint16_t      fItype;                 //!< interface type
      uint16_t      fFsize;                 //!< fifo size
      uint16_t      fRblkSize;              //!< rblk chunk size
      bool          fQueBusy;               //!< rcv queue busy
  };
  
} // end namespace Retro

#include "Rw11CntlLP11.ipp"

#endif
