
default_target: all

ifeq ($(OS),win32)

#-------------------------------------------------------------------
# Project
#-------------------------------------------------------------------
PRJ_NAME := wxWidgets
PRJ_TYPE := lib
PRJ_INCS := zlib jpeg png tiff wxWidgets/src/regex
PRJ_LIBS := 

PRJ_LIBROOT := ..

#-------------------------------------------------------------------
# Configure build
#-------------------------------------------------------------------
include $(PRJ_LIBROOT)/config.mk

#-------------------------------------------------------------------
# File locations
#-------------------------------------------------------------------
export LOC_TAG := def
LOC_INC_def := $(CFG_LIBROOT)/wxWidgets/include
LOC_SRC_def := $(CFG_LIBROOT)/wxWidgets/src/common
LOC_EXC_def := execcmn fontmgrcmn socketevtdispatch
include $(PRJ_LIBROOT)/build.mk

export LOC_TAG := generic
LOC_INC_generic := $(CFG_LIBROOT)/wxWidgets/include
LOC_SRC_generic := $(CFG_LIBROOT)/wxWidgets/src/generic
LOC_EXC_generic := accel caret colour fdrepdlg grid icon listctrl \
				   msgdlgg notebook paletteg regiong statline timer
LOC_OUT_generic := generic
include $(PRJ_LIBROOT)/build.mk

ifeq ($(OS),win32)

	export LOC_TAG := msw
	LOC_INC_msw := $(CFG_LIBROOT)/wxWidgets/include
	LOC_SRC_msw := $(CFG_LIBROOT)/wxWidgets/src/msw
	LOC_OUT_msw := msw
	LOC_EXC_msw := mediactrl_am mediactrl_qt mediactrl_wmp10
	include $(PRJ_LIBROOT)/build.mk
	
	export LOC_TAG := ole
	LOC_INC_ole := $(CFG_LIBROOT)/wxWidgets/include
	LOC_SRC_ole := $(CFG_LIBROOT)/wxWidgets/src/msw/ole
	LOC_OUT_ole := ole
	include $(PRJ_LIBROOT)/build.mk
	
endif

#-------------------------------------------------------------------
# Execute the build
#-------------------------------------------------------------------
include $(PRJ_LIBROOT)/go.mk

else

all:

endif
