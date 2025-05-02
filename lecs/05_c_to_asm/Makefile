# 编译器工具链
CC := clang -fPIE
OPT := opt
LLC := llc
AS := as

# 源文件和输出目录
SRC_DIR := src
BUILD_DIR := build
SRC := $(wildcard $(SRC_DIR)/*.c)
TARGET := program

# 中间文件路径（不再自动删除）
IR_LL := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.ll,$(SRC))
OPT_LL := $(patsubst $(BUILD_DIR)/%.ll,$(BUILD_DIR)/%.opt.ll,$(IR_LL))
ASM_S := $(patsubst $(BUILD_DIR)/%.ll,$(BUILD_DIR)/%.s,$(IR_LL))
OBJ_O := $(patsubst $(BUILD_DIR)/%.s,$(BUILD_DIR)/%.o,$(ASM_S))

# 默认目标：构建最终可执行文件
all: $(BUILD_DIR)/$(TARGET)

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 从C源码生成LLVM IR（文本格式）
$(BUILD_DIR)/%.ll: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -S -emit-llvm -o $@ $<

# 优化LLVM IR
$(BUILD_DIR)/%.opt.ll: $(BUILD_DIR)/%.ll
	$(OPT) -S -O3 -o $@ $<

# 从LLVM IR生成汇编代码（关键修复：添加位置无关代码模型）
$(BUILD_DIR)/%.s: $(BUILD_DIR)/%.ll
	$(LLC) -relocation-model=pic -o $@ $<

# 从汇编代码生成目标文件
$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.s
	$(AS) -o $@ $<

# 链接目标文件生成可执行文件
$(BUILD_DIR)/$(TARGET): $(OBJ_O)
	$(CC) -pie -o $@ $^

# 清理所有生成的文件（可选）
clean:
	rm -rf $(BUILD_DIR)

# 显示帮助信息
help:
	@echo "可用目标:"
	@echo "  all       - 构建可执行文件 (默认)"
	@echo "  clean     - 清理所有生成的文件"
	@echo "  help      - 显示帮助信息"
	@echo ""
	@echo "中间文件保存在 ./build 目录:"
	@echo "  *.ll      - 原始LLVM IR"
	@echo "  *.opt.ll  - 优化后的LLVM IR"
	@echo "  *.s       - 汇编代码"
	@echo "  *.o       - 目标文件"

# 关键修复：禁止Make自动删除中间文件
.PRECIOUS: $(IR_LL) $(ASM_S) $(OBJ_O)

.PHONY: all clean help