#pragma once
#include <termios.h>
//#include <ctype.h>
//#include <stdio.h>
//#include <errno.h>
//#include <memory.h>
#include <unistd.h>
//#include <cstdlib>

//https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html

struct termios orig_termios;

void die(const char* s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

void disableRawMode()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode()
{
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  //raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}
