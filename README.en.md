# FDTC Compiler

#### Introduction
FDTC (Flattened Device Tree Compiler) is specifically designed for embedded systems and uses a tree data structure to describe device trees. 
The compiler's workflow is as follows:
1. Read data from the source file and parse it into a tree structure, including lexical and syntactic analysis.
2. Convert the tree structure into a specific device tree binary format and write it to the target file.
3. Generate a `.c` file for embedded development.

## Quick Start
### 1. Compile the Device Tree
1. Download the [fdtc.exe](https://gitee.com/li-shan-asked/fdt/releases) compiler locally, then use the command line for operations.
2. Compile the device tree to generate a `.dtb` file, for example:
```shell
./fdtc.exe test.dts -o test.dtb
```
You will then obtain the `test.dtb` file, which can be directly placed in the Linux system and loaded into memory. For MCU users, you can also compile and generate a `.c` file.
3. Compile the device tree to generate a `.c` file, for example:
```shell
./fdtc.exe test.dts -c test.c
```
This will compile the `dts` file into a `c` file for MCU development.

### 2. Use the Device Tree in Development
Here is an example using MCU development. Copy the `fdt.c` and `fdt.h` files to the MCU development directory, then call the relevant interfaces to retrieve all information from the device:
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
Device nodes form a tree structure consisting of node names, properties, child nodes, and child node attributes, similar to Linux's dts but different. This FDT syntax uses four spaces for indentation and does not adopt C language’s `{}` scope. Below is a simple example:

1. Create a `uart` node under the root node with identifier `uart0`, property `compatible` set to `ns16550`, `reg` set to `0x10000000`, `interrupts` set to `0`, `clocks` set to `"apb_clk"`, `baudrate` set to `115200`, `tx-pin` set to `40`, and `rx-pin` set to `41`.
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

2. Create a `serial` node under the `uart` node with identifier `serial0`, property `compatible` set to `ns16550`, and `reg` set to `0x10000000`:
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
Alternatively, you can add child nodes via references:
```
&uart:
    status: "okay"
    serial0:
        compatible: "ns16550"
        reg: 0x10000000
```

> [!TIP]  
The root node defaults to `/` but is not explicitly written in the `dts`.

### 2. Add String Properties
FDT supports string properties, such as `compatible`, whose value is a string:
```
uart:
    compatible: "ns16550"
```

### 3. Add Integer Properties
FDT supports integer properties, such as `reg`, whose value is an integer:
```
uart:
    compatible: "ns16550"
    reg: 0x10000000
```

### 4. Add Array Properties
FDT supports array properties, such as `pins`, whose value is an integer array:
```
uart:
    compatible: "ns16550"
    reg: 0x10000000
    pins: [41 42]
```
> [!NOTE]  
Array properties can hold up to 256 elements; exceeding this limit will result in an error.

### 5. Add `dtsi` Files
`dtsi` files are subsets of `dts` files and can include some common configurations, for example:
```
#include "demo.dtsi"
...
uart:
    compatible: "ns16550"
    reg: 0x10000000
    ...
```
Where `#include` is a keyword used to import `dtsi` files. `dtsi` files are similar to `dts` files except for their suffix.

> [!NOTE]  
It is recommended that users place generic configurations in `dtsi` files so that other devices can reuse these configurations, and only need to reference them in `dts` files.

### 6. Node References
For node references, you can use the `&` symbol, for example, modify the `status` property of the `uart` node to `okay`:
```
&uart:
    status: "okay"
```

### 7. Add Macro Definitions
```
#define UART_COMPATIBLE "ns16550"
#define UART_REG 0x10000000

uart:
    compatible: UART_COMPATIBLE
    reg: UART_REG
    ...
```
