all: test.dsk

test.bin: test.c ../demo/stream.c
	cl65 -o $@ -t apple2enh -m $(basename $@).map $^

test.dsk: test.bin
	cp prodos.dsk $@
	java -jar $(AC) -p    $@ test.system sys < $(shell cl65 --print-target-path)/apple2/util/loader.system
	java -jar $(AC) -cc65 $@ test        bin < test.bin

clean:
	-rm -f *.o *.map *.bin test.dsk
