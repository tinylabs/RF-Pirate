#
#             LUFA Library
#     Copyright (C) Dean Camera, 2010.
#
#  dean [at] fourwalledcubicle [dot] com
#      www.fourwalledcubicle.com
#

# Makefile to build the LUFA library, projects and demos.

# Call with "make all" to rebuild everything, "make clean" to clean everything,
# "make clean_list" to remove all intermediatary files but preserve any binaries,
# "make doxygen" to document everything with Doxygen (if installed) and
# "make clean_doxygen" to remove generated Doxygen documentation from everything.


all: start bootloader bsp end

start:
	@echo Executing make on all RF Pirate framework.
	@echo	

# Must clean LUFA before compile since it gets compiled with different flags
bootloader:
	make -C lufa-lib/LUFA clean 
	make -C bootloader all -s

# Must clean LUFA before compile since it gets compiled with different flags
bsp:
	make -C lufa-lib/LUFA clean 
	make -C bsp all -s

end:
	@echo
	@echo make operation complete.

clean:
	make -C bootloader clean
	make -C lufa-lib/LUFA clean 
	make -C bsp clean

.PHONY : all start bootloader bsp end clean


