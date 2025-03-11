SHELL := /bin/bash

CXXFLAGS = -Werror -Wall -Wextra -pedantic -std=c++23

.PHONY: all clean format

all: twsfwphysx.wasm run_all_tests format

clean:
	rm -rf twsfwphysx.wasm twsfwphysx_world_state_generated.h run_all_tests

format: binding.cpp tests.cxx
	clang-format -i $^

emsdk/.emscripten:
	cd emsdk && \
	./emsdk install latest && \
	./emsdk activate latest

flatbuffers/build/flatc:
	mkdir -p flatbuffers/build && \
	cd flatbuffers/build && \
	cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release .. && \
	make -j flatc

twsfwphysx_world_state_generated.h: ../twsfwphysx_world_state.fbs flatbuffers/build/flatc
	./flatbuffers/build/flatc --cpp ../twsfwphysx_world_state.fbs

twsfwphysx.wasm: binding.cpp twsfwphysx_world_state_generated.h emsdk/.emscripten
	source emsdk/emsdk_env.sh && \
	emcc binding.cpp -O3 $(CXXFLAGS) -I../include -Iflatbuffers/include --no-entry -o $@

run_all_tests: tests.cxx binding.cpp twsfwphysx_world_state_generated.h
	$(CXX) -O0 -g $(CXXFLAGS) -I../include -Iflatbuffers/include -fsanitize=address,undefined tests.cxx binding.cpp -o $@
