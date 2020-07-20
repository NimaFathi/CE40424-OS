#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

static const char pcb_dev[] = "/dev/custom_open";

int main(int argc, char const *argv[]) {
    long read_out ;
    int sz;
    bool is_reading_files = true;
    int fd = open(pcb_dev, O_RDWR);
    if (fd < 0) {
        perror("faild to open character device\n");
        exit(1);
    }
    lseek(fd, 0, SEEK_SET);
    int i = 1;
    for (i = 1; i < argc ; i += 1) {
        if (strcmp(argv[i], "--files") == 0) {
            is_reading_files = true;
            continue;
        }
        else if (strcmp(argv[i], "--users") == 0) {
            is_reading_files = false;
            sz = write(fd, ".", 1);
            lseek(fd, sz, SEEK_CUR);
            continue;
        }
        sz = write(fd, argv[i],strlen(argv[i]));
        lseek(fd, sz, SEEK_CUR);
        sz = write(fd, ",", 1);
        lseek(fd, sz, SEEK_CUR);
        i++;
        sz = write(fd, argv[i], strlen(argv[i]));
        lseek(fd, sz, SEEK_CUR);
    }
    close(fd);

    return 0;
}
