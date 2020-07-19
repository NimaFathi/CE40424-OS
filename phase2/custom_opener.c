#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <asm/unistd.h>
#include <linux/uaccess.h>
#include <linux/uidgid.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/string.h>
#include <linux/fcntl.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sina Bolouki");
MODULE_DESCRIPTION("PCB");
MODULE_VERSION("0.1");
static void **sys_call_table = NULL;

static char **filenames = NULL;
static int *filePrios = NULL;
static int *userIds = NULL;
static int *userPrios = NULL;

MODULE_LICENSE("GPL");

int checkFile(char *filename) {
    int i;

    for (i = 0 ; filenames[i][0] != '\0' ; i++) {
        if(strcmp(filenames[i], filename ) == 0 ) {
            return filePrios[i];
	}
    }
    return 0;
}

int checkUser(int userId) {
    int i;
    for (i = 0 ; userIds[i] < 60000; i++) {
         if (userIds[i] == userId) {
             return userPrios[i];
         }
    }
    return 0;
}

static asmlinkage long (*old_open) (const char __user *filename, int flags, umode_t mode);

static asmlinkage long custom_open(const char __user *filename, int flags, umode_t mode)
{
      int fileP = checkFile(filename);
      int userP = checkUser(get_current_user() -> uid.val);
      int newFlag = flags;
      printk(KERN_ALERT "Custom open invoked");
      printk(KERN_ALERT "user_id %d" , get_current_user() ->uid.val);
      if (flags & O_RDONLY) {
           if (fileP > userP) {
               newFlag ^= O_RDONLY;
	   }
      } 
      if (flags & O_WRONLY) {
           if (fileP < userP) {
               newFlag ^= O_WRONLY;
	   }
      }
      if (flags & O_RDWR) {
           if (fileP != userP) {
               newFlag ^= O_RDWR;
	   }
      }
      return old_open(filename, newFlag, mode);
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
    //disable_write_protection();
    //sys_call_table[__NR_open] = custom_open;
    //enable_write_protection();
    printk(KERN_INFO "Original open: %p; New open: %p\n", old_open, custom_open);

    return 0;
}

static void exit(void)
{
    pr_info("exit");
    printk(KERN_INFO "sys_call_table address: %p\n", sys_call_table);
    printk(KERN_INFO "open : %p\n", sys_call_table[__NR_open]);
    disable_write_protection();
    sys_call_table[__NR_open] = old_open;
    enable_write_protection();    
}

module_init(init);
module_exit(exit);

