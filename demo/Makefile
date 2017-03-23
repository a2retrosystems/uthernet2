all: demo.dsk

%.bin: %.s %_main.c
	cl65 -o $@ -t apple2enh -m $(basename $@).map $^

stream.bin: stream.c stream_main.c
	cl65 -o $@ -t apple2enh -m $(basename $@).map $^

shared.bin: udp.s stream.c shared.c
	cl65 -o $@ -t apple2enh -m $(basename $@).map $^

demo.dsk: udp.bin tcp.bin stream.bin shared.bin
	cp prodos.dsk $@
	java -jar $(AC) -cc65 $@ udp    bin < udp.bin
	java -jar $(AC) -cc65 $@ tcp    bin < tcp.bin
	java -jar $(AC) -cc65 $@ stream bin < stream.bin
	java -jar $(AC) -cc65 $@ shared bin < shared.bin

%.exe: %.c
	cl /Fe:$@ $^

clean:
	-rm -f *.o *.map *.bin demo.dsk *.obj *.exe
