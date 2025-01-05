# FDTC Compiler

#### Introduction
FDTC (Flattened Device Tree Compiler) is a tree data structure specifically designed for embedded systems, used to describe device trees. The compiler's workflow is as follows:
 1. Read data from source files and parse it into a tree structure, which includes lexical and syntactic analysis.
 2. Convert the tree structure into a specific device tree binary format and write it to the target file.
 3. Generate .c files for embedded development.

## Quick Start
### 1. Compiling the Device Tree
1. Download the [fdtc.exe](https://github.com/LiShanwenGit/fdt/releases/tag/fdtc-250105) compiler to your local machine, then operate via command line.
2. Compile the device tree to generate a dtb file, for example:
```shell
./fdtc.exe test.dts -o test.dtb
```
You can now obtain the test.dtb file, which can be directly placed in a Linux system and loaded into memory. For MCU users, you can also compile and generate .c files.

3. Compile the device tree to generate a .c file, for example:
```shell
./fdtc.exe test.dts -c test.c
```
This will compile the `dts` file into a `c` file for use in MCU development.

### 2. Using the Device Tree in Development
Taking MCU development as an example, copy the `fdt.c` and `fdt.h` files to the MCU development directory, then call the relevant interfaces to get all information from the device tree:
```c
int main(void)
{
    int ret = -1;
    // Call the fdt_load interface to read the device tree binary file
    ret = fdt_load((void*)fdt_blob, sizeof(fdt_blob));
    if(ret) {
        FDT_LOG_ERROR("fdt load failed\n");
        return -1;
    }

    // Print the version information of the device tree
    printf("version: %x\n", fdt_get_version());

    // Find the uart2 node from the root node
    fdt_node_t *uart2_node = fdt_find_node_by_name(NULL, "uart2");
    if(uart2_node) {
        printf("uart2_node name = %s\n", uart2_node->name);
    }

    // Find the /spi1/st7789v node by path
    fdt_node_t *st7789v = fdt_find_node_by_path("/spi1/st7789v/");
    if(st7789v) {
        printf("st7789v node name = %s\n", st7789v->name);
    }

    // Directly get the string of the /spi1/st7789v/compatible property by path
    const char *str = fdt_read_prop_string_by_path("/spi1/st7789v/", "compatible");
    if(str) {
        printf("/spi1/st7789v/compatible = %s\n", str);
    }

    fdt_node_t *child = NULL;
    fdt_node_t *root =  fdt_get_root_node();
    // Traverse all child nodes of the root node
    fdt_for_each_node_child(root, child) {
        printf("child node name = %s\n", child->name);
    }

    return 0;
}
```

## FDT Syntax
### 1. Device Nodes
Device nodes form a tree structure composed of node names, properties, child nodes, and child node properties, similar to Linux dts but different. This FDT syntax uses four spaces for indentation and does not adopt C language's {} scope. Below is a simple example:

1. Create a `uart` node under the root node with identifier `uart0`, where the `compatible` property is `ns16550`, `reg` is `0x10000000`, `interrupts` is `0`, `clocks` is `"apb_clk"`, `baudrate` is `115200`, `tx-pin` is `40`, and `rx-pin` is `41`.
```yaml
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

2. Create a `serial` node under the `uart` node with identifier `serial0`, where the `compatible` property is `ns16550` and `reg` is `0x10000000`:
```yaml
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
You can also add child nodes using references, as shown below:
```yaml
&uart:
    status: "okay"
    serial0:
        compatible: "ns16550"
        reg: 0x10000000
```
> [!TIP]  
The root node defaults to `/`, but it is not explicitly written in the `dts`. For example, the parent node of the `uart` node is `/`.

### 2. Adding String Properties
FDT supports string properties, such as `compatible`, whose value is a string, for example:
```yaml
uart:
    compatible: "ns16550"
```

### 3. Adding Integer Properties
FDT supports integer properties, such as `reg`, whose value is an integer, for example:
```yaml
uart:
    compatible: "ns16550"
    reg: 0x10000000
```

### 4. Adding Array Properties
FDT supports array properties, such as `pins`, whose value is an integer array, for example:
```yaml
uart:
    compatible: "ns16550"
    reg: 0x10000000
    pins: <41 42>
```
> [!NOTE]  
Array properties can only contain up to 256 elements; exceeding this limit will result in an error.

### 5. Including `dtsi` Files
`dtsi` files are subsets of `dts` files that can include some common configurations, for example:
```yaml
#include "demo.dtsi"
...
uart:
    compatible: "ns16550"
    reg: 0x10000000
    ...
```
The `#include` keyword is used to import `dtsi` files. `dtsi` files are the same as `dts` files except for the suffix.

> [!NOTE]  
It is recommended that users place common configurations in `dtsi` files so that other devices can reuse these configurations. Then, in the `dts` file, you only need to add device nodes through references.

### 6. Node References
For node references, you can use the `&` symbol to reference them, for example, modifying the `status` property of the `uart` node to `okay`:
```yaml
&uart:
    status: "okay"
```
