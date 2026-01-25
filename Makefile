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

webm-config: ./webmbuild
	source ./web/emsdk/emsdk_env.sh && \
	emcmake cmake -S . -B webmbuild -DEMSCRIPTEN=1 -DMOBILE=ON

webm: webm-config
	source ./web/emsdk/emsdk_env.sh && \
	cmake --build webmbuild -j$(JOBS)

webm-run: webm
	source ./web/emsdk/emsdk_env.sh && \
	emrun --no_browser --port 8080 webmbuild/

webm-build-apk: webm
	cp webmbuild/froullete.{data,wasm,js} ./androidproject/app/src/main/assets/
	cd androidproject/ && ./gradlew assembleDebug
