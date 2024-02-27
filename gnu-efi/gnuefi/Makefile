SRCDIR = .

VPATH = $(SRCDIR)

include $(SRCDIR)/../Make.defaults

TOPDIR = $(SRCDIR)/..

CDIR=$(TOPDIR)/..
FILES	= reloc_$(ARCH)  

OBJS	= $(FILES:%=%.o)

# on aarch64, avoid jump tables before all relocations have been processed
reloc_aarch64.o: CFLAGS += -fno-jump-tables

TARGETS	= crt0-efi-$(ARCH).o libgnuefi.a

all:	$(TARGETS)

libgnuefi.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^


clean:
	rm -f $(TARGETS) *~ *.o $(OBJS)

install:
	mkdir -p $(INSTALLROOT)$(LIBDIR)
	$(INSTALL) -m 644 $(TARGETS) $(INSTALLROOT)$(LIBDIR)
ifneq (,$(findstring FreeBSD,$(OS)))
 ifeq ($(ARCH),x86_64)
	$(INSTALL) -m 644 $(SRCDIR)/elf_$(ARCH)_fbsd_efi.lds $(INSTALLROOT)$(LIBDIR)
 else
	$(INSTALL) -m 644 $(SRCDIR)/elf_$(ARCH)_efi.lds $(INSTALLROOT)$(LIBDIR)
 endif
else
	$(INSTALL) -m 644 $(SRCDIR)/elf_$(ARCH)_efi.lds $(INSTALLROOT)$(LIBDIR)
endif

include $(SRCDIR)/../Make.rules
