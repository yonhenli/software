# these are the two directories where source code can be located
IMPL   	= impl_src
SIM 	= sim_src

CC      = gcc
OPTIONS = -O2 -I$(IMPL) -I$(SIM) # Production options
#OPTIONS = -O0 -g -I$(IMPL) -I$(SIM) # Debugging Options
CFLAGS  = $(OPTIONS) -Wall -std=c89 -pedantic -pipe -Werror
SUBMIT  = $(IMPL) $(SIM) Makefile inputs 

IMPL_OBJS = page-fault.o \
               page-replacement.o \
               page-lookup.o \
               eval.o \
               tlb-lookup.o

SIM_OBJS = global.o \
                 memory.o \
                 pagetable.o \
                 process.o \
                 swapfile.o \
                 stats.o \
                 sim.o \
                 tlb.o

ALL_OBJS = $(IMPL_OBJS:%.o=$(IMPL)/%.o) \
           $(SIM_OBJS:%.o=$(SIM)/%.o)

ALL = vsim
all: $(ALL)

vsim: $(ALL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Use this target to collect all the files to submit
submit: clean
	tar zcvf submit.tar.gz $(SUBMIT)

.PHONY: clean
clean:
	rm -rf $(ALL) $(IMPL)/*.o $(SIM)/*.o core* $(IMPL)/*~

# everything below this line describes the dependencies that exist.
$(SIM)/global.o: $(SIM)/global.h $(SIM)/global.c
$(SIM)/memory.o: $(SIM)/memory.h $(SIM)/memory.c \
                       $(SIM)/tlb.h    $(SIM)/stats.h \
                       $(SIM)/useful.h $(SIM)/types.h $(IMPL)/macros.h
$(SIM)/pagetable.o: $(SIM)/pagetable.c $(SIM)/pagetable.h \
                          $(SIM)/types.h
$(SIM)/swapfile.o:    $(SIM)/swapfile.c    $(SIM)/swapfile.h \
                          $(SIM)/types.h         $(SIM)/global.h \
                          $(SIM)/useful.h        $(SIM)/stats.h
$(SIM)/process.o:   $(SIM)/process.c   $(SIM)/process.h \
                          $(SIM)/global.h    $(SIM)/useful.h \
                          $(SIM)/tlb.h       $(SIM)/pagetable.h \
                          $(SIM)/types.h
$(SIM)/sim.o:       $(SIM)/sim.c        $(SIM)/global.h \
                          $(SIM)/process.h    $(SIM)/memory.h \
                          $(SIM)/stats.h $(SIM)/swapfile.h \
                          $(SIM)/tlb.h        $(SIM)/useful.h \
                          $(SIM)/pagetable.h  $(SIM)/types.h
$(SIM)/stats.o: $(SIM)/stats.h $(SIM)/stats.c
$(SIM)/tlb.o: $(SIM)/tlb.c       $(SIM)/tlb.h \
                    $(SIM)/global.h    $(SIM)/useful.h \
                    $(SIM)/types.h $(IMPL)/macros.h
$(IMPL)/eval.o:  $(SIM)/stats.h $(IMPL)/eval.c
$(IMPL)/page-fault.o: $(SIM)/types.h    $(SIM)/process.h \
                         $(SIM)/global.h   $(SIM)/swapfile.h \
                         $(IMPL)/page-fault.c $(SIM)/pagetable.h \
			 $(IMPL)/macros.h
$(IMPL)/page-lookup.o: $(SIM)/pagetable.h $(IMPL)/page-lookup.c \
                          $(SIM)/swapfile.h    $(SIM)/stats.h \
                          $(SIM)/types.h $(IMPL)/macros.h
$(IMPL)/page-replacement.o: $(SIM)/types.h  $(SIM)/pagetable.h \
                               $(SIM)/global.h $(SIM)/process.h \
                               $(IMPL)/page-replacement.c $(IMPL)/macros.h
$(IMPL)/tlb-lookup.o: $(IMPL)/tlb-lookup.c  $(SIM)/tlb.h \
                         $(SIM)/pagetable.h $(SIM)/types.h $(IMPL)/macros.h

