.PHONY : all bootloader arm11bg arm11stub clean
TARGET		=	BootCTR9
PYTHON 		=	python
INDIR		=	data_input
OUTDIR		=	data_output
PACKTOOL	=	common/pack_tool

all : $(OUTDIR) bootloader


bootloader : $(OUTDIR) arm11bg arm11stub arm9bootloader

$(OUTDIR):
	@[ -d $(OUTDIR) ] || mkdir -p $(OUTDIR)

arm11bg:
	@[ -d payload_stage2/data ] || mkdir -p payload_stage2/data
	$(MAKE) -C arm11bg
	@cp arm11bg/arm11bg.bin payload_stage2/data/
arm11stub:
	@[ -d payload_stage2/data ] || mkdir -p payload_stage2/data
	$(MAKE) -C arm11stub
	@cp arm11stub/arm11stub.bin payload_stage2/data/

arm9bootloader :
	@echo make BOOTLOADER
	@[ -d bootloader/data ] || mkdir -p bootloader/data
	@cp arm11bg/arm11bg.bin bootloader/data/
	@cp arm11stub/arm11stub.bin bootloader/data/
	@$(MAKE) -C bootloader
	@cp bootloader/arm9bootloader.bin $(OUTDIR)/arm9bootloader.bin
	@echo BOOTLOADER done!

clean:
	@echo clean...
	@$(MAKE) -C arm11bg clean
	@$(MAKE) -C arm11stub clean
	@$(MAKE) -C bootloader clean
	rm -rf data_output
