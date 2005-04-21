include Makefile.setup

include Input/Makefile
include Audio/Makefile
include Graphics/Makefile
include Core/Physics/Makefile
include Core/Mixers/Makefile
include Core/Model/Makefile

LINKAGE += main.o dynload.o logger.o maths.o config.o

all: rcfs.$(OSTYPE) 

rcfs.$(OSTYPE): $(LINKAGE)
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
