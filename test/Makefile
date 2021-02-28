all: test.dsk

test.bin: test.c ../demo/stream.c
	cl65 -o $@ -t apple2enh -m $(basename $@).map $^

test3.bin: test.c stream3.c
	cl65 -o $@ -t apple2 -m $(basename $@).map $^

test.dsk: test.bin test3.bin
	cp prodos.dsk $@
	java -jar $(AC) -p  $@ test.system  sys < $(shell cl65 --print-target-path)/apple2/util/loader.system
	java -jar $(AC) -as $@ test         bin < test.bin
	java -jar $(AC) -p  $@ test3.system sys < $(shell cl65 --print-target-path)/apple2/util/loader.system
	java -jar $(AC) -as $@ test3        bin < test3.bin

clean:
	-rm -f *.o *.map *.bin test.dsk
