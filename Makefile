JOBS := $(shell nproc)

all: server client

client:
	make -C build -j$(JOBS)

client-run: client
	./build/froullete

server:
	cmake --build build --target froullete-server

server-run: server
	./build/src/Server/froullete-server
