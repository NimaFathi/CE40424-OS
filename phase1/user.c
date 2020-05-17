#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
static const char pcb_dev[] = "/dev/pcb";

int main(int argc, char const *argv[]) {
  long read_out ;
  int sz;
  char* buf = (char *) calloc(100, sizeof(char));
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
      } else {s
        printf("unavailable argument\n" );
        return 0;
      }
    }
    printf("%d\n",pid);
      printf("%d\n", period);
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
  printf("%s\n",str );

  // write pid into driver file (working)
  lseek(fd, 0, SEEK_SET);
  sz = write(fd, str,strlen(str));
  printf("%d\n",sz);
  free(str);
  close(fd);
  // BUG: read from driver(not working) -> there must be a bug in "pcb_device_file_read" function
  fd = open(pcb_dev, O_RDWR );
  read_out = read(fd , buf,10);
  printf("%ld\n", read_out); //print number of read bytes
  buf[read_out] = '\0';
  printf("%s\n", buf ); // print desired output
  return 0;
}
