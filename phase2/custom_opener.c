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
#include <linux/device.h>
#include <linux/cdev.h>

#define MODULE
#define LINUX
#define __KERNEL__
#define GFP_KERNEL      (__GFP_RECLAIM | __GFP_IO | __GFP_FS)

#define DEV_NAME "custom_open"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Sina Bolouki");
MODULE_DESCRIPTION("Custom");

static dev_t open_device;
static struct class *open_class;
static struct cdev *open_cdev;

static void **sys_call_table = NULL;
static char fileNames [100][50];
static int fileAccs [100];
static int userIds[100];
static int userAccs[100];
char writer[2000];
int fileNum;
int userNum;
int checkFile(char *filename) {
    int i;
    if (fileNames == NULL) {
        return 0;
    }
    for (i = 0 ; i < fileNum && i < 100 ; i++) {
        if(strcmp(fileNames[i], filename ) == 0 ) {
            return fileAccs[i];
	}
    }
    return 0;
}

int checkUser(int userId) {
    int i;
    if (userIds == NULL) {
        return 0;
    }
    for (i = 0 ; i < userNum && i < 100; i++) {
         if (userIds[i] == userId) {
             return userAccs[i];
         }
    }
    return 1;
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


static asmlinkage long (*old_open) (const char __user *filename, int flags, umode_t mode);

static asmlinkage long custom_open(const char __user *filename, int flags, umode_t mode)
{
      int fileP = checkFile(filename);
      int userP = checkUser(get_current_user() -> uid.val);
      int newFlag = flags;
      printk(KERN_INFO "Custom open invoked\n");
      printk(KERN_INFO "user_id %d, filename %s\n" , get_current_user() ->uid.val, filename);
      if (fileP == 0) {
        return old_open(filename, flags, mode);
      }
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

static ssize_t device_read(struct file *file, char *buffer, size_t size, loff_t *offset){
    return 0;
}

static loff_t device_llseek(struct file *file, loff_t offset, int orig)
{
    loff_t new_pos = 0;
    switch (orig) {
    case 0:
        new_pos = offset;
        break;
    case 1:
        new_pos = file->f_pos + offset;
        break;
    case 2:
        new_pos = 3000 - offset;
        break;
    }

    if (new_pos > 3000)
        new_pos = 3000;  // max case
    if (new_pos < 0)
        new_pos = 0;        // min case
    file->f_pos = new_pos;  // This is what we'll use now
    return new_pos;
}

static ssize_t device_write(struct file * filename, const char * buf, size_t count, loff_t *offset){
    unsigned long ret;
    if (count >= sizeof(writer))
        return -EINVAL;
    ret = copy_from_user(writer, buf, count);
    if (ret)
        return -EFAULT;
    int i=0;
    int j=0;
    int k;
    printk("%s", writer);
    for (i = 0; i<sizeof(writer); i++) {
        k = 0;
        while(writer[i] != ',') {
            fileNames[j][k] = writer[i];
            i++;
            k++;
        }
        i++;
        fileAccs[j] = writer[i] - '0';
        j++;
        i += 2;
        if (writer[i] == '.'){
            break;
        }
    }
    fileNum = j;
    j = 0;
    for(i; i<sizeof(writer); i++) {
        k = 0;
        while(writer[i] != ',') {
            int charNum = writer[i] - '0';
            userIds[j] = userIds[j]*10 + charNum;
            i++;
            k++;
        }
        i++;
        userAccs[j] = writer[i] - '0';
        j++;
        i+=2;
        if(writer[i] == '.'){
            break;
        }
    }
    userNum = j;
    i = 0;
    for (i; i < fileNum ; i ++) {
        printk(KERN_INFO "file name %s, file access %d \n", fileNames[i], fileAccs[i]);
    }
    printk(KERN_INFO "sys_call_table address: %p\n", sys_call_table);
    disable_write_protection();
    sys_call_table[__NR_open] = custom_open;
    enable_write_protection();

    return count;
}


const struct file_operations open_fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .llseek = device_llseek,
    .write = device_write,
};

static int open_init(void)
{
    int result = 0;
    result = alloc_chrdev_region(&open_device, 0, 1, DEV_NAME);
    sys_call_table = (void *)kallsyms_lookup_name("sys_call_table");
    old_open = sys_call_table[__NR_open];
    
    if (result < 0){
      printk("faild to register pcb character device with error code:%i", result);
    }
    open_cdev = cdev_alloc();
    if (open_cdev == NULL){
      printk(KERN_ALERT "faild to alloc cdev");
      result = -1;
      unregister_chrdev_region(open_device, 1);
      return result;
    }
    cdev_init(open_cdev, &open_fops);
    result = cdev_add(open_cdev, open_device,1);

    if (result < 0){
      printk(KERN_ALERT "faild to add cdev");
      result = -2;
      unregister_chrdev_region(open_device,1);
      return result;
    }
    open_class = class_create(THIS_MODULE, DEV_NAME);
    if (!open_class){
      printk(KERN_ALERT "Faild to create device class");
      result = -3;
      cdev_del(open_cdev);
      unregister_chrdev_region(open_device,1);
      return result;
    }
    if(!device_create(open_class, NULL, open_device, NULL, DEV_NAME)) {
      printk(KERN_ALERT "Faild to create device");
      result = -4;
      class_destroy(open_class);
      cdev_del(open_cdev);
      unregister_chrdev_region(open_device,1);
      return result;
    }
    printk(KERN_INFO "in init \n");
    return result;
}

static void open_exit(void)
{
    pr_info("exit");
    disable_write_protection();
    sys_call_table[__NR_open] = old_open;
    enable_write_protection();
    device_destroy(open_class, open_device);
    class_destroy(open_class);
    cdev_del(open_cdev);
    unregister_chrdev_region(open_device, 1);
    printk(KERN_INFO "custom_open unloaded");
}

module_init(open_init);
module_exit(open_exit);
