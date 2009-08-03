
default_target: all

#-------------------------------------------------------------------
# Project
#-------------------------------------------------------------------
PRJ_NAME := gstreamer
PRJ_TYPE := lib
PRJ_INCS := gstreamer glib glib/glib glib/gmodule glib/gobject \
			libxml/include winglib/dep/etc/libxml/inc/posix
PRJ_LIBS := 
PRJ_DEFS := HAVE_CONFIG_H=1 
PRJ_LIBROOT := ..

#-------------------------------------------------------------------
# Configure build
#-------------------------------------------------------------------
include $(PRJ_LIBROOT)/config.mk

ifndef BUILD_GSTREAMER
UNSUPPORTED := Set make option BUILD_GSTREAMER=1 to build
include $(PRJ_LIBROOT)/unsupported.mk
else

#-------------------------------------------------------------------
# File locations
#-------------------------------------------------------------------

export LOC_TAG := gst
LOC_CXX_gst := c
LOC_SRC_gst := $(CFG_LIBROOT)/gstreamer/gst
LOC_EXC_gst := gstregistryxml
include $(PRJ_LIBROOT)/build.mk

export LOC_TAG := gst_parse
LOC_CXX_gst_parse := c
LOC_SRC_gst_parse := $(CFG_LIBROOT)/gstreamer/gst/parse
#LOC_EXC_gst_parse := 
include $(PRJ_LIBROOT)/build.mk


#-------------------------------------------------------------------
# Execute the build
#-------------------------------------------------------------------
include $(PRJ_LIBROOT)/go.mk

endif
