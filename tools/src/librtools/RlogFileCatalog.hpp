// $Id: RlogFileCatalog.hpp 1084 2018-12-16 12:23:53Z mueller $
//
// Copyright 2011-2018 by Walter F.J. Mueller <W.F.J.Mueller@gsi.de>
//
// This program is free software; you may redistribute and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY, without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for complete details.
// 
// Revision History: 
// Date         Rev Version  Comment
// 2018-12-16  1084   1.0.2  use =delete for noncopyable instead of boost
// 2018-12-07  1078   1.0.1  use std::shared_ptr instead of boost
// 2013-02-22   491   1.0    Initial version
// ---------------------------------------------------------------------------

/*!
  \file
  \brief   Declaration of class RlogFileCatalog.
*/

#ifndef included_Retro_RlogFileCatalog
#define included_Retro_RlogFileCatalog 1

#include <map>
#include <memory>

#include "RlogFile.hpp"

namespace Retro {

  class RlogFileCatalog {
    public:

      static RlogFileCatalog&  Obj();    

      const std::shared_ptr<RlogFile>& FindOrCreate(const std::string& name);
      void          Delete(const std::string& name);

    private:
                    RlogFileCatalog();
                   ~RlogFileCatalog();

                    RlogFileCatalog(const RlogFileCatalog&) = delete; // noncopy 
      RlogFileCatalog& operator=(const RlogFileCatalog&) = delete;    // noncopy

  protected:
      typedef std::map<std::string, std::shared_ptr<RlogFile>> map_t;

      map_t         fMap;                   //!< name->rlogfile map
  };
  
} // end namespace Retro

//#include "RlogFileCatalog.ipp"

#endif
