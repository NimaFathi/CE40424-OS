#include<stdio.h>
#include <stdlib.h>
#include<string.h>

int main(int argc, char const *argv[]) {
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
        pid = atoi(argv[i+1]);
      } else {
        printf("unavailable argument\n" );
        return 0;
      }
    }
    printf("%d\n", pid);
    printf("%d\n", period);
  }
  return 0;
}
