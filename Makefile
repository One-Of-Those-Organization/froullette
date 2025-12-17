JOBS := $(shell nproc)

all:
	make -C build -j$(JOBS)

run: all
	./build/froullete

server: src/Server/Server.cpp
	cmake --build build --target froullete-server

server-run: server
	./build/src/Server/froullete-server
