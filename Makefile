JOBS := $(shell nproc)

native: client server

all: client server web

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

server-gdb: server
	gdb ./build/src/Server/froullete-server

web-config: ./webbuild
	source ./web/emsdk/emsdk_env.sh && \
	emcmake cmake -S . -B webbuild -DEMSCRIPTEN=1

web: web-config
	source ./web/emsdk/emsdk_env.sh && \
	cmake --build webbuild -j$(JOBS)

web-run: web
	source ./web/emsdk/emsdk_env.sh && \
	emrun --no_browser --port 8080 webbuild/

web-android: web
	cp webbuild/froullete.{data,wasm,js} /home/goad/Documents/dev/web-static
