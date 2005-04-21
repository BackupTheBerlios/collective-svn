include Makefile.setup

include input/Makefile
include graphics/Makefile
include core/physics/Makefile
include core/mixers/Makefile
include core/model/Makefile
include arch/Makefile

LINKAGE += main.o dynload.o logger.o maths.o config.o

all: ../dist/$(OSTYPE)/collective

../dist/$(OSTYPE)/collective: $(LINKAGE)
	@@echo "Linking   $@"
#	@$(LD) $(LDFLAGS) -o $@ $(LINKAGE)
	@$(LD) -o $@ $(LINKAGE) $(LDFLAGS) 

%.o:%.c
	@@echo "Compiling $<"
	@$(CC) $(CFLAGS) -Iinclude/ -c $< -o $@

ifeq ($(OSTYPE),IRIX)
%.o:%.so
	@@echo "Linking $@"
	@$(LD) $(LDFLAGS) -shared $<.o -soname $@ -o $@
endif

ifeq ($(OSTYPE),Linux)
%.o:%.so
	@@echo "Linking $@"
	@$(LD) $(LDFLAGS) -shared $<.o -soname $@ -o $@
endif

ifeq ($(OSTYPE),Darwin)
%.o:%.so
	@echo "Linking $@"
	@$(LD) $(SOFLAGS)  $<.o -o $@
endif

clean:
	@@find . -name "*.o" -exec rm {} \;
	@@find . -name "*.so" -exec rm {} \;
	@@find . -name "*~" -exec rm {} \;
	@@find . -name "so_locations" -exec rm {} \;
	@@rm -f rcfs.$(OSTYPE) nohup.out 
