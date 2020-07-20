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
    int sz;
    bool is_reading_files = true, is_first = false;
    int fd = open(pcb_dev, O_RDWR);
    if (fd < 0) {
        perror("failed to open character device\n");
        exit(1);
    }
    char full_str[800];
    lseek(fd, 0, SEEK_SET);
    int i = 1;
    for (i = 1; i < argc ; i += 1) {
        if (strcmp(argv[i], "--files") == 0) {
            is_reading_files = true;
            continue;
        }
        else if (strcmp(argv[i], "--users") == 0) {
            is_reading_files = false;
            strcat(full_str, ".");
            continue;
        }
        if (!is_first) {
            strcpy(full_str, argv[i]);
            is_first = true;
        } else {
            strcat(full_str, argv[i]);
        }
        strcat(full_str, ",");
        i++;
        strcat(full_str, argv[i]);
    }
    strcat(full_str, ".");
    sz = write(fd, full_str, strlen(full_str));
    close(fd);

    return 0;
}
