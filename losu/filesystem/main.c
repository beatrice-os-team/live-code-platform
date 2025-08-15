#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>

#include "losu.h"

// 文件系统操作类型
typedef enum {
    FS_OP_READ,
    FS_OP_WRITE,
    FS_OP_APPEND,
    FS_OP_DELETE,
    FS_OP_RENAME,
    FS_OP_MKDIR,
    FS_OP_LIST,
    FS_OP_STAT,
    FS_OP_EXISTS
} fs_operation_type_t;

// 打印文件系统操作结果
void print_fs_operation(fs_operation_type_t op, const char* path, const char* result) {
    const char* op_names[] = {
        "读取文件", "写入文件", "追加文件", "删除文件", 
        "重命名文件", "创建目录", "列出目录", "获取文件信息", "检查文件存在"
    };
    
    printf("📁 文件系统操作: %s\n", op_names[op]);
    printf("   路径: %s\n", path);
    printf("   结果: %s\n", result);
    printf("   ──────────────────\n");
}

// 文件系统是否已初始化的标志
static int fs_initialized = 0;

// 确保目录存在的辅助函数（只在首次调用时初始化文件和目录）
static void ensure_demo_directory() {
    // 总是确保基础 /demo 目录存在
    mkdir("/demo", 0755);
    
    // 只在首次调用时创建默认子目录和文件
    if (!fs_initialized) {
        FILE* fp;
        
        // 创建默认子目录
        mkdir("/demo/subdir", 0755);
        
        // hello.txt
        fp = fopen("/demo/hello.txt", "w");
        if (fp) {
            fputs("Hello, FileSystem Demo!", fp);
            fclose(fp);
        }
        
        // data.txt
        fp = fopen("/demo/data.txt", "w");
        if (fp) {
            fputs("This is a test file for filesystem operations.", fp);
            fclose(fp);
        }
        
        // numbers.txt
        fp = fopen("/demo/numbers.txt", "w");
        if (fp) {
            fputs("1\n2\n3\n4\n5", fp);
            fclose(fp);
        }
        
        // 子目录文件
        fp = fopen("/demo/subdir/nested.txt", "w");
        if (fp) {
            fputs("Nested file content", fp);
            fclose(fp);
        }
        
        fs_initialized = 1;
        printf("🔧 首次初始化文件系统，创建了默认演示文件和目录\n");
    }
}

// 确保基础目录存在但不重新创建子目录和文件的函数
static void ensure_demo_directory_only() {
    // 只确保基础 /demo 目录存在，不强制重建子目录
    mkdir("/demo", 0755);
}

// 文件系统初始化函数 - 页面加载时自动调用
EMSCRIPTEN_KEEPALIVE void filesystem_init() {
    printf("=== 文件系统自动初始化 ===\n");
    
    // 强制进行首次初始化，创建所有默认文件和目录
    ensure_demo_directory();
    
    printf("✅ 文件系统初始化完成！\n");
    printf("📁 已创建默认演示文件和目录\n");
    printf("💡 您现在可以开始使用文件系统功能了\n");
}

// 演示文件读取操作
EMSCRIPTEN_KEEPALIVE void demo_fs_read(const char* filepath) {
    printf("=== 文件系统读取演示 ===\n");
    printf("正在读取文件: %s\n", filepath);
    
    // 确保演示目录存在（但不重新创建被删除的文件）
    ensure_demo_directory_only();
    
    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        printf("❌ 文件读取失败: %s\n", strerror(errno));
        print_fs_operation(FS_OP_READ, filepath, "失败");
        return;
    }
    
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (file_size < 0) {
        printf("❌ 无法获取文件大小\n");
        fclose(fp);
        print_fs_operation(FS_OP_READ, filepath, "失败");
        return;
    }
    
    printf("📏 文件大小: %lld 字节\n", (long long)file_size);
    
    // 读取文件内容
    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        printf("❌ 内存分配失败\n");
        fclose(fp);
        print_fs_operation(FS_OP_READ, filepath, "失败");
        return;
    }
    
    size_t bytes_read = fread(buffer, 1, file_size, fp);
    buffer[bytes_read] = '\0';
    
    if (ferror(fp)) {
        printf("❌ 文件读取错误: %s\n", strerror(errno));
        free(buffer);
        fclose(fp);
        print_fs_operation(FS_OP_READ, filepath, "失败");
        return;
    }
    
    printf("✅ 文件读取成功!\n");
    printf("📄 文件内容:\n");
    printf("─────────────────\n");
    printf("%s\n", buffer);
    printf("─────────────────\n");
    printf("📊 实际读取: %zu 字节\n", bytes_read);
    
    free(buffer);
    fclose(fp);
    print_fs_operation(FS_OP_READ, filepath, "成功");
}

// 演示文件写入操作
EMSCRIPTEN_KEEPALIVE void demo_fs_write(const char* filepath, const char* content) {
    printf("=== 文件系统写入演示 ===\n");
    printf("正在写入文件: %s\n", filepath);
    printf("写入内容: %s\n", content);
    
    // 对于写入操作，如果是首次调用任何文件系统操作，才创建默认演示文件
    ensure_demo_directory();
    
    // 如果路径包含目录，确保目录存在
    char* dir_path = strdup(filepath);
    char* last_slash = strrchr(dir_path, '/');
    if (last_slash && last_slash != dir_path) {
        *last_slash = '\0';
        // 递归创建目录
        mkdir(dir_path, 0755);
    }
    free(dir_path);
    
    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        printf("❌ 文件写入失败: %s\n", strerror(errno));
        print_fs_operation(FS_OP_WRITE, filepath, "失败");
        return;
    }
    
    size_t content_len = strlen(content);
    size_t bytes_written = fwrite(content, 1, content_len, fp);
    
    if (ferror(fp)) {
        printf("❌ 文件写入错误: %s\n", strerror(errno));
        fclose(fp);
        print_fs_operation(FS_OP_WRITE, filepath, "失败");
        return;
    }
    
    fclose(fp);
    
    printf("✅ 文件写入成功!\n");
    printf("📏 预期写入: %zu 字节\n", content_len);
    printf("📊 实际写入: %zu 字节\n", bytes_written);
    
    // 验证写入
    printf("🔍 验证写入内容...\n");
    FILE* verify_fp = fopen(filepath, "rb");
    if (verify_fp) {
        fseek(verify_fp, 0, SEEK_END);
        long verify_size = ftell(verify_fp);
        fseek(verify_fp, 0, SEEK_SET);
        
        if (verify_size > 0) {
            char* verify_buffer = malloc(verify_size + 1);
            if (verify_buffer) {
                size_t verify_read = fread(verify_buffer, 1, verify_size, verify_fp);
                verify_buffer[verify_read] = '\0';
                printf("✅ 验证成功，文件大小: %ld 字节\n", verify_size);
                printf("📄 验证内容: %s\n", verify_buffer);
                free(verify_buffer);
            }
        }
        fclose(verify_fp);
    }
    
    print_fs_operation(FS_OP_WRITE, filepath, "成功");
}

// 演示目录操作
EMSCRIPTEN_KEEPALIVE void demo_fs_mkdir(const char* dirpath) {
    printf("=== 文件系统目录创建演示 ===\n");
    printf("正在创建目录: %s\n", dirpath);
    
    // 确保演示目录存在
    ensure_demo_directory();
    
#ifdef __EMSCRIPTEN__
    // 在Emscripten环境中，mkdir可能需要特殊处理
    if (mkdir(dirpath, 0755) != 0) {
#else
    // 标准Unix系统
    if (mkdir(dirpath, 0755) != 0) {
#endif
        printf("❌ 目录创建失败: %s\n", strerror(errno));
        print_fs_operation(FS_OP_MKDIR, dirpath, "失败");
        return;
    }
    
    printf("✅ 目录创建成功!\n");
    
    // 验证目录存在
    struct stat st;
    if (stat(dirpath, &st) == 0 && S_ISDIR(st.st_mode)) {
        printf("✅ 验证目录存在，权限: %o\n", st.st_mode & 0777);
    } else {
        printf("⚠️ 无法验证目录状态\n");
    }
    
    print_fs_operation(FS_OP_MKDIR, dirpath, "成功");
}

// 演示目录列表操作
EMSCRIPTEN_KEEPALIVE void demo_fs_readdir(const char* dirpath) {
    printf("=== 文件系统目录列表演示 ===\n");
    printf("正在列出目录: %s\n", dirpath);
    
    // 确保演示目录存在（但不重新创建被删除的文件）
    ensure_demo_directory_only();
    
    DIR* dir = opendir(dirpath);
    if (!dir) {
        printf("❌ 目录列表失败: %s\n", strerror(errno));
        print_fs_operation(FS_OP_LIST, dirpath, "失败");
        return;
    }
    
    printf("✅ 目录打开成功!\n");
    printf("📂 目录内容:\n");
    printf("─────────────────\n");
    
    struct dirent* entry;
    int count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        count++;
        
        // 构建完整路径
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);
        
        // 获取文件信息
        struct stat st;
        if (stat(fullpath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                printf("  %d. 📁 目录 %s\n", count, entry->d_name);
            } else if (S_ISREG(st.st_mode)) {
                printf("  %d. 📄 文件 %s (%lld 字节)\n", count, entry->d_name, (long long)st.st_size);
            } else {
                printf("  %d. 🔗 其他 %s\n", count, entry->d_name);
            }
        } else {
            printf("  %d. ❓ %s (无法获取信息)\n", count, entry->d_name);
        }
    }
    
    if (count == 0) {
        printf("  (目录为空)\n");
    }
    
    printf("─────────────────\n");
    printf("📊 总计: %d 个项目\n", count);
    
    closedir(dir);
    print_fs_operation(FS_OP_LIST, dirpath, "成功");
}

// 演示文件删除操作
EMSCRIPTEN_KEEPALIVE void demo_fs_unlink(const char* filepath) {
    printf("=== 文件系统删除演示 ===\n");
    printf("正在删除文件: %s\n", filepath);
    
    // 确保演示目录存在（但不重新创建被删除的文件）
    ensure_demo_directory_only();
    
    // 检查文件是否存在
    struct stat st;
    if (stat(filepath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            printf("📄 删除前文件信息:\n");
            printf("   文件大小: %lld 字节\n", (long long)st.st_size);
            printf("   文件权限: %o\n", st.st_mode & 0777);
        } else {
            printf("⚠️ 指定路径不是普通文件\n");
        }
    } else {
        printf("❌ 文件不存在: %s\n", strerror(errno));
        print_fs_operation(FS_OP_DELETE, filepath, "失败");
        return;
    }
    
    // 删除文件
    if (unlink(filepath) != 0) {
        printf("❌ 文件删除失败: %s\n", strerror(errno));
        print_fs_operation(FS_OP_DELETE, filepath, "失败");
        return;
    }
    
    printf("✅ 文件删除成功!\n");
    
    // 验证删除
    if (stat(filepath, &st) != 0) {
        printf("✅ 验证: 文件已成功删除\n");
    } else {
        printf("⚠️ 验证: 文件仍然存在 (可能删除失败)\n");
    }
    
    print_fs_operation(FS_OP_DELETE, filepath, "成功");
}

// 演示文件重命名操作
EMSCRIPTEN_KEEPALIVE void demo_fs_rename(const char* oldpath, const char* newpath) {
    printf("=== 文件系统重命名演示 ===\n");
    printf("重命名: %s -> %s\n", oldpath, newpath);
    
    // 确保演示目录存在（但不重新创建被删除的文件）
    ensure_demo_directory_only();
    
    // 检查源文件是否存在
    struct stat st;
    if (stat(oldpath, &st) != 0) {
        printf("❌ 源文件不存在: %s\n", strerror(errno));
        print_fs_operation(FS_OP_RENAME, oldpath, "失败");
        return;
    }
    
    printf("📄 重命名前文件信息:\n");
    printf("   原路径: %s\n", oldpath);
    printf("   文件大小: %lld 字节\n", (long long)st.st_size);
    
    // 重命名文件
    if (rename(oldpath, newpath) != 0) {
        printf("❌ 文件重命名失败: %s\n", strerror(errno));
        print_fs_operation(FS_OP_RENAME, oldpath, "失败");
        return;
    }
    
    printf("✅ 文件重命名成功!\n");
    printf("   新路径: %s\n", newpath);
    
    // 验证重命名
    if (stat(oldpath, &st) != 0) {
        printf("✅ 验证: 原文件已不存在\n");
    } else {
        printf("⚠️ 验证: 原文件仍然存在 (可能重命名失败)\n");
    }
    
    if (stat(newpath, &st) == 0) {
        printf("✅ 验证: 新文件存在, 大小: %lld 字节\n", (long long)st.st_size);
    } else {
        printf("⚠️ 验证: 新文件不存在 (重命名可能失败)\n");
    }
    
    print_fs_operation(FS_OP_RENAME, oldpath, "成功");
}

// 演示文件统计信息
EMSCRIPTEN_KEEPALIVE void demo_fs_stat(const char* filepath) {
    printf("=== 文件系统统计信息演示 ===\n");
    printf("正在获取文件信息: %s\n", filepath);
    
    // 确保演示目录存在（但不重新创建被删除的文件）
    ensure_demo_directory_only();
    
    struct stat st;
    if (stat(filepath, &st) != 0) {
        printf("❌ 获取文件信息失败: %s\n", strerror(errno));
        print_fs_operation(FS_OP_STAT, filepath, "失败");
        return;
    }
    
    printf("✅ 文件统计信息:\n");
    printf("   路径: %s\n", filepath);
    printf("   大小: %lld 字节\n", (long long)st.st_size);
    printf("   模式: %o\n", st.st_mode);
    printf("   修改时间: %lld\n", (long long)st.st_mtime);
    printf("   访问时间: %lld\n", (long long)st.st_atime);
    printf("   创建时间: %lld\n", (long long)st.st_ctime);
    printf("   inode: %ld\n", (long)st.st_ino);
    printf("   链接数: %ld\n", (long)st.st_nlink);
    
    printf("📊 类型判断:\n");
    printf("   是普通文件: %s\n", S_ISREG(st.st_mode) ? "是" : "否");
    printf("   是目录: %s\n", S_ISDIR(st.st_mode) ? "是" : "否");
    printf("   是符号链接: %s\n", S_ISLNK(st.st_mode) ? "是" : "否");
    printf("   是字符设备: %s\n", S_ISCHR(st.st_mode) ? "是" : "否");
    printf("   是块设备: %s\n", S_ISBLK(st.st_mode) ? "是" : "否");
    printf("   是FIFO: %s\n", S_ISFIFO(st.st_mode) ? "是" : "否");
    printf("   是Socket: %s\n", S_ISSOCK(st.st_mode) ? "是" : "否");
    
    print_fs_operation(FS_OP_STAT, filepath, "成功");
}

// 演示目录删除操作
EMSCRIPTEN_KEEPALIVE void demo_fs_rmdir(const char* dirpath) {
    printf("=== 文件系统目录删除演示 ===\n");
    printf("正在删除目录: %s\n", dirpath);
    
    // 确保演示目录存在（但不重新创建被删除的文件）
    ensure_demo_directory_only();
    
    // 检查目录是否存在
    struct stat st;
    if (stat(dirpath, &st) != 0) {
        printf("❌ 目录不存在: %s\n", strerror(errno));
        print_fs_operation(FS_OP_DELETE, dirpath, "失败");
        return;
    }
    
    if (!S_ISDIR(st.st_mode)) {
        printf("❌ 指定路径不是目录\n");
        print_fs_operation(FS_OP_DELETE, dirpath, "失败");
        return;
    }
    
    printf("📂 删除前目录信息:\n");
    printf("   目录路径: %s\n", dirpath);
    printf("   目录权限: %o\n", st.st_mode & 0777);
    
    // 尝试删除目录
    if (rmdir(dirpath) != 0) {
        if (errno == ENOTEMPTY) {
            printf("❌ 目录删除失败: 目录不为空\n");
            printf("💡 提示: 请先删除目录中的所有文件和子目录\n");
            
            // 显示目录内容
            printf("📋 目录内容:\n");
            DIR* dir = opendir(dirpath);
            if (dir) {
                struct dirent* entry;
                int count = 0;
                while ((entry = readdir(dir)) != NULL) {
                    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                        count++;
                        printf("   %d. %s\n", count, entry->d_name);
                    }
                }
                closedir(dir);
                if (count == 0) {
                    printf("   (目录为空，但删除仍然失败)\n");
                }
            }
        } else {
            printf("❌ 目录删除失败: %s\n", strerror(errno));
        }
        print_fs_operation(FS_OP_DELETE, dirpath, "失败");
        return;
    }
    
    printf("✅ 目录删除成功!\n");
    
    // 验证删除
    if (stat(dirpath, &st) != 0) {
        printf("✅ 验证: 目录已成功删除\n");
    } else {
        printf("⚠️ 验证: 目录仍然存在 (可能删除失败)\n");
    }
    
    print_fs_operation(FS_OP_DELETE, dirpath, "成功");
}

EMSCRIPTEN_KEEPALIVE void run(const char* input) {
    // 保持原有的完整运行功能
    losu_vm_t vm = vm_create(1024);
    if (!vm) {
        fprintf(stderr, "Failed to create Losu VM\n");
        return;
    }
    vm_setargs(vm, 0, NULL);
    vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);
    if (vm_dostring(vm, input) == 0) {
        losu_ctype_bool b = 1;
        while (b) {
            b = 0;
            for (losu_object_coroutine_t coro = vm->coropool; coro;
                 coro = coro->next) {
                if (vm_await(vm, coro) != -1)
                    b = 1;
            }
        }
        gc_setthreshold(vm, 0);
        gc_collect(vm);
        printf("--------------------------------\n");
        printf("mem max: %.8g KB\n", (double)gc_getmemmax(vm) / 1024);
        printf("mem now: %.8g KB\n", (double)gc_getmemnow(vm) / 1024);
        printf("运行结束\n");
    } else {
        fprintf(stderr, "运行错误\n");
    }
    vm_close(vm);
} 

// 综合文件系统演示
EMSCRIPTEN_KEEPALIVE void filesystem_demo(const char* input) {
    if (!input || strlen(input) == 0) {
        printf("文件系统演示输入为空\n");
        return;
    }

    printf("=== Losu文件系统操作演示 ===\n");
    printf("输入代码:\n%s\n", input);
    printf("\n=== 开始文件系统演示 ===\n");
    
    // 创建虚拟机并初始化文件系统扩展
    losu_vm_t vm = vm_create(1024);
    if (!vm) {
        fprintf(stderr, "创建虚拟机失败\n");
        return;
    }

    vm_setargs(vm, 0, NULL);
    vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);
    
    // 初始化演示文件系统
    printf("🔧 初始化演示文件系统...\n");
    
    // 创建演示目录
    if (mkdir("/demo", 0755) == 0 || errno == EEXIST) {
        printf("✅ 创建演示目录 /demo\n");
        
        // 创建一些演示文件
        FILE* fp;
        
        fp = fopen("/demo/hello.txt", "w");
        if (fp) {
            fputs("Hello, FileSystem Demo!", fp);
            fclose(fp);
        }
        
        fp = fopen("/demo/data.txt", "w");
        if (fp) {
            fputs("This is a test file for filesystem operations.", fp);
            fclose(fp);
        }
        
        fp = fopen("/demo/numbers.txt", "w");
        if (fp) {
            fputs("1\n2\n3\n4\n5", fp);
            fclose(fp);
        }
        
        printf("✅ 创建演示文件\n");
        
        // 创建子目录
        if (mkdir("/demo/subdir", 0755) == 0 || errno == EEXIST) {
            fp = fopen("/demo/subdir/nested.txt", "w");
            if (fp) {
                fputs("Nested file content", fp);
                fclose(fp);
            }
        }
        
        printf("✅ 文件系统初始化完成\n");
    } else {
        printf("❌ 文件系统初始化失败: %s\n", strerror(errno));
    }
    
    printf("📋 演示各种文件系统操作:\n\n");
    
    // 1. 列出根目录
    demo_fs_readdir("/demo");
    
    // 2. 读取文件
    demo_fs_read("/demo/hello.txt");
    
    // 3. 获取文件信息
    demo_fs_stat("/demo/data.txt");
    
    // 4. 写入新文件
    demo_fs_write("/demo/new_file.txt", "This is a newly created file!");
    
    // 5. 重命名文件
    demo_fs_rename("/demo/numbers.txt", "/demo/renamed_numbers.txt");
    
    // 6. 创建新目录
    demo_fs_mkdir("/demo/new_directory");
    
    // 7. 再次列出目录，查看变化
    demo_fs_readdir("/demo");
    
    // 8. 删除文件
    demo_fs_unlink("/demo/new_file.txt");
    
    // 9. 最终列出目录
    demo_fs_readdir("/demo");
    
    printf("\n=== 文件系统演示完成 ===\n");
    printf("💡 提示: 可以在代码编辑器中使用 fs.read(), fs.write() 等函数\n");
    printf("🔍 支持的操作: read, write, append, rename, remove\n");
    
    // 尝试执行用户代码
    if (strstr(input, "fs.") != NULL) {
        printf("\n=== 执行用户代码 ===\n");
        if (vm_dostring(vm, input) == 0) {
            printf("✅ 用户代码执行完成\n");
        } else {
            printf("❌ 用户代码执行失败\n");
        }
    }
    
    // 清理资源
    vm_close(vm);

    run(input);
}
