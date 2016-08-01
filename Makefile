.PHONY : all hax bootloader stage2_update firm0 firm1 sector arm11bg stage2 installer clean
TARGET		=	arm9loaderhax
PYTHON 		=	python
INDIR		=	data_input
OUTDIR		=	data_output
PACKTOOL	=	common/pack_tool

all : $(OUTDIR) hax bootloader installer

hax : $(OUTDIR) firm0 firm1 sector arm11bg stage2 package
	@cd payload_installer/installer && make TARGET=../../$(OUTDIR)/$(TARGET)

bootloader : $(OUTDIR) arm11bg arm9bootloader

stage2_update: $(OUTDIR) $(PACKTOOL) stage2 
	@$(PACKTOOL) null null null $(OUTDIR)/stage0x5C000.bin
	@dd if=payload_stage2/label.txt of=arm9loaderhax.pack bs=32 seek=1 conv=notrunc
	@mv arm9loaderhax.pack $(OUTDIR)/arm9loaderhax.pack
	@rm -f $(OUTDIR)/stage0x5C000.bin

$(OUTDIR):
	@[ -d $(OUTDIR) ] || mkdir -p $(OUTDIR)

firm0:
	@cd payload_stage1 && make
	@cp $(INDIR)/new3ds90.firm $(OUTDIR)/firm0.bin
	@dd if=payload_stage1/payload_stage1.bin of=$(OUTDIR)/firm0.bin bs=1 seek=984464 conv=notrunc

firm1:
	@cp $(INDIR)/new3ds10.firm $(OUTDIR)/firm1.bin

sector:
	@$(PYTHON) common/sector_generator.py $(INDIR)/secret_sector.bin $(INDIR)/otp.bin $(OUTDIR)/sector.bin


arm11bg:
	@[ -d payload_stage2/data ] || mkdir -p payload_stage2/data
	$(MAKE) -C arm11bg
	@cp arm11bg/arm11bg.bin payload_stage2/data/

stage2: arm11bg
	@cp arm11bg/arm11bg.bin payload_stage2/data
	@$(MAKE) -C payload_stage2
	@cp payload_stage2/payload_stage2.bin $(OUTDIR)/stage0x5C000.bin
	
package: $(PACKTOOL)
	@$(PACKTOOL) $(OUTDIR)/firm0.bin $(OUTDIR)/firm1.bin $(OUTDIR)/sector.bin $(OUTDIR)/stage0x5C000.bin
	@mv arm9loaderhax.pack $(OUTDIR)/arm9loaderhax.pack
	@rm -f $(OUTDIR)/firm0.bin $(OUTDIR)/firm1.bin $(OUTDIR)/sector.bin $(OUTDIR)/stage0x5C000.bin
	
$(PACKTOOL):
	@echo Building pack_tool...
	@cd $(PACKTOOL)_src && make

arm9bootloader :
	@echo make BOOTLOADER
	@[ -d bootloader/data ] || mkdir -p bootloader/data
	@cp arm11bg/arm11bg.bin bootloader/data/
	@$(MAKE) -C bootloader
	@cp bootloader/arm9bootloader.bin $(OUTDIR)/arm9bootloader.bin
	@echo BOOTLOADER done!

installer:
	@mkdir -p payload_installer/brahma2/data/
	@cd payload_installer && make TARGET=../$(OUTDIR)/$(TARGET)

clean:
	@echo clean...
	@$(MAKE) -C payload_stage1 clean
	@$(MAKE) -C arm11bg clean
	@$(MAKE) -C payload_stage2 clean
	@$(MAKE) -C payload_installer clean TARGET=../$(TARGET)
	@$(MAKE) -C bootloader clean
	rm -rf data_output
