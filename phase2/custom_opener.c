#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <asm/unistd.h>
#include <linux/uaccess.h>
#include <linux/uidgid.h>

MODULE_LICENSE("GPL");

static void **sys_call_table = NULL;

static asmlinkage long (*old_open) (const char __user *filename, int flags, umode_t mode);

static asmlinkage long custom_open(const char __user *filename, int flags, umode_t mode)
{
      printk(KERN_INFO "Custom open invoked");
      printk(KERN_INFO "user_id %d" , get_current_user() ->uid.val);
      return old_open(filename, flags, mode);
}

inline void mywrite_cr0(unsigned long cr0) {
  asm volatile("mov %0,%%cr0" : "+r"(cr0), "+m"(__force_order));
}

void enable_write_protection(void) {
  unsigned long cr0 = read_cr0();
  set_bit(16, &cr0);
  mywrite_cr0(cr0);
}

void disable_write_protection(void) {
  unsigned long cr0 = read_cr0();
  clear_bit(16, &cr0);
  mywrite_cr0(cr0);
}

static int init(void)
{
    printk (KERN_INFO "in here");
    sys_call_table = (void *)kallsyms_lookup_name("sys_call_table");
    printk(KERN_INFO "sys_call_table address: %p\n", sys_call_table);

    old_open = sys_call_table[__NR_open];
    disable_write_protection();
    sys_call_table[__NR_open] = custom_open;
    enable_write_protection();
    printk(KERN_INFO "Original open: %p; New open: %p\n", old_open, custom_open);

    return 0;
}

static void exit(void)
{
    pr_info("exit");
    disable_write_protection();
    sys_call_table[__NR_open] = old_open;
    enable_write_protection();    
}

module_init(init);
module_exit(exit);

