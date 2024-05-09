COMPILER=g++-14
PLUGINS_PATH=$(shell $(COMPILER) -print-file-name=plugin)
GMP_PATH=/opt/homebrew/Cellar/gmp/6.3.0
INPUT_DIRS=-I${PLUGINS_PATH}/include -I${GMP_PATH}/include
# На маке вместо -shared работает с этими флагами
WTFLAGS=-dynamiclib -undefined dynamic_lookup
CXX_ARGS=-g -Wall -Wextra -std=c++14 ${INPUT_DIRS} -fPIC -fno-rtti ${WTFLAGS}

phi-debug:
	mkdir -p plugin
	${COMPILER} ${CXX_ARGS} src/phi-debug/phi-debug.cpp -o plugin/$@.so

test:
	${COMPILER} -fplugin=plugin/phi-debug.so -O0 src/test/test.c -o src/test/test
	rm src/test/test

.PHONY : clean

clean:
	rm -rf plugin src/test/test
