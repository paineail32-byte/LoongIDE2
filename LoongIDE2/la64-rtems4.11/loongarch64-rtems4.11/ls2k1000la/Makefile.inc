#
# BSP specific settings. To be included in application Makefiles
#

RTEMS_BSP = ls2k1000la

prefix = /opt/la64-rtems4.11-rc1.6
exec_prefix = /opt/la64-rtems4.11-rc1.6/loongarch64-rtems4.11

CC_FOR_TARGET = loongarch64-rtems4.11-gcc --pipe
CXX_FOR_TARGET = loongarch64-rtems4.11-g++
AS_FOR_TARGET = loongarch64-rtems4.11-as
AR_FOR_TARGET = loongarch64-rtems4.11-ar
NM_FOR_TARGET = loongarch64-rtems4.11-nm
LD_FOR_TARGET = loongarch64-rtems4.11-ld
SIZE_FOR_TARGET = loongarch64-rtems4.11-size
OBJCOPY_FOR_TARGET = loongarch64-rtems4.11-objcopy

RTEMS_API = @RTEMS_API@

CC= $(CC_FOR_TARGET)
CXX= $(CXX_FOR_TARGET)
AS= $(AS_FOR_TARGET)
LD= $(LD_FOR_TARGET)
NM= $(NM_FOR_TARGET)
AR= $(AR_FOR_TARGET)
SIZE= $(SIZE_FOR_TARGET)
OBJCOPY= $(OBJCOPY_FOR_TARGET)

export CC
export CXX
export AS
export LD
export NM
export AR
export SIZE
export OBJCOPY

RTEMS_ROOT = $(prefix)
PROJECT_ROOT = $(RTEMS_ROOT)
RTEMS_CUSTOM = $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).cfg
RTEMS_SHARE = $(RTEMS_ROOT)/share/rtems$(RTEMS_API)

RTEMS_USE_OWN_PDIR = no
RTEMS_HAS_POSIX_API = yes
RTEMS_HAS_ITRON_API = no
RTEMS_HAS_CPLUSPLUS = no

export RTEMS_BSP
export RTEMS_CUSTOM
export PROJECT_ROOT

