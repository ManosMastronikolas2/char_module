#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/compiler.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/io.h>

#include "/usr/src/nvidia-565.57.01/nvidia/nv-p2p.h"

/*A dummy module used just to resolve symbol errors at compile time*/

int nvidia_p2p_get_pages(uint64_t p2p_token, uint32_t va_space,
                         uint64_t virtual_address,
                         uint64_t length,
                         struct nvidia_p2p_page_table **page_table,
                         void (*free_callback)(void *data),
                         void *data)
{
    return -EINVAL;
}
EXPORT_SYMBOL(nvidia_p2p_get_pages);




int nvidia_p2p_free_page_table(struct nvidia_p2p_page_table *page_table)
{
    return -EINVAL;
}
EXPORT_SYMBOL(nvidia_p2p_free_page_table);

static int __init nv_p2p_dummy_init(void)
{
    return 0;
}


static void __exit nv_p2p_dummy_cleanup(void)
{
}









module_init(mod_init);
module_exit(mod_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manos Mastronikolas");