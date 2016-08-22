# Xi18n_sample
samples := Xi18n_test

all: IMdkit samples

IMdkit:
	$(MAKE) -C IMdkit

samples:
	@for dir in $(SAMPLES); do \
	$(MAKE) -C $$dir; \
	done

clean:
#	@find  Ximd -name "*.[ah]" | xargs -I{} rm {}
#	@find IMdkit -name "*.[oadi]" | xargs -I{} rm {}
#	@find Ximp_sample -name sampleIM | xargs -I{} rm {}
	$(MAKE) -C IMdkit clean

#$(SAMPLES):
#	$(MAKE) -C $@

.PHONY: all IMdkit samples clean
