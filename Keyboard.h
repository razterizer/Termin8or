#pragma once
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif
//#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
//#include <memory.h>
#ifndef _WIN32
#include <unistd.h>
#endif
//#include <cstdlib>

#ifdef _WIN32
HANDLE hStdin;
DWORD fdwSaveOldMode;
#else
//https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html

struct termios orig_termios;
#endif

void die(const char* s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

void disableRawMode()
{
#ifdef _WIN32
  // Restore the original console mode.
  if (!SetConsoleMode(hStdin, fdwSaveOldMode))
    die("SetConsoleMode");
#else
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
#endif
}

void enableRawMode()
{
#ifdef _WIN32
  // Get the handle to the input buffer.
  hStdin = GetStdHandle(STD_INPUT_HANDLE);
  if (hStdin == INVALID_HANDLE_VALUE)
    die("GetStdHandle");

  // Save the current input mode.
  if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
    die("GetConsoleMode");

  // Modify the input mode to enable raw mode.
  DWORD fdwMode = fdwSaveOldMode;
  fdwMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
  if (!SetConsoleMode(hStdin, fdwMode))
    die("SetConsoleMode");

  // Perform any additional configuration needed for raw mode in Windows.
  // ...
#else
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
#endif
}

char readKeystroke()
{
#ifdef _WIN32
  INPUT_RECORD irInput;
  DWORD cNumRead;

  if (!PeekConsoleInput(hStdin, &irInput, 1, &cNumRead))
  {
    die("PeekConsoleInput");
  }

  if (cNumRead)
  {
    if (ReadConsoleInput(hStdin, &irInput, 1, &cNumRead))
    {
      if (irInput.EventType == KEY_EVENT && irInput.Event.KeyEvent.bKeyDown)
      {
        return irInput.Event.KeyEvent.uChar.AsciiChar;
      }
    }
    else
    {
      die("ReadConsoleInput");
    }
  }

  return '\0'; // Return null character if no key is pressed
#else
  char ch = '\0';
  if (read(STDIN_FILENO, &ch, 1) == -1 && errno != EAGAIN)
    die("read");
  return ch;
#endif
}
