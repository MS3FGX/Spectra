# App info
APPNAME = spectra
VERSION = 1.2

# Compiler and options
CC = gcc
CFLAGS += -Wall -O2

# Libraries to link
LIBS = -lgd -lpng -lm

# Files
SOURCES = spectra.c
DOCS = ChangeLog COPYING README examples/

# Targets
# Build
$(APPNAME): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) $(LIBS) -o $(APPNAME)

# Build tarball
release: clean
	tar --exclude='.git' -C ../ -czvf /tmp/$(APPNAME)-$(VERSION).tar.gz $(APPNAME)-$(VERSION)

# Clean for dist
clean:
	rm -rf $(APPNAME) *.o *.txt *.log *.png 

# Install to system
install: $(APPNAME)
	mkdir -p $(DESTDIR)/usr/bin/
	mkdir -p $(DESTDIR)/usr/share/doc/$(APPNAME)-$(VERSION)/
	cp $(APPNAME) $(DESTDIR)/usr/bin/
	cp -a $(DOCS) $(DESTDIR)/usr/share/doc/$(APPNAME)-$(VERSION)/
	
# Upgrade from previous source install
upgrade: removeold install

# Remove current version from system
uninstall:
	rm -rf $(DESTDIR)/usr/share/doc/$(APPNAME)-$(VERSION)/
	rm -f $(DESTDIR)/usr/bin/$(APPNAME)

# Remove older versions
removeold:
	rm -rf $(DESTDIR)/usr/share/doc/$(APPNAME)*
	rm -f $(DESTDIR)/usr/bin/$(APPNAME)	
