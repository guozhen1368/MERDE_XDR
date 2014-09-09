
# (C) Copyright 2007-2010
# Hu Chunlin <chunlin.hu@gmail.com>
#
# Makefile - To make my life easier.
#

ROOTDIR = $(shell cd ../hlyt ; pwd ; cd - > /dev/null)
export ROOTDIR
D=2

#-------------------------------------------------------------------------------
# Targets
#-------------------------------------------------------------------------------
mexe = appsvr_proxy
appsvr_proxy-objs= main.o ap_adapter.o

#-------------------------------------------------------------------------------
# Customized flags
#-------------------------------------------------------------------------------
EXTRACFLAGS += -D__STDC_CONSTANT_MACROS

IPATH=-I./ -I$(ROOTDIR)/include

LFLAGS=\
       -Wl,-rpath,$(ROOTDIR)/lib -L$(ROOTDIR)/lib \
       -llchadapter \
       -llchutil \
       -laplog \
       -llchmm \
       -lapfrm \
       -llchlicense \
       -llchsdtp

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
-include $(mexe:=.o.dep)
