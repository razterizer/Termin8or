#pragma once
#include <Core/StringHelper.h>
#ifdef _WIN32
#define NOMINMAX // Should fix the std::min()/max() and std::numeric_limits<T>::min()/max() compilation problems.
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifndef _WIN32
#include <unistd.h>
#endif


namespace keyboard
{

#ifdef _WIN32
  HANDLE hStdin;
  DWORD fdwSaveOldMode;
#else
  //https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
  struct termios orig_termios;
#endif
  static bool m_raw_mode = false;
  
  void die(const char* s)
  {
    //perror(s);
    std::cerr << s << "\n";
    exit(EXIT_FAILURE);
  }
  
  void disableRawMode()
  {
    m_raw_mode = false;
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
    m_raw_mode = true;
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
    if (!m_raw_mode)
      die("You need to enable raw mode for readKeystroke() to work properly!");
    
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
  
  char waitKeystroke()
  {
    if (!m_raw_mode)
      die("You need to enable raw mode for waitKeystroke() to work properly!");
    
#ifdef _WIN32
    // This works better on Windows as the lin/mac code below
    //  seems to sometimes cause Windows to refocus on another window after
    //  a key has been pressed.
    return _getch();
#else
    // #FIXME: Find a way on lin/mac that doesn't require raw-mode
    //  (and thus readKeystroke()).
    char ch = 0;
    while (ch == 0)
      ch = readKeystroke();
    return ch;
#endif
  }
  
  void pressAnyKey(const std::string_view sv_msg = "Press any key to continue...")
  {
    std::cout << sv_msg << "\n";
    waitKeystroke();
  }
  
  enum class SpecialKey { None, Left, Right, Down, Up, Enter };
  int arrow_key_ctr = 0;
  int key_ctr = 0;
  struct KeyPressData
  {
    char curr_key = 0;
    SpecialKey curr_special_key = SpecialKey::None;
    std::array<SpecialKey, 3> arrow_key_buffer;
    bool pause = false;
    bool quit = false;
    
    SpecialKey get_buffered_arrow_key() const
    {
      auto it_begin = std::begin(arrow_key_buffer);
      auto it_end = std::end(arrow_key_buffer);
      if (std::all_of(it_begin, it_end, [](auto v) { return v == SpecialKey::None; }))
        return SpecialKey::None;
      if (std::any_of(it_begin, it_end, [](auto v) { return v == SpecialKey::Left; }))
        return SpecialKey::Left;
      if (std::any_of(it_begin, it_end, [](auto v) { return v == SpecialKey::Right; }))
        return SpecialKey::Right;
      if (std::any_of(it_begin, it_end, [](auto v) { return v == SpecialKey::Down; }))
        return SpecialKey::Down;
      if (std::any_of(it_begin, it_end, [](auto v) { return v == SpecialKey::Up; }))
        return SpecialKey::Up;
      return SpecialKey::None;
    }
  };
  
  KeyPressData register_keypresses(bool use_wasd_arrow_keys)
  {
    KeyPressData kpd;
  
    char ch = readKeystroke();
    if (key_ctr == 0 && ch == 0x1B)
      key_ctr++;
    else if (key_ctr == 1 && ch == 0x5B)
      key_ctr++;
    else if (key_ctr == 2)
    {
      key_ctr = 0;
      switch (ch)
      {
        case 0x44: kpd.curr_special_key = SpecialKey::Left; break;
        case 0x43: kpd.curr_special_key = SpecialKey::Right; break;
        case 0x42: kpd.curr_special_key = SpecialKey::Down; break;
        case 0x41: kpd.curr_special_key = SpecialKey::Up; break;
        default: break;
      }
      kpd.arrow_key_buffer[arrow_key_ctr % 3] = kpd.curr_special_key;
      arrow_key_ctr++;
    }
    else
    {
      kpd.curr_key = ch;
      if (use_wasd_arrow_keys && str::to_lower(ch) == 'a')
      {
        kpd.curr_special_key = SpecialKey::Left;
        kpd.arrow_key_buffer[arrow_key_ctr % 3] = SpecialKey::Left;
        arrow_key_ctr++;
      }
      else if (use_wasd_arrow_keys && str::to_lower(ch) == 'd')
      {
        kpd.curr_special_key = SpecialKey::Right;
        kpd.arrow_key_buffer[arrow_key_ctr % 3] = SpecialKey::Right;
        arrow_key_ctr++;
      }
      else if (use_wasd_arrow_keys && str::to_lower(ch) == 's')
      {
        kpd.curr_special_key = SpecialKey::Down;
        kpd.arrow_key_buffer[arrow_key_ctr % 3] = SpecialKey::Down;
        arrow_key_ctr++;
      }
      else if (use_wasd_arrow_keys && str::to_lower(ch) == 'w')
      {
        kpd.curr_special_key = SpecialKey::Up;
        kpd.arrow_key_buffer[arrow_key_ctr % 3] = SpecialKey::Up;
        arrow_key_ctr++;
      }
      else if (str::to_lower(ch) == 'p')
        kpd.pause = true;
      else if (str::to_lower(ch) == 'q')
        kpd.quit = true;
      else if (ch == 13)
        kpd.curr_special_key = SpecialKey::Enter;
      else if (ch == -1 || ch == 0)
      {
        kpd.arrow_key_buffer[arrow_key_ctr % 3] = SpecialKey::None;
        arrow_key_ctr++;
      }
    }
    return kpd;
  }
  
}
