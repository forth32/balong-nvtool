CC       = gcc
LIBS     = -lmd
BUILDNO=$(shell cat build)
CFLAGS   = -O2  -Wunused -Wno-unused-result -D BUILDNO=$(BUILDNO) $(LIBS) 

.PHONY: all clean

all:    balong-nvtool nvdload-split nvdload-combine

clean: 
	rm *.o
	rm balong-nvtool

balong-nvtool: balong-nvtool.o nvio.o nvid.o
	@gcc $^ -o $@ $(LIBS) 
	@echo Current buid: $(BUILDNO)
	@echo $$((`cat build`+1)) >build

nvdload-split: nvdload-split.o
	@gcc $^ -o $@ $(LIBS) 

nvdload-combine: nvdload-combine.o
	@gcc $^ -o $@ $(LIBS) 
