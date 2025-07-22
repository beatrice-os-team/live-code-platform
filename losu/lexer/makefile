lexer.js:
	emcc\
		losu/**/*.c losu/**/**/*.c main.c\
		-I include -O3 -o ../../www/assets/wasm/lexer.m.js\
		-s WASM=1 -s SINGLE_FILE=1  -s TOTAL_MEMORY=512MB -s WASM_MEM_MAX=2048MB -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=1 -s MODULARIZE=1 -s EXPORT_NAME="LosuLiveCode"\
		-s EXPORTED_RUNTIME_METHODS="['ccall']"

	