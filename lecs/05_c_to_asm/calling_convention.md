# Linux x86-64 Calling Convention 详解

## 核心规则
1. **参数传递顺序** (前6个参数):
   - 整数/指针: `RDI, RSI, RDX, RCX, R8, R9`
   - 浮点数: `XMM0-XMM7`
   - 超过6个参数时使用栈传递，从右到左压栈

2. **返回值**:
   - 整数: `RAX` (64-bit), `RAX:RDX` (128-bit)
   - 浮点数: `XMM0`, `XMM1`
   - 结构体: 小结构体通过寄存器返回，大结构体通过内存（由调用者分配）

3. **寄存器保护责任**:
   - 调用者保存 (Caller-saved): `RAX, RCX, RDX, RSI, RDI, R8-R11, XMM0-XMM15`
   - 被调用者保存 (Callee-saved): `RBX, RBP, RSP, R12-R15`

4. **栈对齐**:
   - 函数调用时栈指针 (`RSP`) 必须是16字节对齐
   - 注意：`call` 指令会压入8字节的返回地址，因此进入函数体时 `RSP` 实际上是8字节对齐

5. **红区 (Red Zone)**:
   - 函数可以使用 `RSP` 之下的128字节作为临时存储
   - 这个区域不会被信号处理器破坏

## 寄存器用途（扩展表）
| 功能        | 64位名   | 32位名    | 16位名    | 8位名（高/低）     | 说明                            |
| ---------- | ------ | ------- | ------- | ------------ | ------------------------------- |
| 返回值       | `%rax` | `%eax`  | `%ax`   | `%ah`, `%al` | 用于返回函数值，也用于除法操作            |
| 被调用者保存   | `%rbx` | `%ebx`  | `%bx`   | `%bh`, `%bl` | 被调用者必须在使用前保存，返回前恢复        |
| 第4个参数     | `%rcx` | `%ecx`  | `%cx`   | `%ch`, `%cl` | 第4个整型/指针参数，也用于循环计数         |
| 第3个参数     | `%rdx` | `%edx`  | `%dx`   | `%dh`, `%dl` | 第3个整型/指针参数，也用于I/O操作和乘除运算  |
| 第2个参数     | `%rsi` | `%esi`  | `%si`   | `%sil`       | 第2个整型/指针参数，历史上用于字符串源索引   |
| 第1个参数     | `%rdi` | `%edi`  | `%di`   | `%dil`       | 第1个整型/指针参数，历史上用于字符串目标索引 |
| 栈基址       | `%rbp` | `%ebp`  | `%bp`   | `%bpl`       | 通常作为帧指针，指向当前栈帧的基址        |
| 栈顶指针      | `%rsp` | `%esp`  | `%sp`   | `%spl`       | 栈顶，指向栈内最新分配的数据             |
| 第5个参数     | `%r8`  | `%r8d`  | `%r8w`  | `%r8b`       | 第5个整型/指针参数                   |
| 第6个参数     | `%r9`  | `%r9d`  | `%r9w`  | `%r9b`       | 第6个整型/指针参数                   |
| 临时寄存器    | `%r10` | `%r10d` | `%r10w` | `%r10b`      | 临时寄存器，系统调用中用于传递系统调用号    |
| 用于链接/调用  | `%r11` | `%r11d` | `%r11w` | `%r11b`      | 通常由调用者使用，如存调用地址           |
| 被调用者保存   | `%r12` | `%r12d` | `%r12w` | `%r12b`      | 保留性与 `%rbx` 类似                |
| 被调用者保存   | `%r13` | `%r13d` | `%r13w` | `%r13b`      | 同上                             |
| 被调用者保存   | `%r14` | `%r14d` | `%r14w` | `%r14b`      | 同上                             |
| 被调用者保存   | `%r15` | `%r15d` | `%r15w` | `%r15b`      | 同上                             |
| 指令指针     | `%rip` | N/A     | N/A     | N/A          | 指向下一条要执行的指令                  |

## 浮点寄存器
| 寄存器  | 用途                     | 保存责任   |
| ----- | ----------------------- | -------- |
| XMM0  | 第1个浮点参数/返回值        | 调用者保存 |
| XMM1  | 第2个浮点参数/第2个返回值    | 调用者保存 |
| XMM2-7| 第3-8个浮点参数           | 调用者保存 |
| XMM8-15| 临时寄存器              | 调用者保存 |

## 栈帧结构
典型的栈帧结构：
```
高地址
|-----------------|
| 调用者栈帧       |
|-----------------|
| 返回地址        | <- 由 call 指令压入
|-----------------|
| 保存的 RBP      | <- 函数序言: push %rbp
|-----------------|
| 局部变量        |
|                 |
|-----------------|
| 临时存储         |
|-----------------|
| 参数构建区域     | <- 为下一个函数调用准备参数（第7个及以后的参数）
|-----------------|
| [红区 - 128字节] | <- 可选的临时存储区，不需要调整 RSP
|-----------------|
低地址
```

## 函数调用流程详解

### 函数序言 (Prologue)
```assembly
push %rbp           # 保存调用者的栈基址
mov %rsp, %rbp      # 设置新的栈基址
sub $N, %rsp        # 为局部变量分配空间，N通常是16的倍数以保持16字节对齐
```

### 函数尾声 (Epilogue)
```assembly
mov %rbp, %rsp      # 恢复栈指针
pop %rbp            # 恢复调用者的栈基址
ret                 # 返回到调用点
```

## C语言示例与其汇编代码对照

### C代码
```c
// 函数C：计算三个参数之和
int C(int a, int b, int c) {
    return a + b + c;
}

// 函数B：调用C并加2个参数
int B(int x, int y) {
    return C(x, y, 5) + x + y;
}

// 函数A：调用B并处理结果
int A(void) {
    return B(2, 3) * 4;
}
```

### 对应的汇编代码

#### 函数C
```assembly
C:
    # 参数已在寄存器: a in %edi, b in %esi, c in %edx
    # 函数序言 (可能被优化掉)
    push    %rbp
    mov     %rsp, %rbp
    
    # 函数体
    mov     %edi, %eax     # 将a移到%eax
    add     %esi, %eax     # 加上b
    add     %edx, %eax     # 加上c
    
    # 函数尾声 (可能被优化掉)
    pop     %rbp
    ret
```

#### 函数B
```assembly
B:
    # 参数: x in %edi, y in %esi
    push    %rbp
    mov     %rsp, %rbp
    sub     $16, %rsp      # 分配栈空间
    
    # 保存参数，因为它们会在调用C后重复使用
    mov     %edi, -4(%rbp)    # 保存x到栈上
    mov     %esi, -8(%rbp)    # 保存y到栈上
    
    # 准备调用C(x, y, 5)
    # x已在%edi, y已在%esi
    mov     $5, %edx       # 设置第三个参数为5
    call    C
    
    # C返回值在%eax，加上x和y
    mov     -4(%rbp), %edx    # 取回x
    add     %edx, %eax        # 加上x
    mov     -8(%rbp), %edx    # 取回y
    add     %edx, %eax        # 加上y
    
    # 返回
    leave                   # 等价于 mov %rbp, %rsp; pop %rbp
    ret
```

#### 函数A
```assembly
A:
    push    %rbp
    mov     %rsp, %rbp
    
    # 准备调用B(2, 3)
    mov     $2, %edi       # 第一个参数: 2
    mov     $3, %esi       # 第二个参数: 3
    call    B
    
    # B返回值在%eax，乘以4
    imul    $4, %eax, %eax
    
    # 返回
    pop     %rbp
    ret
```

## 参数和返回值类型处理

### 不同类型的参数传递
1. **整数和指针**：按顺序使用 RDI, RSI, RDX, RCX, R8, R9
2. **浮点数**：按顺序使用 XMM0 到 XMM7
3. **结构体和联合体**：
   - 小于等于16字节的结构体：拆分到寄存器中
   - 大于16字节的结构体：通过栈传递，并传递指向该结构体的指针

### 混合类型参数示例
```c
void mixed_params(int a, double b, int c, double d) {
    // a在%edi, b在%xmm0, c在%esi, d在%xmm1
}
```

```assembly
mixed_params:
    # 参数: a in %edi, b in %xmm0, c in %esi, d in %xmm1
    push    %rbp
    mov     %rsp, %rbp
    # 函数体...
    pop     %rbp
    ret
```

## 高级特性

### 变长参数函数 (Variadic Functions)
变长参数函数（如printf）使用 `%al` 寄存器指示使用了多少个XMM寄存器：
```c
int printf(const char *format, ...);
```

调用变长参数函数时:
```assembly
mov     $2, %al        # 表示使用了2个XMM寄存器
call    printf
```

### System V ABI 与 Microsoft x64 ABI 区别
Linux 和 macOS 使用System V ABI，Windows使用Microsoft x64 ABI：

| 特性 | System V ABI (Linux/macOS) | Microsoft x64 ABI (Windows) |
| ---- | -------------------------- | ---------------------------- |
| 整数参数 | RDI, RSI, RDX, RCX, R8, R9 | RCX, RDX, R8, R9 |
| 浮点参数 | XMM0-XMM7 | XMM0-XMM3 |
| 栈对齐 | 16字节 | 16字节 |
| 调用者保存 | RAX,RCX,RDX,RSI,RDI,R8-R11 | RAX,RCX,RDX,R8-R11 |
| 被调用者保存 | RBX,RBP,RSP,R12-R15 | RBX,RBP,RSI,RDI,RSP,R12-R15 |
| 影子空间 | 无 | 必须为每个函数调用分配32字节 |

## 调用约定性能考虑
1. **避免使用过多参数**：前6个参数通过寄存器传递，效率最高
2. **减少跨调用保存的临时值**：过多使用被调用者保存寄存器可能导致大量压栈和出栈操作
3. **利用红区**：对于叶函数（不调用其他函数的函数），可以利用红区而不调整RSP
4. **内联小函数**：频繁调用的小函数可以考虑内联以避免调用开销

## 实用调试提示
1. 使用 GDB 检查寄存器状态：`info registers`
2. 以汇编格式显示函数：`disassemble function_name`
3. 在编译时添加帧指针：`gcc -fno-omit-frame-pointer`