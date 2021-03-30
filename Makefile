BUILDDIR:=target
CMAKE:=cmake

# perform a CMake out-of-source build
# (or configure IDE to do the same)
.PHONY:
build:
	mkdir -p $(BUILDDIR)
	$(CMAKE) -B$(BUILDDIR) -S.
	$(MAKE) -C$(BUILDDIR)

.PHONY:
run:
	$(BUILDDIR)/compiler

.PHONY:
clean:
	rm -rf $(BUILDDIR)
