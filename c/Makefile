OBJDIR := out
OUTOBJS := $(addprefix $(OBJDIR)/, meter.o fft.o voice.o shape.o note.o pq.o parser.o format.o rand.o)

WOBJDIR := wout
WOUTOBJS := $(addprefix $(WOBJDIR)/, meter.o fft.o voice.o shape.o note.o pq.o parser.o format.o rand.o)

FLAGS := -O3 -g -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wunused-parameter -Wunused-but-set-parameter -ferror-limit=100 -Wall -Wextra

out/%.o: src/%.c
	gcc $(FLAGS) -c -Iinclude $< -o $@

wout/%.o: src/%.c
	emcc -c -Iinclude $< -o $@

$(OUTOBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(WOUTOBJS): | $(WOBJDIR)

$(WOBJDIR):
	mkdir $(WOBJDIR)

clean:
	rm -rf out wout


# test relies on out
test: clean $(OUTOBJS)
	gcc $(FLAGS) -I include/ tests/main.c $(OUTOBJS) -o out/test && ./out/test

main: clean $(OUTOBJS)
	gcc $(FLAGS) -I include/ src/main.c $(OUTOBJS) -o ./bluerhapsody && ./bluerhapsody

debug: clean $(OUTOBJS)
	gcc $(FLAGS) -I include/ src/main.c $(OUTOBJS) -o ./bluerhapsody && ./bluerhapsody

wasm: clean $(WOUTOBJS)
	emcc -g -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wunused-parameter -Wunused-but-set-parameter -ferror-limit=100 -Iinclude src/wasm.c $(WOUTOBJS) -o bluerhapsody.mjs -sEXPORTED_FUNCTIONS=_wasm_get_channels,_wasm_spectra,_wasm_fft,_malloc,_free -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,HEAPU8,HEAPU32,HEAPF64,HEAPU64 -sALLOW_MEMORY_GROWTH -sMODULARIZE -sEXPORT_NAME='createModule' -sENVIRONMENT='web' -sSINGLE_FILE=1 && mv bluerhapsody.mjs ../react/wasm

