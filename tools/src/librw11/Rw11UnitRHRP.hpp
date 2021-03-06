// $Id: Rw11UnitRHRP.hpp 1186 2019-07-12 17:49:59Z mueller $
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2015-2017 by Walter F.J. Mueller <W.F.J.Mueller@gsi.de>
// 
// Revision History: 
// Date         Rev Version  Comment
// 2017-04-07   868   1.0.1  Dump(): add detail arg
// 2015-05-14   680   1.0    Initial version
// 2015-03-21   659   0.1    First draft
// ---------------------------------------------------------------------------


/*!
  \brief   Declaration of class Rw11UnitRHRP.
*/

#ifndef included_Retro_Rw11UnitRHRP
#define included_Retro_Rw11UnitRHRP 1

#include "Rw11UnitDiskBase.hpp"

namespace Retro {

  class Rw11CntlRHRP;                       // forw decl to avoid circular incl

  class Rw11UnitRHRP : public Rw11UnitDiskBase<Rw11CntlRHRP> {
    public:
                    Rw11UnitRHRP(Rw11CntlRHRP* pcntl, size_t index);
                   ~Rw11UnitRHRP();

      virtual void  SetType(const std::string& type);
      uint16_t      Rpdt() const;
      bool          IsRmType() const;

      void          SetRpds(uint16_t rpds);
      uint16_t      Rpds() const;

      virtual void  Dump(std::ostream& os, int ind=0, const char* text=0,
                         int detail=0) const;

    // some constants (also defined in cpp)
      static const uint16_t kDTE_M_RM = kWBit02; //!< rm type flag
      static const uint16_t kDTE_RP04 = 00; //!< drive type of RP04  rm=0
      static const uint16_t kDTE_RP06 = 01; //!< drive type of RP06  rm=0
      static const uint16_t kDTE_RM03 = 04; //!< drive type of RM03  rm=1
      static const uint16_t kDTE_RM80 = 05; //!< drive type of RM80  rm=1
      static const uint16_t kDTE_RM05 = 06; //!< drive type of RM05  rm=1
      static const uint16_t kDTE_RP07 = 07; //!< drive type of RP07  rm=1

    protected:
      uint16_t      fRpdt;                  //!< drive type (encoded)
      uint16_t      fRpds;                  //!< drive status
  };
  
} // end namespace Retro

#include "Rw11UnitRHRP.ipp"

#endif
