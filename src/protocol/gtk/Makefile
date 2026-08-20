empty :=
space := $(empty) $(empty)
.RECIPEPREFIX := $(space)

-include ../../config.mak

GTK_IM_MODULE=im-alpha.so
GTK4_IM_MODULE=libim-alpha.so
SRCS=gaimimcontext.c

GTK_MAJORS := 4 3

define gtk_pkg_for
$(strip $(if $(filter 4,$1),gtk4,$(if $(filter 3,$1),gtk+-3.0,$(if $(filter 2,$1),gtk+-2.0,))))
endef

define gtk_pc_for
$(strip $(if $(filter 2,$1),gtk+-2.0 gio-2.0,$(call gtk_pkg_for,$1)))
endef

define gtk_dir_for
$(strip $(if $(filter 4,$1),gtk-4.0,$(if $(filter 3,$1),gtk-3.0,$(if $(filter 2,$1),gtk-2.0,))))
endef

define gtk_has_version
$(shell pkg-config --exists $(call gtk_pc_for,$1) >/dev/null 2>&1 && echo yes || echo no)
endef

define gtk_version
$(shell pkg-config --modversion $(call gtk_pkg_for,$1) 2>/dev/null)
endef

define gtk_cflags
$(shell pkg-config --cflags $(call gtk_pc_for,$1) 2>/dev/null)
endef

define gtk_libs
$(shell pkg-config --libs $(call gtk_pc_for,$1) 2>/dev/null)
endef

define gtk_module_dir
$(shell pkg-config --variable=libdir $(call gtk_pkg_for,$1) 2>/dev/null)/$(call gtk_dir_for,$1)/$(shell pkg-config --variable=gtk_binary_version $(call gtk_pkg_for,$1) 2>/dev/null)/immodules
endef

GTK_BUILDS :=
GTK_INSTALL_VERSIONS :=

$(foreach major,$(GTK_MAJORS),\
  $(if $(filter yes,$(call gtk_has_version,$(major))),\
    $(eval GTK_BUILDS += build/gtk$(major)/$(if $(filter 4,$(major)),$(GTK4_IM_MODULE),$(GTK_IM_MODULE)))\
    $(eval GTK_INSTALL_VERSIONS += $(major))\
  )\
)

.SECONDARY: build/gtk2/gaimimcontext.o build/gtk3/gaimimcontext.o build/gtk4/gaimimcontext.o

a11: $(GTK_BUILDS)

build/gtk3 build/gtk4:
    @mkdir -p $@

build/gtk%/%.o: %.c | build/gtk%
    $(CC) $(call gtk_cflags,$*) -I../../.. -fPIC -MMD -c $< -o $@

build/gtk%/$(GTK_IM_MODULE): build/gtk%/gaimimcontext.o
    $(CC) $^ $(call gtk_libs,$*) -shared -o $@

build/gtk4/$(GTK4_IM_MODULE): build/gtk4/gaimimcontext.o
    $(CC) $^ $(call gtk_libs,4) -shared -o $@

build/gtk%/gaimimcontext.o: gaimimcontext.c | build/gtk%
    $(CC) $(call gtk_cflags,$*) -I../../.. -fPIC -MMD -c $< -o $@

-include $(shell find build -name '*.d' -type f 2>/dev/null)

install: a11
    @for major in $(GTK_INSTALL_VERSIONS); do \
        case $$major in \
            4) pkg=gtk4; dir=gtk-4.0 ;; \
            3) pkg=gtk+-3.0; dir=gtk-3.0 ;; \
            2) pkg=gtk+-2.0; dir=gtk-2.0 ;; \
        esac; \
        module_dir="$$(pkg-config --variable=libdir $$pkg)/$$dir/$$(pkg-config --variable=gtk_binary_version $$pkg)/immodules"; \
        $(INSTALL) -d "$(DESTDIR)$$module_dir"; \
        module="$(GTK_IM_MODULE)"; \
        test "$$major" = 4 && module="$(GTK4_IM_MODULE)"; \
        $(INSTALL) -m 0644 "build/gtk$$major/$$module" "$(DESTDIR)$$module_dir/$$module"; \
        test "$$major" = 4 && gio-querymodules "$(DESTDIR)$$module_dir" || true; \
    done

uninstall:
    @for major in $(GTK_INSTALL_VERSIONS); do \
        case $$major in \
            4) pkg=gtk4; dir=gtk-4.0 ;; \
            3) pkg=gtk+-3.0; dir=gtk-3.0 ;; \
        esac; \
        module_dir="$$(pkg-config --variable=libdir $$pkg)/$$dir/$$(pkg-config --variable=gtk_binary_version $$pkg)/immodules"; \
        rm -f "$(DESTDIR)$$module_dir/$(GTK_IM_MODULE)"; \
    done

clean:
    -@rm -rf build
    -@rm -f $(GTK_IM_MODULE) *.o *.d

distclean: clean

.PHONY: a11 install uninstall clean distclean
