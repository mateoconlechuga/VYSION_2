# Exported from https://tiplanet.org/pb/ on Sun Apr 25 04:29:50 2021 (CEST)

# ----------------------------
# Program Options
# ----------------------------

NAME         ?= VYSION2
ICON         ?= icon.png
DESCRIPTION  ?= "VYSION 2 Experimental"
COMPRESSED   ?= NO
ARCHIVED     ?= YES
HAS_SPRINTF  ?= NO

# ----------------------------
# Compile Options
# ----------------------------

CFLAGS   ?= -Oz -W -Wall -Wextra -Wwrite-strings
CXXFLAGS ?= -Oz -W -Wall -Wextra -Wwrite-strings

# ----------------------------
# Debug Options
# ----------------------------

# ----------------------------

ifndef CEDEV
$(error CEDEV environment path variable is not set)
endif

include $(CEDEV)/meta/makefile.mk
