# $Id: generic_so_c.mk 1168 2019-06-20 11:52:51Z mueller $
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2014-2019 by Walter F.J. Mueller <W.F.J.Mueller@gsi.de>
#
#  Revision History: 
# Date         Rev Version  Comment
# 2019-06-20  1168   1.1    use -scrUuv for ar to avoid "'u' modifier ignored"
# 2014-11-02   600   1.0    Initial version (cloned from generic_so.mk)
#---
#
# Build a sharable library and an archive
# --> from C sources only!
# --> with $(CC) rather than $(CXX)
#
# Before including, defined the following variables:
#   SOPATH    relative directory path of the library (def: $RETROBASE/tools/lib)
#   SONAME    name of the library
#   SOMAJV    major version number
#   SOMINV    minor version number
#
ifndef SOPATH
SOPATH     = ${RETROBASE}/tools/lib
endif
#
SOFILE     = lib$(SONAME).so
SOFILEV    = lib$(SONAME).so.$(SOMAJV)
SOFILEVV   = lib$(SONAME).so.$(SOMAJV).$(SOMINV)
AFILE      = lib$(SONAME).a
#
.PHONY : libs
libs : $(SOPATH)/$(AFILE) $(SOPATH)/$(SOFILEVV) 
#
# Build the sharable library
#
$(SOPATH)/$(SOFILEVV) : $(OBJ_all)
	if [ ! -d $(SOPATH) ]; then mkdir -p $(SOPATH); fi
	$(CC) -shared -Wl,-soname,$(SOFILEV) -o $(SOPATH)/$(SOFILEVV) \
		$(OBJ_all) $(LDLIBS)
	(cd $(SOPATH); rm -f $(SOFILE)   $(SOFILEV))
	(cd $(SOPATH); ln -s $(SOFILEVV) $(SOFILEV))
	(cd $(SOPATH); ln -s $(SOFILEV)  $(SOFILE))
#
# Build an archive
#
$(SOPATH)/$(AFILE) : $(OBJ_all)
	if [ ! -d $(SOPATH) ]; then mkdir -p $(SOPATH); fi
	ar -scrUuv $(SOPATH)/$(AFILE) $?
#
