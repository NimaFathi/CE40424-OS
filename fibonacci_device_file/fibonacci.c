#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define LENGTH 120
#define DEV "fibonacci"

MODULE_LICENSE("Dual MIT/GPL");

static dev_t fib_dev = 0;
static struct cdev *fib_cdev;
static struct class *fib_class;

static int fibonacci_sequence(int n)
{
    int fibonacci[n + 2];

    fibonacci[0] = 0;
    fibonacci[1] = 1;

    for (int i = 2; i <= n; i++) {
        fibonacci[i] = fibonacci[i - 1] + fibonacci[i - 2];
    }

    return fibonacci[n];
}

static ssize_t fibonacci_read(struct file *file_ptr, char __user *user_buffer , size_t count, loff_t *position)
{
    printk(KERN_NOTICE "fibonacci_driver: Device file is read at offset %i, read bytes count=%u", (int)*position, (unsigned int) count);
    return (ssize_t) fibonacci_sequence(*position);
}

static loff_t fibonacci_lseek(struct file *file, loff_t offset, int orig)
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
        new_pos = LENGTH - offset;
        break;
    }

    if (new_pos > LENGTH)
        new_pos = LENGTH;
    if (new_pos < 0)
        new_pos = 0;
    file->f_pos = new_pos;
    return new_pos;
}

const struct file_operations fibonacci_fops = {
    .owner = THIS_MODULE,
    .read = fibonacci_read,
    .llseek = fibonacci_lseek,
};

static int __init fibonacci_init(void)
{
   int result = 0 ;
   result = alloc_chrdev_region(&fib_dev, 0, 1, DEV);
   if (result < 0) {
       printk(KERN_ALERT "Failed to register the fibonacci char device with error code = %i",result);
       return result;
   }
   fib_cdev = cdev_alloc();
   if (fib_cdev == NULL) {
       printk(KERN_ALERT "Failed to alloc cdev");
       result = -1;
       unregister_chrdev_region(fib_dev, 1);
       return result;
   }
   cdev_init(fib_cdev, &fibonacci_fops);
   result = cdev_add(fib_cdev, fib_dev, 1);

   if (result < 0) {
       printk(KERN_ALERT "Failed to add cdev");
       result = -2;
       unregister_chrdev_region(fib_dev, 1);
       return result;
   }
   fib_class = class_create(THIS_MODULE, DEV);

   if (!fib_class) {
       printk(KERN_ALERT "Failed to create device class");
       result = -3;
       cdev_del(fib_cdev);
       unregister_chrdev_region(fib_dev,1);
       return result;
   }

   if (!device_create(fib_class, NULL, fib_dev, NULL, DEV)) {
       printk(KERN_ALERT "Failed to create device");
       result = -4;
       class_destroy(fib_class);
       cdev_del(fib_cdev);
       unregister_chrdev_region(fib_dev,1);
       return result;
   }
   return result;
}

static void __exit fibonacci_exit(void)
{
   device_destroy(fib_class, fib_dev);
   class_destroy(fib_class);
   cdev_del(fib_cdev);
   unregister_chrdev_region(fib_dev, 1);
}

module_init(fibonacci_init);
module_exit(fibonacci_exit);
