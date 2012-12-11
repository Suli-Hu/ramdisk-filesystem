#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)
#define KEYBOARD _IOR(1, 7, struct keyboard_struct)

// Used to get a char from the keyboard using IOCTL
struct keyboard_struct {
    char letter;
};

char my_getchar(int fd, struct keyboard_struct key);

int main () {

  /* IOCTL_TEST STUFF
  // attribute structures
  struct ioctl_test_t {
    int field1;
    char field2;
  } ioctl_test;

  ioctl_test.field1 = 10;
  ioctl_test.field2 = 'a';

  ioctl (fd, IOCTL_TEST, &ioctl_test);
  */

  struct keyboard_struct key;

  int fd = open ("/proc/ioctl_test", O_RDONLY);

  /* This loops until the system detects that the user has pressed the enter key */
  int done = 0;
  char letter;
  while (!done) {
    /* the character is both in key and letter (debugging stuff I dealt with) */
    letter = my_getchar(fd, key);
    
    if (letter == '\n') {
      printf("\nEntered if statement\n");
      done = 1;
    }

    printf("%c",key.letter);
  }
  printf("Exiting");
  fflush(stdin);
  return 0;
}

char my_getchar(int fd, struct keyboard_struct key) {
  ioctl (fd, KEYBOARD, &key);
  return key.letter;
}
