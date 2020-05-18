#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

static const char pcb_dev[] = "/dev/pcb";

int main(int argc, char const *argv[]) {
  long read_out ;
  int sz;
  int pid = 0;
  int period = 0;
  if (argc < 5) {
    printf("not enough arguments \n" );
    return 0;
  } else if (argc > 5) {
    printf("to many arguments \n" );
    return 0;
  } else {
    int i = 1;
    for (i = 1; i < 4 ; i += 2) {
      if (strcmp(argv[i], "--period") == 0) {
        period = atoi(argv[i+1]);
      }
      else if (strcmp(argv[i], "--pid") == 0) {
        pid = (pid_t) atoi(argv[i+1]);
      } else {
        printf("unavailable argument\n" );
        return 0;
      }
    }
  }
  int fd = open(pcb_dev, O_RDWR );
  if (fd < 0) {
    perror("faild to open character device\n");
    exit(1);
  }
  // convert int to string
  int length = snprintf(NULL, 0 , "%d", pid);
  char * str = malloc(length + 1);
  snprintf(str, length + 1 , "%d", pid);

  // write pid into driver file (working)
  lseek(fd, 0, SEEK_SET);
  sz = write(fd, str,strlen(str));
  printf("# written bytes in driver: %d\n",sz);
  while(1){
    char buffer[1000];
    sleep(period);
    lseek(fd, strlen(buffer) + sz , SEEK_SET);
    read_out = read(fd , buffer,1000);
    printf("# read bytes: %ld\n", read_out); //print number of read bytes
    buffer[read_out] = '\0';
    printf("%s\n", buffer);
  }

  return 0;
}
