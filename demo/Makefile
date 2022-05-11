all: demo.dsk

%.bin: %.s %_main.c
	cl65 -o $@ -t apple2enh -m $(basename $@).map $^

stream.bin: stream.c stream_main.c
	cl65 -o $@ -t apple2enh -m $(basename $@).map $^

stream.irq.bin: stream.c stream_main.c
	cl65 -o $@ -t apple2enh -m $(basename $@).map -D SOCKET_IRQ $^

demo.dsk: udp.bin tcp.bin stream.bin stream.irq.bin
	cp prodos.dsk $@
	java -jar $(AC) -as $@ udp        bin < udp.bin
	java -jar $(AC) -as $@ tcp        bin < tcp.bin
	java -jar $(AC) -as $@ stream     bin < stream.bin
	java -jar $(AC) -as $@ stream.irq bin < stream.irq.bin

%.exe: %.c
	cl /Fe:$@ $^

peer: peer.c
	gcc -o $@ $^

clean:
	-rm -f *.o *.map *.bin demo.dsk *.obj *.exe peer
