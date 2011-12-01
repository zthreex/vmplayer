# FRIBIDI
FRIBIDI_VERSION := 0.19.2
FRIBIDI_URL := http://fribidi.org/download/fribidi-$(FRIBIDI_VERSION).tar.gz

PKGS += fribidi
ifeq ($(call need_pkg,"fribidi"),)
PKGS_FOUND += fribidi
endif

$(TARBALLS)/fribidi-$(FRIBIDI_VERSION).tar.gz:
	$(call download,$(FRIBIDI_URL))

.sum-fribidi: fribidi-$(FRIBIDI_VERSION).tar.gz

fribidi: fribidi-$(FRIBIDI_VERSION).tar.gz .sum-fribidi
	$(UNPACK)
	$(APPLY) $(SRC)/fribidi/fribidi.patch
	$(MOVE)

# FIXME: DEPS_fribidi = iconv $(DEPS_iconv)
.fribidi: fribidi
	cd $< && rm -f configure && ./bootstrap
	cd $< && $(HOSTVARS) ./configure $(HOSTCONF)
	cd $< && $(MAKE) install
	touch $@
