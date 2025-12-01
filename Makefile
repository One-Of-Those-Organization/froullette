JOBS := $(shell nproc)

all:
	make -C build -j$(JOBS)

run: all
	./build/froullete
