#
# (C) Copyright 2007-2010
# Hu Chunlin <chunlin.hu@gmail.com>
#
# Makefile - To make my life easier.
#

ROOTDIR = $(shell cd ../.. ; pwd ; cd - > /dev/null)
export ROOTDIR

#-------------------------------------------------------------------------------
# Targets
#-------------------------------------------------------------------------------
exe = itf

#-------------------------------------------------------------------------------
# Customized flags
#-------------------------------------------------------------------------------
EXTRACFLAGS += -D__STDC_CONSTANT_MACROS

IPATH=-I./ -I../../include

LFLAGS=\
	   -L../../lib -llchadapter \
	   -L../../lib -llchutil \
	   -L../../lib -laplog \
	   -L../../lib -llchmm \
	   -L../../lib -lapfrm \
	   -L../../lib -llchlicense

#-------------------------------------------------------------------------------
# The real stuff to do the tricks.
#-------------------------------------------------------------------------------
ifneq ($(DAP_CONFIG_MK_INCLUDED),1)
-include $(ROOTDIR)/config.mk
endif
ifneq ($(DAP_MODULE_APP),)
DAPMFLAG+=-DDAP_CURRENT_MODULE=$(DAP_MODULE_APP)
endif

include $(ROOTDIR)/files.mk

#-------------------------------------------------------------------------------
# Dependency, customized
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Dependency, automatically generated.
#-------------------------------------------------------------------------------
-include $(exe:=.o.dep)

