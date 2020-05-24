#include <linux/module.h>       // Needed by all modules
#include <linux/kernel.h>       // KERN_INFO
#include <linux/init.h>
#include <linux/sched.h>        // for_each_process, pr_info
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <asm/uaccess.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include <linux/pid.h>
#include <linux/fs_struct.h>
#include <linux/fdtable.h>
#include <linux/dcache.h>

#define MAX_LENGTH 900
#define DEV_NAME "pcb"


MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Nima Fathi");
MODULE_DESCRIPTION("PCB");
MODULE_VERSION("0.1");

static char Message[MAX_LENGTH];
static char *Message_Ptr;
static int num = 0;
static int Device_Open = 0;
typedef struct thisstr {
    char userarray[20];
}localstr;




static dev_t pcb_dev = 0;
static struct cdev *pcb_cdev;
static struct class  *pcb_class;


static int pcb_device_open(struct inode *inode, struct file *file)
{

	printk(KERN_INFO "device_open(%p)\n", file);


	if (Device_Open)
		return -EBUSY;

	Device_Open++;

	Message_Ptr = Message;
	try_module_get(THIS_MODULE);
	return 0;
}

static int pcb_device_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "device_release(%p,%p)\n", inode, file);

	Device_Open--;

	module_put(THIS_MODULE);
	return 0;
}



static char* integer_str(int length ,long myint){
  char *mystr = (char *) kmalloc(length, GFP_KERNEL);
  int i = 0 ;
  while(myint >0 ){
    mystr[i] = '0' + myint % 10;
    myint /= 10;
    i ++;
  }
  return mystr;
}

static char* piderror(void){
  char *error_str = (char*) kmalloc(13, GFP_KERNEL);
  error_str = "pid not valid";
  return error_str;
}

static int integer_len(long state){
  long temp = state;
  int len = 0;
  while(temp > 0){
      len++;
      temp /= 10;
  }
  return len;
}





static ssize_t pcb_device_file_read(struct file *file, char *buffer, size_t size, loff_t *offset){
    int count = *offset;
    char buf[1500];
    pid_t pid = num;
    struct task_struct *tsk;
    tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
    long state = tsk->state;
    int index = 0;
    if (state == -1){
      char * mst = (char *) kmalloc(13, GFP_KERNEL);
      mst = piderror();
      for(int i = 0 ; i < 13; i ++){
          buf[index] = mst[i];
          index ++;
          count ++;
      }
    }
    else{
    int length = integer_len(state);
    char* reverseState = (char *) kmalloc(length, GFP_KERNEL);
    reverseState = integer_str(length, state);
    for(int i = 0 ; i < length ; i++) {
      buf[index] = reverseState[length - i - 1];
      index++;
      count++;
    }
    buf[index] = '-';
    index ++;
    count ++;
    long nvcsw = tsk->nvcsw;
    length = integer_len(nvcsw);
    char* reversenvcsw = (char *) kmalloc(length, GFP_KERNEL);
    reversenvcsw = integer_str(length, nvcsw);
    for(int i = 0 ; i < length ; i++) {
      buf[index] = reversenvcsw[length - i - 1];
      index++;
      count++;
    }
    buf[index] = '-';
    index ++;
    count ++;

    long nivcsw = tsk->nivcsw;
    length = integer_len(nivcsw);
    char* reversenivscw = (char*) kmalloc(length, GFP_KERNEL);
    reversenivscw = integer_str(length, nivcsw);
    for(int i = 0 ; i < length ; i++) {
      buf[index] = reversenivscw[length - i - 1];
      index++;
      count++;
    }
    buf[index] = '\n';
    index ++;
    count ++;

    kfree(reversenvcsw);
    kfree(reverseState);
    kfree(reversenivscw);
    /*
    struct files_struct *current_files;
    struct fdtable *files_table;
    unsigned int *fds;
    int j = 0;
    struct path files_path;
    char *cwd;
    char *buffer_2 = (char *) kmalloc(100 * sizeof(char), GFP_KERNEL);
    current_files = tsk->files;
    files_table = files_fdtable(current_files);
    size_t counter;
    while (files_table->fd[j] != NULL) {
        files_path = files_table->fd[j]->f_path;
        cwd = d_path(&files_path, buffer_2, 100 * sizeof(char));
        counter = 0;

        while (cwd[counter] != '\0') {
            buf[index] = cwd[counter];
            counter++;
            index++;
            count++;
        }
    	buf[index] = '\n';
    	index++;
      j++;
      count++;
    }
    buf[index] = '\0';
    index++;
    count++;
    kfree(buffer_2);*/
  }
    copy_to_user(buffer, buf, count+10);
    return count;
}

static loff_t pcb_device_lseek(struct file *file, loff_t offset, int orig)
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
        new_pos = MAX_LENGTH - offset;
        break;
    }

    if (new_pos > MAX_LENGTH)
        new_pos = MAX_LENGTH;  // max case
    if (new_pos < 0)
        new_pos = 0;        // min case
    file->f_pos = new_pos;  // This is what we'll use now
    return new_pos;
}
static ssize_t pcb_device_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset){
  unsigned long ret;
  localstr mystr;
  int tmp;
  int i;
  if (count > sizeof(mystr.userarray) - 1) // count is actual number of writtern bytes
      return -EINVAL;
  ret = copy_from_user(mystr.userarray, user_buffer, count);
  if (ret)
      return -EFAULT;
  mystr.userarray[count] = '\0';
  for (i = count - 1; i >= 0; i--) {
	   num *= 10;
	   tmp = mystr.userarray[i] - '0';
	   num += tmp;
  }
  printk(KERN_ALERT "pid number %d \n", num);
  return count;
}



const struct file_operations pcb_fops = {
    .owner = THIS_MODULE,
    .read = pcb_device_file_read,
    .llseek = pcb_device_lseek,
    .write = pcb_device_write,
    .open = pcb_device_open,
    .release = pcb_device_release,
};



static int __init init_proc_task(void)
{
        int result = 0;
        result = alloc_chrdev_region(&pcb_dev, 0, 1, DEV_NAME);
        if (result < 0){
          printk("faild to register pcb character device with error code:%i", result);
        }
        pcb_cdev = cdev_alloc();
        if (pcb_cdev == NULL){
          printk(KERN_ALERT "faild to alloc cdev");
          result = -1;
          unregister_chrdev_region(pcb_dev, 1);
          return result;
        }
        cdev_init(pcb_cdev, &pcb_fops);
        result = cdev_add(pcb_cdev, pcb_dev,1);

        if (result < 0){
          printk(KERN_ALERT "faild to add cdev");
          result = -2;
          unregister_chrdev_region(pcb_dev,1);
          return result;
        }
        pcb_class = class_create(THIS_MODULE, DEV_NAME);
        if (!pcb_class){
          printk(KERN_ALERT "Faild to create device class");
          result = -3;
          cdev_del(pcb_cdev);
          unregister_chrdev_region(pcb_dev,1);
          return result;
        }
        if(!device_create(pcb_class, NULL, pcb_dev, NULL, DEV_NAME)) {
          printk(KERN_ALERT "Faild to create device");
          result = -4;
          class_destroy(pcb_class);
          cdev_del(pcb_cdev);
          unregister_chrdev_region(pcb_dev,1);
          return result;
        }
        return result;
}

static void __exit exit_proc_task(void)
{
  device_destroy(pcb_class, pcb_dev);
  class_destroy(pcb_class);
  cdev_del(pcb_cdev);
  unregister_chrdev_region(pcb_dev, 1);

}


module_init(init_proc_task);
module_exit(exit_proc_task);
