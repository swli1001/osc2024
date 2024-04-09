#include "dtb.h"
#include "uart1.h"
#include "utils.h"
#include "cpio.h"

extern void* CPIO_DEFAULT_PLACE;
char* dtb_ptr;

// The structure represents the header of a Flattened Device Tree blob stored in big endian format.
struct fdt_header {
    uint32_t magic;            // Magic number identifying the FDT, should be 0xd00dfeed.
    uint32_t totalsize;        // Total size of the DTB, including the header and all other blocks.
    uint32_t off_dt_struct;    // Offset from the start of the header to the structure block.
    uint32_t off_dt_strings;   // Offset from the start of the header to the strings block.
    uint32_t off_mem_rsvmap;   // Offset from the start of the header to the memory reservation block.
    uint32_t version;          // Version number of the DTB format.
    uint32_t last_comp_version;// The last compatible version number of the DTB format.
    uint32_t boot_cpuid_phys;  // Physical CPU ID of the boot processor.
    uint32_t size_dt_strings;  // Size of the strings block.
    uint32_t size_dt_struct;   // Size of the structure block.
};

// Convert a 32-bit integer from big-endian to little-endian format.
// This is necessary when reading data from a source that uses big-endian format
// (e.g., network data or certain file formats like DTB) on a little-endian machine (most common architecture).
uint32_t uint32_endian_big2lttle(uint32_t data)
{
    // Cast the data to a char pointer, allowing us to access individual bytes.
    char* r = (char*)&data;
    
    // r[0] is the most significant byte (big-endian), and r[3] is the least.
    // Shift and OR the bytes to construct the little-endian integer.
    // r[3] is shifted by 0 bits (remains the least significant byte),
    // r[2] is shifted by 8 bits (becomes the second byte),
    // r[1] is shifted by 16 bits (becomes the third byte),
    // r[0] is shifted by 24 bits (becomes the most significant byte).
    return (r[3] << 0) | (r[2] << 8) | (r[1] << 16) | (r[0] << 24);
}

void traverse_device_tree(void *dtb_ptr, dtb_callback callback)
{
    struct fdt_header* header = dtb_ptr;

    // Verify the DTB's magic number to ensure it's a valid device tree.
    if(uint32_endian_big2lttle(header->magic) != 0xD00DFEED)
    {
        // If the magic number doesn't match, print an error message and exit the function.
        uart_puts("traverse_device_tree : wrong magic in traverse_device_tree");
        return;
    }
    // https://abcamus.github.io/2016/12/28/uboot%E8%AE%BE%E5%A4%87%E6%A0%91-%E8%A7%A3%E6%9E%90%E8%BF%87%E7%A8%8B/
    // https://blog.csdn.net/wangdapao12138/article/details/82934127
    uint32_t struct_size = uint32_endian_big2lttle(header->size_dt_struct);
    // Calculate pointers to the structure and strings blocks within the DTB.
    char* dt_struct_ptr = (char*)((char*)header + uint32_endian_big2lttle(header->off_dt_struct));
    char* dt_strings_ptr = (char*)((char*)header + uint32_endian_big2lttle(header->off_dt_strings));

    // Define the end of the structure block to know when the traversal is complete.
    char* end = (char*)dt_struct_ptr + struct_size;
    char* pointer = dt_struct_ptr; // Current position pointer within the structure block.

    while(pointer < end)
    {
        uint32_t token_type = uint32_endian_big2lttle(*(uint32_t*)pointer);

        // Advance the pointer by 4 bytes to move past the token type.
        // Each token type in the device tree is represented by a 32-bit (4-byte) integer.
        // After reading the token type, we need to move the pointer forward to access the
        // data associated with that token or to reach the next token in the sequence.
        pointer += 4;
        // Check if the current token indicates the beginning of a new device tree node.
        if(token_type == FDT_BEGIN_NODE)
        {
            // If it's a BEGIN_NODE token, invoke the callback function with the current
            // token type and the pointer to the node's name. At this point, the pointer
            // is positioned at the start of the node's name, a null-terminated string.
            callback(token_type, pointer, 0, 0);
            
            // Advance the pointer past the node name. The name is stored as a null-terminated
            // string, so we use strlen(pointer) to calculate the length of the string and move
            // the pointer beyond the name, including the null terminator.
            pointer += strlen(pointer);
            
            // Align the pointer to the next 4-byte boundary to maintain proper data alignment.
            // This alignment is necessary because the device tree specification requires that
            // data structures within the DTB are aligned on 4-byte boundaries. The alignment
            // ensures that subsequent reads are correctly aligned, especially important for
            // architectures that require or perform optimally with aligned memory access.
            // The expression calculates how many bytes are needed to make the pointer 4-byte aligned
            // and increments the pointer accordingly.
            pointer += 4 - ((unsigned long long)pointer % 4);
        }else if(token_type == FDT_END_NODE)
        {
            callback(token_type, 0, 0, 0);
        }else if(token_type == FDT_PROP)
        {
            // Read the length of the property's value and convert it from big endian to the system's native endianness.
            // This length tells us how many bytes we need to read to get the complete property value.
            uint32_t len = uint32_endian_big2lttle(*(uint32_t*)pointer);
            pointer += 4; // Move the pointer past the length field.

            // Calculate the property name's offset in the string block and then use it to get the property name.
            // The string block contains all the names used in the device tree.
            char* name = (char*)dt_strings_ptr + uint32_endian_big2lttle(*(uint32_t*)pointer);
            pointer += 4; // Move the pointer past the name offset field.

            // Call the callback function with the property's token type, name, pointer to its value, and length.
            // The callback function can then process the property as needed.
            callback(token_type, name, pointer, len);

            // Advance the pointer by the length of the property's value to reach the next token or data in the device tree.
            pointer += len;

            // Align the pointer to a 4-byte boundary to ensure proper reading of subsequent tokens or data.
            // This is necessary because the device tree specification requires that data is aligned on 4-byte boundaries.
            if((unsigned long long)pointer % 4 != 0) pointer += 4 - (unsigned long long)pointer % 4;
        }else if(token_type == FDT_NOP)
        {
            callback(token_type, 0, 0, 0);
        }else if(token_type == FDT_END)
        {
            callback(token_type, 0, 0, 0);
        }else
        {
            uart_puts("error type:%x\n",token_type);
            return;
        }
    }
}

// Callback function to display the structure of the device tree.
// This function visualizes the tree by indenting child nodes and properties,
// allowing for an easier understanding of the device tree's hierarchy.
void dtb_callback_show_tree(uint32_t node_type, char *name, void *data, uint32_t name_size)
{
    // 'level' keeps track of the current depth in the tree for indentation purposes.
    static int level = 0;

    // When a new node begins, increase the indentation level and print the node name with an opening brace.
    if(node_type == FDT_BEGIN_NODE)
    {
        for(int i = 0; i < level; i++) uart_puts("   ");
        uart_puts("%s{\n", name);
        level++;
    }
    // When a node ends, decrease the indentation level and close the brace.
    else if(node_type == FDT_END_NODE)
    {
        level--;
        for(int i = 0; i < level; i++) uart_puts("   ");
        uart_puts("}\n");
    }
    // For properties within a node, maintain the current indentation level and print the property name.
    else if(node_type == FDT_PROP)
    {
        for(int i = 0; i < level; i++) uart_puts("   ");
        uart_puts("%s\n", name);
    }
}

// Callback function to set the initial ramdisk (initrd) starting address
// This is used during the device tree traversal to find and set the location
// of the initial ramdisk in memory, which is necessary for the boot process.
void dtb_callback_initramfs(uint32_t node_type, char *name, void *value, uint32_t name_size) {
    // Check if the current node is a property ('FDT_PROP') and the name of the property is "linux,initrd-start".
    // The "linux,initrd-start" property indicates the start of the initial RAM filesystem (initrd).
    if(node_type == FDT_PROP && strcmp(name, "linux,initrd-start") == 0)
    {
        // The value of the property is the address of the initrd start, stored as a big-endian 32-bit value.
        // Use the `uint32_endian_big2lttle` function to convert it to the system's native endian format.
        // The converted address is then cast to a pointer and stored in the global variable `CPIO_DEFAULT_PLACE`.
        // This address is used by the system to access the initrd in memory during bootup.
        CPIO_DEFAULT_PLACE = (void *)(unsigned long long)uint32_endian_big2lttle(*(uint32_t*)value);
    }
}
