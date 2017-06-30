#
# Makefile
#
# $Id$
#
# Makefile for OSP UrlPa
#

BUNDLE_TOOL = $(POCO_BASE)/OSP/BundleCreator/$(POCO_HOST_BINDIR)/bundle

include $(POCO_BASE)/build/rules/global

objects = UrlPa

target         = com.appinf.osp.url-pa
target_version = 1
target_libs    = PocoOSP PocoUtil PocoJSON PocoXML PocoFoundation

postbuild      = $(SET_LD_LIBRARY_PATH) $(BUNDLE_TOOL) -n$(OSNAME) -a$(OSARCH) -o../bundles UrlPa.bndlspec

include $(POCO_BASE)/build/rules/dylib
