out_dir = build

.PHONY: all
all:
	mkdir -p $(out_dir)
	cmake -B$(out_dir) -H.
	$(MAKE) -C $(out_dir) -s
	ln -sf $(out_dir)/dcc dcc

.PHONY: clean
clean:
	rm -rf $(out_dir)
	rm -f dcc
