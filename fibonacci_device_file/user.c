
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static const char fib_dev[] = "/dev/fibonacci";

int main()
{
   long read_out;
   long myarray[30];
   int number = 0 ;
   char buf[1];
   printf("please insert a number < 20 \n");
   scanf("%d", &number );
   int fd = open(fib_dev, O_RDWR);
   if (fd < 0) {
       perror("Failed to open character device\n");
       exit(1);
   }

   for (int i = 0; i <= number; i++) {
       lseek(fd, i, SEEK_SET);
       read_out = read(fd, buf, 1);
       myarray[i] = read_out;
   }

   close(fd);
   // print out sequence
   for (int i = 0 ; i <= number ; i ++) {
     printf("%ld ", myarray[i]);
   }
   return 0;
}
