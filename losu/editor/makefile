editor.js:
	emcc\
		losu/**/*.c losu/**/**/*.c main.c\
		-I include -O3 -o editor.m.js\
		-s WASM=1 -s SINGLE_FILE=1 -s NO_EXIT_RUNTIME=1 -s MODULARIZE=1 -s EXPORT_NAME="LosuLiveCode"\
		-s EXPORTED_RUNTIME_METHODS="['ccall']"

	