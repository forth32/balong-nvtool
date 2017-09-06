CC       = gcc
BUILDNO=$(shell cat build)
CFLAGS   = -g -O2  -Wunused -Wno-unused-result -D BUILDNO=$(BUILDNO) $(LIBS) 
TARGET = balong-nvtool nvdload-split nvdload-combine

.PHONY: all clean

all:    $(TARGET)

clean: 
	rm *.o
	rm $(TARGET)

balong-nvtool: balong-nvtool.o nvio.o nvid.o sha2.o nvcrc.o
	@gcc $^ -o $@ $(LIBS) 
	@echo Current buid: $(BUILDNO)
	@echo $$((`cat build`+1)) >build

nvdload-split: nvdload-split.o
	@gcc $^ -o $@ $(LIBS) 

nvdload-combine: nvdload-combine.o
	@gcc $^ -o $@ $(LIBS) 
