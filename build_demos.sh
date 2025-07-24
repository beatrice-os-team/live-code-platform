#!/bin/bash

echo "构建Losu编译器演示模块..."

# 检查emcc是否可用
if ! command -v emcc &> /dev/null; then
    echo "错误: 未找到emcc编译器，请安装Emscripten SDK"
    exit 1
fi

# 设置编译标志
EMCC_FLAGS="-O2 -s WASM=1 -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s EXPORT_NAME=LosuLiveCode"

# 记录当前目录
SCRIPT_DIR=$(pwd)

echo "1. 编译词法分析模块..."
cd losu/lexer
# 包含目录 (相对于当前编译目录)
INCLUDE_DIRS="-I ../include"
# 获取所有源文件，排除重复的.core.dtc.c文件
LOSU_SRC_FILES=$(find ../losu -name "*.c" | grep -v "\.core\.dtc\.c" | tr '\n' ' ')
emcc main.c ${LOSU_SRC_FILES} ${INCLUDE_DIRS} ${EMCC_FLAGS} -o ../../www/assets/wasm/lexer.m.js
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
emcc main.c ${LOSU_SRC_FILES} ${INCLUDE_DIRS} ${EMCC_FLAGS} -o ../../www/assets/wasm/parser.m.js
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
emcc main.c ${LOSU_SRC_FILES} ${INCLUDE_DIRS} ${EMCC_FLAGS} -o ../../www/assets/wasm/sema.m.js
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
emcc main.c ${LOSU_SRC_FILES} ${INCLUDE_DIRS} ${EMCC_FLAGS} -o ../../www/assets/wasm/codegen.m.js
if [ $? -eq 0 ]; then
    echo "   ✓ 代码生成模块编译完成"
else
    echo "   ✗ 代码生成模块编译失败"
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