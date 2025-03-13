SHELL := /bin/bash

CXXFLAGS = -Werror -Wall -Wextra -pedantic -std=c++23

.PHONY: all clean format

all: build/twsfwphysx.wasm build/run_all_tests format

clean:
	rm -rf build/ 

format: source/binding.cpp source/tests.cxx
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

build/twsfwphysx_world_state_generated.h: ../twsfwphysx_world_state.fbs flatbuffers/build/flatc
	./flatbuffers/build/flatc --cpp -o build/ ../twsfwphysx_world_state.fbs

build/twsfwphysx.wasm: source/binding.cpp build/twsfwphysx_world_state_generated.h emsdk/.emscripten
	source emsdk/emsdk_env.sh && \
	emcc source/binding.cpp -O3 $(CXXFLAGS) -Ibuild/ -I../include -Iflatbuffers/include --no-entry -o $@

build/run_all_tests: source/tests.cxx source/binding.cpp build/twsfwphysx_world_state_generated.h
	$(CXX) -O0 -g $(CXXFLAGS) -Ibuild/ -I../include -Iflatbuffers/include -fsanitize=address,undefined source/tests.cxx source/binding.cpp -o $@
