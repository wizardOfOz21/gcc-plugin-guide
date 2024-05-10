COMPILER=g++-14
PLUGINS_PATH=$(shell $(COMPILER) -print-file-name=plugin)
GMP_PATH=/opt/homebrew/Cellar/gmp/6.3.0
INPUT_DIRS=-Iinclude/ -I${PLUGINS_PATH}/include/ -I${PLUGINS_PATH}/include/c-family -I${GMP_PATH}/include
# На маке вместо -shared работает с этими флагами
WTFLAGS=-dynamiclib -undefined dynamic_lookup
CXX_PLUGIN_ARGS=-g -Wall -Wextra -std=c++14 ${INPUT_DIRS} -fPIC -fno-rtti ${WTFLAGS}

SRC=$(shell ls src/*.cpp)
SRC_PLUGIN=$(shell ls src/plugin/*.cpp)
SRC_SBOX=$(shell ls src/sandbox/*.cpp)

HEADERS=$(shell ls include/*.hpp)

BUILD_PATH_TO_PLUGIN=build/plugin/plugin.so
BUILD_PATH_TO_SBOX=build/sandbox/sbox
BUILD_PATH_TO_TESTS=build/test
PATH_TO_TESTS=src/test

plugin: ${SRC} ${SRC_PLUGIN} ${HEADERS}
	mkdir -p build/plugin
	${COMPILER} ${CXX_PLUGIN_ARGS} ${SRC_PLUGIN} ${SRC} -o ${BUILD_PATH_TO_PLUGIN}

sbox: ${SRC} ${SRC_SBOX} ${HEADERS}
	mkdir -p build/sandbox
	${COMPILER} -Iinclude/ ${SRC} ${SRC_SBOX} -o ${BUILD_PATH_TO_SBOX}

run: plugin
	mkdir -p build/test
	${COMPILER} -fplugin=${BUILD_PATH_TO_PLUGIN} -O0 ${PATH_TO_TESTS}/test.c -o ${BUILD_PATH_TO_TESTS}/test
	rm ${BUILD_PATH_TO_TESTS}/test

sandbox: sbox
	./${BUILD_PATH_TO_SBOX}

# говорим, что clean это не имя файла
.PHONY : clean

clean:
	rm -rf plugin src/test/test
