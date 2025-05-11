# FDTC 编译器

#### 介绍
FDTC (Flattened Device Tree Compiler)是一个专为嵌入式而设计的树形数据结构，用于描述设备树。
该编译器的工作流程如下：   
 1. 从源文件中读取数据，并解析为树形结构，其中包含词法和语法分析
 2. 将树形结构转换为特定的设备树二进制格式，并写入目标文件中。
 3. 生成.c文件用于嵌入式开发

## 快速开始
### 1. 编译设备树
1. 下载[fdtc.exe](https://gitee.com/li-shan-asked/fdt/releases)编译器到本地，然后使用命令行进行操作
2. 编译设备树，生成dtb文件，例如：
```shell
./fdtc.exe test.dts -o test.dtb
```
此时就可以得到test.dtb文件了，该文件可以直接放在Linux系统中，然后搬运到内存中加载。对于MCU用户而言，也可以编译生成.c文件

3. 编译设备树生成.c文件，例如：
```shell
./fdtc.exe test.dts -c test.c
```
这样就可以将`dts`文件编译为`c`文件，供MCU开发使用了

### 2. 在开发中使用设备树
这里以MCU开发为例，将`fdt.c`和`fdt.h`文件拷贝到MCU开发目录中，然后调用相关的接口即可获取设备中的所有信息：
```c
int main(void)
{
    int ret = -1;
    //调用fdt_load接口，读取设备树二进制文件
    ret = fdt_load((void*)fdt_blob, sizeof(fdt_blob));
    if(ret) {
        FDT_LOG_ERROR("fdt load failed\n");
        return -1;
    }

    //打印设备树的版本信息
    printf("version: %x\n", fdt_get_version());

    //从根节点查找uart2节点
    fdt_node_t *uart2_node = fdt_find_node_by_name(NULL, "uart2");
    if(uart2_node) {
        printf("uart2_node name = %s\n", uart2_node->name);
    }

    //通过路径查找/spi1/st7789v节点
    fdt_node_t *st7789v = fdt_find_node_by_path("/spi1/st7789v/");
    if(st7789v) {
        printf("st7789v node name = %s\n", st7789v->name);
    }

    //通过路径直接获取/spi1/st7789v/compatible属性的字符串
    const char *str = fdt_read_prop_string_by_path("/spi1/st7789v/", "compatible");
    if(str) {
        printf("/spi1/st7789v/compatible = %s\n", str);
    }

    fdt_node_t *child = NULL;
    fdt_node_t *root =  fdt_get_root_node();
    //遍历根节点的所有子节点
    fdt_for_each_node_child(root, child) {
        printf("child node name = %s\n", child->name);
    }

    return 0;
}

```

## FDT语法
### 1. 设备节点
设备节点是树形结构，由节点名称、属性、子节点、子节点属性等组成，类似于Linux的dts，但是和Linux的dts又不同，此FDT语法采用四个空格为缩进，不采用C语言的{}作用域。下面是一个简单的例子说明： 

1. 在根节点下创建一个`uart`节点，其识符为`uart0`，属性`compatible`为`ns16550`，`reg`为`0x10000000`，`interrupts`为`0`，`clocks`为`"apb_clk"`，`baudrate`为`115200`，`tx-pin`为`40`，`rx-pin`为`41`。
```
uart:
    compatible: "ns16550"
    reg: 0x10000000
    reg-size: 0x100
    interrupts: 0
    clocks: "apb_clk"
    baudrate: 115200
    tx-pin: 40
    rx-pin: 41
```

2. 在`uart`节点下创建一个`serial`节点，其识别符为`serial0`，属性`compatible`为`ns16550`，`reg`为`0x10000000`：
```
uart:
    compatible: "ns16550"
    reg: 0x10000000
    reg-size: 0x100
    interrupts: 0
    clocks: "apb_clk"
    baudrate: 115200
    tx-pin: 40
    rx-pin: 41
    status: "disabled"

    serial0:
        compatible: "ns16550"
        reg: 0x10000000
```
这里使用了最简单的方式，直接在需要添加子节点的节点下添加子节点，也可以通过`引用`的方式来添加子节点，如下所示：
```
&uart:
    status: "okay"
    serial0:
        compatible: "ns16550"
        reg: 0x10000000
```
> [!TIP]  
根节点默认是`/`，但是并不会显示写在`dts`中，例如上面的`uart`节点，其父节点是`/`。


### 2. 添加字符串属性
`FDT`支持字符串属性，例如`compatible`是字符串属性，其值就是一个字符串，如下：
```
uart:
    compatible: "ns16550"
```
### 3. 添加整型属性
`FDT`支持整型属性，例如`reg`是整型属性，其值就是一个整型数据，如下：
```
uart:
    compatible: "ns16550"
    reg: 0x10000000
```
### 4. 添加数组属性
`FDT`支持数组属性，例如`pins`是数组属性，其值是一个整型数组，如下：
```
uart:
    compatible: "ns16550"
    reg: 0x10000000
    pins: [41 42]
```
> [!NOTE]  
数组属性最大只能容纳256个元素，超过256个元素会报错。

### 5. 添加`dtsi`文件
`dtsi`文件是`dts`文件的子集，可以包含一些通用的配置，例如：
```
#include "demo.dtsi"
...
uart:
    compatible: "ns16550"
    reg: 0x10000000
    ...
```
其中`#include`是一个关键字，用来导入`dtsi`文件。`dtsi`文件和`dts`文件一样，只是后缀不同。

> [!NOTE]  
建议用户将一些通用的配置放在`dtsi`文件中，这样其他设备也可以复用这些配置，然后在`dts`文件中只需要通过引用的方式来添加设备节点。

### 6. 节点引用
对于节点的引用，可以使用`&`符号来引用，例如下面修改`uart`节点的`status`属性为`okay`：
```
&uart:
    status: "okay"
```

### 7. 添加宏定义
```
#define UART_COMPATIBLE "ns16550"
#define UART_REG 0x10000000

uart:
    compatible: UART_COMPATIBLE
    reg: UART_REG
    ...

```
