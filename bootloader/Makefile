#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# SPECS is the directory containing the important build and link files
#---------------------------------------------------------------------------------
export TARGET		:=	$(shell basename $(CURDIR))
BUILD		:=	build
SOURCES		:=	source ../libs/fatfs source/inih ../libs/i2c ../libs/fatfs/sdmmc
DATA		:=	data
INCLUDES	:=	include ../libs/fatfs source/inih ../libs/i2c ../libs/fatfs/sdmmc

#---------------------------------------------------------------------------------
# Setup some defines
#---------------------------------------------------------------------------------
VERSION		:=	$(shell git describe --abbrev=4 --always --dirty --tags || echo "UNKNOWN")
#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb -mthumb-interwork 

 
CFLAGS	:=	-g -Wall -O2\
			-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
			-ffast-math -std=c99\
			$(ARCH) -DVERSION_STRING="\"$(VERSION)\""

CFLAGS	+=	$(INCLUDE) -DARM9
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions 


ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-nostartfiles -g --specs=../stub.specs $(ARCH) -Wl,-Map,$(TARGET).map

LIBS	:= 

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=
  
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
 
export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:= $(addsuffix .o,$(BINFILES)) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all bootloaderLoader
 
#---------------------------------------------------------------------------------
all: $(BUILD) bootloaderLoader



$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	mv $(OUTPUT).bin arm9bootloader.bin

bootloaderLoader:
	@echo make BOOTLOADERLOADER
	@cd bootloaderloader && make
	@cp bootloaderloader/arm9loaderhax.bin arm9loaderhax.bin
	@echo BOOTLOADERLOADER done!
 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@cd bootloaderloader && make clean
	@rm -fr $(BUILD) $(OUTPUT).elf arm9bootloader.bin arm9loaderhax.bin
 
 
#---------------------------------------------------------------------------------
else
 
DEPENDS	:=	$(OFILES:.o=.d)
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).bin	:	$(OUTPUT).elf
$(OUTPUT).elf	:	$(OFILES)


#---------------------------------------------------------------------------------
%.bin: %.elf
	@$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)
 
#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
	
-include $(DEPENDS)

 
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
