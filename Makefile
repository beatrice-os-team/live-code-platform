.PHONY : all help run

NODE := node

all: help

# 启动开发服务器
run:
	@echo "正在启动开发服务器..."
	@echo "请确保已安装依赖: npm install"
	@if [ ! -d "node_modules" ]; then \
		echo "正在安装依赖包..."; \
		npm install; \
	fi
	@echo "启动服务器..."
	$(NODE) index.js

# 构建演示模块
build-demos:
	@echo "正在构建编译器演示模块..."
	@if command -v emcc >/dev/null 2>&1; then \
		./build_demos.sh; \
	else \
		echo "警告: 未找到 emcc 编译器"; \
		echo "请安装 Emscripten SDK: https://emscripten.org/docs/getting_started/downloads.html"; \
		echo "或者使用现有的 WASM 模块进行演示"; \
	fi

# 清理生成的WASM文件
clean-demos:
	@echo "清理演示模块..."
	rm -f www/assets/wasm/lexer.m.js www/assets/wasm/lexer.m.wasm
	rm -f www/assets/wasm/parser.m.js www/assets/wasm/parser.m.wasm
	rm -f www/assets/wasm/sema.m.js www/assets/wasm/sema.m.wasm
	rm -f www/assets/wasm/codegen.m.js www/assets/wasm/codegen.m.wasm
	@echo "清理完成"

# 运行并构建演示模块
run-with-demos: build-demos run

# 显示帮助信息
help:
	@echo "Losu Live Code Platform - 编译器演示系统"
	@echo ""
	@echo "可用命令:"
	@echo "  make run          - 启动开发服务器"
	@echo "  make build-demos  - 构建编译器演示模块 (需要 Emscripten)"
	@echo "  make clean-demos  - 清理生成的演示模块"
	@echo "  make run-with-demos - 构建演示模块并启动服务器"
	@echo "  make help         - 显示此帮助信息"
	@echo ""
	@echo "演示模块:"
	@echo "  - 词法分析 (Lexer): http://localhost:8080/pages/lexer.html"
	@echo "  - 语法分析 (Parser): http://localhost:8080/pages/parser.html"
	@echo "  - 语义分析 (Semantic): http://localhost:8080/pages/sema.html"
	@echo "  - 代码生成 (CodeGen): http://localhost:8080/pages/codegen.html"
	@echo ""
	@echo "注意: 如果没有安装 Emscripten，系统将使用现有的 WASM 模块"

.PHONY: run build-demos clean-demos run-with-demos help
