CC = gcc
AS = sh-elf-as
OBJCOPY = sh-elf-objcopy
CFLAGS = -Wall -c

COMPILER_OBJS = spc.o opcodes.o
COMPILER_BIN = spc.exe

all:		bin2c.exe spc.exe sh4_files #pixels.dis

check_syntax:	pixels.o

#====================================================================

bin2c.exe:	bin2c.c
		$(CC) -Wall -o $@ $<

$(COMPILER_BIN):$(COMPILER_OBJS)
		$(CC) -o $@ $(COMPILER_OBJS)

spc.o:		spc.c opcodes.h shared/sample_data.h shared/bitmap.h \
		shared/types.h
		$(CC) $(CFLAGS) -o $@ spc.c

opcodes.o:	opcodes.c opcodes.h shared/types.h
		$(CC) $(CFLAGS) -o $@ opcodes.c


sh4_files:	$(COMPILER_BIN)
		./$(COMPILER_BIN)
		./bin2c Code__bl_g_1.sh4 Code__bl_g_1.h >/dev/null
		./bin2c Code__bl_g_14.sh4 Code__bl_g_14.h >/dev/null
		./bin2c Code__hopper_0.sh4 Code__hopper_0.h >/dev/null
		./bin2c Code__lava_plant.sh4 Code__lava_plant.h >/dev/null

#-----------------

pixels.s:	$(COMPILER_BIN)
		./$(COMPILER_BIN) -v >pixels.s

pixels.o:	pixels.s
		$(AS) -little -o $@ $<

pixels.bin:	pixels.o
		$(OBJCOPY) -O binary $< $@

pixels.dis:	pixels.bin
		dcdis $< >$@

#-----------------

clean:
	rm -f *.bin *.o *.dis *.sh4 spc.exe
