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

PLUGIN_NAME=plugin
BUILD_PATH_TO_PLUGIN=build/plugin/${PLUGIN_NAME}.so
BUILD_PATH_TO_SBOX=build/sandbox/sbox
BUILD_PATH_TO_TESTS=build/test
PATH_TO_TESTS=src/test

OUT_PATH=build/out/funcs/text
IMGS_PATH=build/out/funcs/imgs

IMG_FORMAT=png

plugin: ${SRC} ${SRC_PLUGIN} ${HEADERS}
	mkdir -p build/plugin
	${COMPILER} ${CXX_PLUGIN_ARGS} ${SRC_PLUGIN} ${SRC} -o ${BUILD_PATH_TO_PLUGIN}

sbox: ${SRC} ${SRC_SBOX} ${HEADERS}
	mkdir -p build/sandbox
	${COMPILER} -Iinclude/ ${SRC} ${SRC_SBOX} -o ${BUILD_PATH_TO_SBOX}

TEST_CASE=1
run: plugin
	mkdir -p ${OUT_PATH}
	mkdir -p build/test
	${COMPILER} -fplugin=${BUILD_PATH_TO_PLUGIN} -fplugin-arg-${PLUGIN_NAME}-out_path=${OUT_PATH} -O0 ${PATH_TO_TESTS}/test${TEST_CASE}.c -o ${BUILD_PATH_TO_TESTS}/test
	rm ${BUILD_PATH_TO_TESTS}/test

print: run
	mkdir -p build/out/funcs/imgs
	for descr in $(shell ls ${OUT_PATH}) ; do \
    	dot -T${IMG_FORMAT} -Gdpi=300 -o ${IMGS_PATH}/$$descr.${IMG_FORMAT} ${OUT_PATH}/$$descr; \
	done

cprint: clean print

sandbox: sbox
	./${BUILD_PATH_TO_SBOX}

# говорим, что clean это не имя файла
.PHONY : clean

clean:
	rm -rf build
