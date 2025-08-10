#!/bin/bash

echo "构建Losu编译器演示模块..."

# 检查emcc是否可用
if ! command -v emcc &> /dev/null; then
    echo "错误: 未找到emcc编译器，请安装Emscripten SDK"
    exit 1
fi

# 记录当前目录
SCRIPT_DIR=$(pwd)

echo "1. 编译词法分析模块..."
cd losu/lexer
# 包含目录 (相对于当前编译目录)
INCLUDE_DIRS="-I ../include"
# 获取所有源文件，排除重复的.core.dtc.c文件
LOSU_SRC_FILES=$(find ../losu -name "*.c" | grep -v "\.core\.dtc\.c" | tr '\n' ' ')
emcc main.c ${LOSU_SRC_FILES} ${INCLUDE_DIRS} \
     -s WASM=1 -s MODULARIZE=1 -s SINGLE_FILE=1 -s EXPORT_NAME="LosuLexer" \
     -s EXPORTED_FUNCTIONS="['_lexer_demo', '_run', '_malloc', '_free']" \
     -s EXPORTED_RUNTIME_METHODS="['lengthBytesUTF8', 'stringToUTF8']" \
     -s ALLOW_MEMORY_GROWTH=1 \
     -o ../../www/assets/wasm/lexer.m.js
if [ $? -eq 0 ]; then
    echo "   ✓ 词法分析模块编译完成"
else
    echo "   ✗ 词法分析模块编译失败"
fi
cd "${SCRIPT_DIR}"

echo "2. 编译语法分析模块..."
cd losu/parser
INCLUDE_DIRS="-I ../include"
LOSU_SRC_FILES=$(find ../losu -name "*.c" | grep -v "\.core\.dtc\.c" | tr '\n' ' ')
emcc main.c ${LOSU_SRC_FILES} ${INCLUDE_DIRS} \
     -s WASM=1 -s MODULARIZE=1 -s SINGLE_FILE=1 -s EXPORT_NAME="LosuParser" \
     -s EXPORTED_FUNCTIONS="['_parser_demo', '_run', '_malloc', '_free']" \
     -s EXPORTED_RUNTIME_METHODS="['lengthBytesUTF8', 'stringToUTF8']" \
     -s ALLOW_MEMORY_GROWTH=1 \
     -o ../../www/assets/wasm/parser.m.js
if [ $? -eq 0 ]; then
    echo "   ✓ 语法分析模块编译完成"
else
    echo "   ✗ 语法分析模块编译失败"
fi
cd "${SCRIPT_DIR}"

echo "3. 编译语义分析模块..."
cd losu/sema
INCLUDE_DIRS="-I ../include"
LOSU_SRC_FILES=$(find ../losu -name "*.c" | grep -v "\.core\.dtc\.c" | tr '\n' ' ')
emcc main.c ${LOSU_SRC_FILES} ${INCLUDE_DIRS} \
     -s WASM=1 -s MODULARIZE=1 -s SINGLE_FILE=1 -s EXPORT_NAME="LosuSema" \
     -s EXPORTED_FUNCTIONS="['_sema_demo', '_run', '_malloc', '_free']" \
     -s EXPORTED_RUNTIME_METHODS="['lengthBytesUTF8', 'stringToUTF8']" \
     -s ALLOW_MEMORY_GROWTH=1 \
     -o ../../www/assets/wasm/sema.m.js
if [ $? -eq 0 ]; then
    echo "   ✓ 语义分析模块编译完成"
else
    echo "   ✗ 语义分析模块编译失败"
fi
cd "${SCRIPT_DIR}"

echo "4. 编译代码生成模块..."
cd losu/codegen
INCLUDE_DIRS="-I ../include"
LOSU_SRC_FILES=$(find ../losu -name "*.c" | grep -v "\.core\.dtc\.c" | tr '\n' ' ')
emcc main.c ${LOSU_SRC_FILES} ${INCLUDE_DIRS} \
     -s WASM=1 -s MODULARIZE=1 -s SINGLE_FILE=1 -s EXPORT_NAME="LosuCodegen" \
     -s EXPORTED_FUNCTIONS="['_codegen_demo', '_run', '_malloc', '_free']" \
     -s EXPORTED_RUNTIME_METHODS="['lengthBytesUTF8', 'stringToUTF8']" \
     -s ALLOW_MEMORY_GROWTH=1 \
     -o ../../www/assets/wasm/codegen.m.js
if [ $? -eq 0 ]; then
    echo "   ✓ 代码生成模块编译完成"
else
    echo "   ✗ 代码生成模块编译失败"
fi
cd "${SCRIPT_DIR}"

echo "5. 编译内存管理模块..."
cd losu/memory
INCLUDE_DIRS="-I ../include"
LOSU_SRC_FILES=$(find ../losu -name "*.c" | grep -v "\.core\.dtc\.c" | tr '\n' ' ')
emcc main.c ${LOSU_SRC_FILES} ${INCLUDE_DIRS} \
     -s WASM=1 -s MODULARIZE=1 -s SINGLE_FILE=1 -s EXPORT_NAME="LosuMemory" \
     -s EXPORTED_FUNCTIONS="['_memory_demo', '_run', '_malloc', '_free']" \
     -s EXPORTED_RUNTIME_METHODS="['lengthBytesUTF8', 'stringToUTF8']" \
     -s ALLOW_MEMORY_GROWTH=1 \
     -o ../../www/assets/wasm/memory.m.js
if [ $? -eq 0 ]; then
    echo "   ✓ 内存管理模块编译完成"
else
    echo "   ✗ 内存管理模块编译失败"
fi
cd "${SCRIPT_DIR}"

echo "6. 编译文件系统模块..."
cd losu/filesystem
INCLUDE_DIRS="-I ../include"
LOSU_SRC_FILES=$(find ../losu -name "*.c" | grep -v "\.core\.dtc\.c" | tr '\n' ' ')
emcc main.c ${LOSU_SRC_FILES} ${INCLUDE_DIRS} \
     -s WASM=1 -s MODULARIZE=1 -s SINGLE_FILE=1 -s EXPORT_NAME="LosuFilesystem" \
     -s EXPORTED_FUNCTIONS="['_filesystem_demo', '_demo_fs_read', '_demo_fs_write', '_demo_fs_mkdir', '_demo_fs_readdir', '_demo_fs_unlink', '_demo_fs_rename', '_demo_fs_stat', '_demo_fs_rmdir', '_run', '_malloc', '_free']" \
     -s EXPORTED_RUNTIME_METHODS="['lengthBytesUTF8', 'stringToUTF8']" \
     -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 \
     -o ../../www/assets/wasm/filesystem.m.js
if [ $? -eq 0 ]; then
    echo "   ✓ 文件系统模块编译完成"
else
    echo "   ✗ 文件系统模块编译失败"
fi
cd "${SCRIPT_DIR}"

echo ""
echo "所有模块编译完成！"
echo "编译后的WASM文件位于: www/assets/wasm/"
echo ""
echo "使用方法："
echo "1. 启动开发服务器: make run"
echo "2. 访问对应页面进行测试"
echo "   - 词法分析: http://localhost:8080/pages/lexer.html"
echo "   - 语法分析: http://localhost:8080/pages/parser.html"
echo "   - 语义分析: http://localhost:8080/pages/sema.html"  
echo "   - 代码生成: http://localhost:8080/pages/codegen.html"
echo "   - 内存管理: http://localhost:8080/pages/memory.html"
echo "   - 文件系统: http://localhost:8080/pages/filesystem.html" 