# $Id: Makefile 1176 2019-06-30 07:16:06Z mueller $
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2013- by Walter F.J. Mueller <W.F.J.Mueller@gsi.de>
#
#  Revision History: 
# Date         Rev Version  Comment
# 2013-02-10   485   1.0    Initial version
#
# Compile and Link search paths
#
include ../checkpath_cpp.mk
#
INCLFLAGS  = -I${TCLINC} -I${RETROBASE}/tools/src
LDLIBS     = -lreadline
LDLIBS    += -L${TCLLIB} -l${TCLLIBNAME} -lreadline
LDLIBS    += -L${RETROBASE}/tools/lib -lrtcltools
LDLIBS    +=   -lrutiltpp -lrlinktpp -lrwxxtpp
#LDLIBS    +=   -lrusbtpp
#
# Object files to be included
#
OBJ_all    = testtclsh.o
#
DEP_all    = $(OBJ_all:.o=.dep)
#
testtclsh : $(OBJ_all)

#- generic part ----------------------------------------------------------------
#
include ${RETROBASE}/tools/make/generic_cpp.mk
include ${RETROBASE}/tools/make/generic_dep.mk
include ${RETROBASE}/tools/make/dontincdep.mk
#
# The magic auto-dependency include
#
ifndef DONTINCDEP
include $(DEP_all)
endif
#
# cleanup phonies:
#
.PHONY    : clean cleandep distclean
clean     :
	@ rm -f $(OBJ_all)
	@ echo "Object files removed"
#
cleandep  :
	@ rm -f $(DEP_all)
	@ echo "Dependency files removed"
#
