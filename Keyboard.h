#pragma once
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <optional>
#include <variant>
#include <iostream>

namespace keyboard
{
  enum class SpecialKey
  {
    None,
    Left,
    Right,
    Down,
    Up,
    Enter,
    Tab,
    Backspace
  };
  
  using KeyPressData = std::optional<std::variant<SpecialKey, char>>;
  
  SpecialKey get_special_key(const KeyPressData& kpd)
  {
    if (std::holds_alternative<SpecialKey>(*kpd))
      return std::get<SpecialKey>(*kpd);
    return SpecialKey::None;
  }
  
  char get_char_key(const KeyPressData& kpd)
  {
    if (std::holds_alternative<char>(*kpd))
      return std::get<char>(*kpd);
    return 0;
  }
  
  std::string special_key_to_string(SpecialKey special_key)
  {
    switch (special_key)
    {
      case SpecialKey::None: return "None";
      case SpecialKey::Left: return "Left";
      case SpecialKey::Right: return "Right";
      case SpecialKey::Down: return "Down";
      case SpecialKey::Up: return "Up";
      case SpecialKey::Enter: return "Enter";
      case SpecialKey::Tab: return "Tab";
      case SpecialKey::Backspace: return "Backspace";
    }
    return "N/A";
  }
  
  
  
  class StreamKeyboard
  {
  public:
    StreamKeyboard()
    {
      enableRawMode();
    }
    
    ~StreamKeyboard()
    {
      disableRawMode();
    }
    
    // Reads a key and returns either a SpecialKey or a regular character.
    KeyPressData readKey()
    {
#ifdef _WIN32
      INPUT_RECORD irInBuf[1];
      DWORD cNumRead;
      
      while (true)
      {
        ReadConsoleInput(hStdin, irInBuf, 1, &cNumRead);
        
        if (irInBuf[0].EventType == KEY_EVENT && irInBuf[0].Event.KeyEvent.bKeyDown)
        {
          WORD virtualKeyCode = irInBuf[0].Event.KeyEvent.wVirtualKeyCode;
          char ch = irInBuf[0].Event.KeyEvent.uChar.AsciiChar;
          
          if (virtualKeyCode == VK_LEFT) return SpecialKey::Left;
          if (virtualKeyCode == VK_RIGHT) return SpecialKey::Right;
          if (virtualKeyCode == VK_UP) return SpecialKey::Up;
          if (virtualKeyCode == VK_DOWN) return SpecialKey::Down;
          if (virtualKeyCode == VK_RETURN) return SpecialKey::Enter;
          if (virtualKeyCode == VK_TAB) return SpecialKey::Tab;
          if (virtualKeyCode == VK_BACK) return SpecialKey::Backspace;
          
          if (ch >= 32 && ch <= 126) // Printable ASCII characters
            return ch;
          
          return SpecialKey::None;
        }
      }
#else
      unsigned char c;
      if (read(STDIN_FILENO, &c, 1) == -1) return std::nullopt;
      
      if (c == 27)
      { // Escape sequence
        if (read(STDIN_FILENO, &c, 1) == -1) return std::nullopt;
        if (c == '[')
        {
          if (read(STDIN_FILENO, &c, 1) == -1) return std::nullopt;
          switch (c)
          {
            case 'A': return SpecialKey::Up;
            case 'B': return SpecialKey::Down;
            case 'C': return SpecialKey::Right;
            case 'D': return SpecialKey::Left;
          }
        }
      }
      else
      {
        switch (c)
        {
          case 10:
          case 13:
            return SpecialKey::Enter;
          case 9:
            return SpecialKey::Tab;
          case 8:
          case 127: // 127 for backspace on MacOS.
            return SpecialKey::Backspace;
          default:
            if (c >= 32 && c <= 126) // Printable ASCII characters
              return static_cast<char>(c);
            return SpecialKey::None;
        }
      }
      return SpecialKey::None;
#endif
    }
    
    KeyPressData waitKey()
    {
#ifdef _WIN32
      // This works better on Windows as the lin/mac code below
      //  seems to sometimes cause Windows to refocus on another window after
      //  a key has been pressed.
      return _getch();
#else
      // #FIXME: Find a way on lin/mac that doesn't require raw-mode
      //  (and thus readKeystroke()).
      KeyPressData key = std::nullopt;
      while (!key.has_value())
        key = readKey();
      return key;
#endif
    }
    
    void pressAnyKey(const std::string_view sv_msg = "Press any key to continue...")
    {
      std::cout << sv_msg << "\n";
      waitKey();
    }
    
  private:
    void disableRawMode()
    {
#ifdef _WIN32
      // Restore the original console mode.
      if (!SetConsoleMode(hStdin, fdwSaveOldMode))
      {
        std::cerr << "Error in SetConsoleMode()!" << std::endl;
        return;
      }
#else
      if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
      {
        std::cerr << "Error in tcsetattr()!" << std::endl;
        return;
      }
#endif
    }
    
    void enableRawMode()
    {
#ifdef _WIN32
      // Get the handle to the input buffer.
      hStdin = GetStdHandle(STD_INPUT_HANDLE);
      if (hStdin == INVALID_HANDLE_VALUE)
      {
        std::cerr << "Error in GetStdHandle()!" << std::endl;
        exit(EXIT_FAILURE);
      }
      
      // Save the current input mode.
      if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
      {
        std::cerr << "Error in GetConsoleMode()!" << std::endl;
        exit(EXIT_FAILURE);
      }
      
      // Modify the input mode to enable raw mode.
      DWORD fdwMode = fdwSaveOldMode;
      fdwMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
      if (!SetConsoleMode(hStdin, fdwMode))
      {
        std::cerr << "Error in SetConsoleMode()!" << std::endl;
        exit(EXIT_FAILURE);
      }
      
      // Perform any additional configuration needed for raw mode in Windows.
      // ...
#else
      if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
      {
        std::cerr << "Error in tcgetattr()!" << std::endl;
        exit(EXIT_FAILURE);
      }
      
      struct termios raw = orig_termios;
      raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
      //raw.c_oflag &= ~(OPOST);
      raw.c_cflag |= (CS8);
      raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
      raw.c_cc[VMIN] = 0;
      raw.c_cc[VTIME] = 1;
      
      if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
      {
        std::cerr << "Error in tcsetattr()!" << std::endl;
        exit(EXIT_FAILURE);
      }
#endif
    }
    
#ifdef _WIN32
    HANDLE hStdin { nullptr };
    DWORD fdwSaveOldMode { 0 };
#else
    struct termios orig_termios;
#endif
  };
  
}
