JOBS := $(shell nproc)

all: client server

client:
	cmake --build build -j$(JOBS)

client-run: client
	./build/froullete

client-dbg: client
	gf2 ./build/froullete

server:
	cmake --build build --target froullete-server -j$(JOBS)

server-run: server
	./build/src/Server/froullete-server

server-dbg: server
	gf2 ./build/src/Server/froullete-server
