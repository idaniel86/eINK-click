# List of all the eINK-click related files.
EINKCLICKSRCPP = eINK-click/ssd16xx/ssd16xx.cpp \
                 eINK-click/epd.cpp

# Required include directories
EINKCLICKINC = eINK-click \
               eINK-click/ssd16xx \
               eINK-click/fonts

# Shared variables
ALLCPPSRC += $(EINKCLICKSRCPP)
ALLINC    += $(EINKCLICKINC)
