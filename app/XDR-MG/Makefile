#
# (C) Copyright 2007-2010
# Hu Chunlin <chunlin.hu@gmail.com>
#
# Makefile - To make my life easier.
#

ROOTDIR = $(shell cd ~/work/ ; pwd ; cd - > /dev/null)
export ROOTDIR

#-------------------------------------------------------------------------------
# Targets
#-------------------------------------------------------------------------------
mexe = mg_xdr
mg_xdr-objs = mg.o mg_compose.o mg_parse.o mg_userdata.o redis_help.o \
		redis/my_loop.o redis/my_poll.o

#-------------------------------------------------------------------------------
# Customized flags
#-------------------------------------------------------------------------------
EXTRACFLAGS += -D__STDC_CONSTANT_MACROS -g

IPATH=-I$(ROOTDIR)/include -I/home/yiyang/zzy/redis/include/hiredis

LFLAGS=\
	   -L$(ROOTDIR)/lib -llchadapter \
	   -llchutil \
	   -laplog \
	   -llchmm \
	   -lapfrm \
	   -llchlicense \
	   -L/home/yiyang/zzy/redis/lib -lhiredis

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
-include $(mg_xdr-objs:.o=.o.dep)
-include $(mexe:=.dep)

