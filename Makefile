BUILDDIR:=target
CMAKE:=cmake
CMAKEFLAGS?=

# perform a CMake out-of-source build
# (or configure IDE to do the same)
.PHONY:
build:
	mkdir -p $(BUILDDIR)
	$(CMAKE) $(CMAKEFLAGS) -B$(BUILDDIR) -S.
	$(MAKE) -C$(BUILDDIR)

# target-specific variable values: https://stackoverflow.com/a/1080180/2397327
debug: CMAKEFLAGS += -DCMAKE_BUILD_TYPE=Debug
debug: build

run:
	$(BUILDDIR)/compiler

.PHONY:
clean:
	rm -rf $(BUILDDIR)
