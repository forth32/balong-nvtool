CC       = gcc
BUILDNO=$(shell cat buildno)
CFLAGS   = -g -O2  -Wunused -Wno-unused-result -D BUILDNO=$(BUILDNO) $(LIBS) 
TARGET = balong-nvtool nvdload-split nvdload-combine
INCREASE_BUILDNO ?= 0

.PHONY: all clean

ifeq ($(INCREASE_BUILDNO), 0)
all:    $(TARGET)
else
all:	$(TARGET) buildno
endif

clean: 
	rm *.o
	rm $(TARGET)

balong-nvtool: balong-nvtool.o nvio.o nvid.o sha2.o nvcrc.o
	@gcc $^ -o $@ $(LIBS) 
	@echo Current buid: $(BUILDNO)

nvdload-split: nvdload-split.o
	@gcc $^ -o $@ $(LIBS) 

nvdload-combine: nvdload-combine.o
	@gcc $^ -o $@ $(LIBS) 

buildno: *.c *.h
	@echo $$(($(BUILDNO) + 1)) >$(@)
