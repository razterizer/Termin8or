#pragma once
#ifdef _WIN32
#define NOMINMAX // Should fix the std::min()/max() and std::numeric_limits<T>::min()/max() compilation problems
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <optional>
#include <variant>
#include <iostream>
#include <Core/Delay.h>


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
    Backspace,
    Escape
  };
  
  using KeyPressData = std::optional<std::variant<SpecialKey, char>>;
  
  SpecialKey get_special_key(const KeyPressData& kpd)
  {
    if (!kpd.has_value())
      return SpecialKey::None;
    if (std::holds_alternative<SpecialKey>(*kpd))
      return std::get<SpecialKey>(*kpd);
    return SpecialKey::None;
  }
  
  char get_char_key(const KeyPressData& kpd)
  {
    if (!kpd.has_value())
      return 0;
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
      case SpecialKey::Escape: return "Escape";
    }
    return "N/A";
  }
  
  SpecialKey string_to_special_key(const std::string& special_key_str)
  {
    if (special_key_str == "None")
      return SpecialKey::None;
    else if (special_key_str == "Left")
      return SpecialKey::Left;
    else if (special_key_str == "Right")
      return SpecialKey::Right;
    else if (special_key_str == "Down")
      return SpecialKey::Down;
    else if (special_key_str == "Up")
      return SpecialKey::Up;
    else if (special_key_str == "Enter")
      return SpecialKey::Enter;
    else if (special_key_str == "Tab")
      return SpecialKey::Tab;
    else if (special_key_str == "Backspace")
      return SpecialKey::Backspace;
    else if (special_key_str == "Escape")
      return SpecialKey::Escape;
    return SpecialKey::None;
  }
  
  bool is_key_pressed(const KeyPressData& kpd)
  {
    auto ch = get_char_key(kpd);
    if (ch > 0)
      return true;
    auto key = get_special_key(kpd);
    if (key == SpecialKey::None)
      return false;
    return key != SpecialKey::None;
  }
  
  std::string get_key_description(const KeyPressData& kpd)
  {
    auto ch = get_char_key(kpd);
    if (ch > 0)
      return std::string(1, ch);
    auto key = get_special_key(kpd);
    if (key == SpecialKey::None)
      return "";
    return special_key_to_string(key);
  }
  
  struct KeyPressDataPair
  {
    KeyPressData transient = std::nullopt;
    KeyPressData held = std::nullopt;
  };
  
  
  
  class StreamKeyboard
  {
  public:
    StreamKeyboard(int buf_size = 10)
    {
      key_press_buffer.resize(buf_size);
      enableRawMode();
    }
    
    ~StreamKeyboard()
    {
      disableRawMode();
    }
    
    // Reads a key and returns either a SpecialKey or a regular character.
    KeyPressDataPair readKey()
    {
      KeyPressDataPair kpdp;
      auto kpd = parseKey();
      kpdp.transient = kpd;
      
      key_press_buffer[buffer_idx++] = kpd;
      if (buffer_idx >= static_cast<int>(key_press_buffer.size()))
        buffer_idx = 0;
      if (is_key_pressed(kpd))
      {
        kpdp.held = kpd;
        return kpdp;
      }
        
      for (const auto& buf_kpd : key_press_buffer)
      {
        if (char ch = get_char_key(buf_kpd); ch != 0)
        {
          kpdp.held = ch;
          return kpdp;
        }
        if (SpecialKey key = get_special_key(buf_kpd); key != SpecialKey::None)
        {
          kpdp.held = key;
          return kpdp;
        }
      }
      kpdp.held = SpecialKey::None;
      return kpdp;
    }
    
    KeyPressData waitKey()
    {
      KeyPressDataPair kpdp;
      char char_key = 0;
      SpecialKey special_key = SpecialKey::None;
      do
      {
        kpdp = readKey();
        char_key = get_char_key(kpdp.transient);
        special_key = get_special_key(kpdp.transient);
      } while (char_key == 0 && special_key == SpecialKey::None);
      return kpdp.transient;
    }
    
    void pressAnyKey(const std::string_view sv_msg = "Press any key to continue...")
    {
      std::cout << sv_msg << "\n";
      waitKey();
    }
    
    void set_held_buffer_size_from_fps(float fps)
    {
      auto buf_size = static_cast<int>(std::max(5.f, -4.4893e-05f*math::sq(fps) + 1.9383e-01f*fps + 1.0662e+00f));
      //std::cout << "fps = " << fps << ", buf_size = " << buf_size << std::endl;
      key_press_buffer.resize(buf_size);
    }
    
  private:
    void disableRawMode()
    {
#ifdef _WIN32
      // Restore the original console mode.
      if (!SetConsoleMode(hStdin, fdwSaveOldMode))
      {
        std::cerr << "Error in SetConsoleMode()!" << std::endl;
        exit(EXIT_FAILURE);
      }
#else
      if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
      {
        std::cerr << "Error in tcsetattr()!" << std::endl;
        exit(EXIT_FAILURE);
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
    
    KeyPressData parseKey()
    {
#ifdef _WIN32
      if (_kbhit())
      {
        int ch = _getch();
        switch (ch)
        {
          case 224: // Special keys
            ch = _getch();
            switch (ch)
            {
              case 75: return SpecialKey::Left;
              case 77: return SpecialKey::Right;
              case 72: return SpecialKey::Up;
              case 80: return SpecialKey::Down;
            }
            break;
          case 27: return SpecialKey::Escape;
          case 13: return SpecialKey::Enter;
          case 9: return SpecialKey::Tab;
          case 8: return SpecialKey::Backspace;
          default:
            if (ch >= 32 && ch <= 126) // Printable ASCII characters
              return static_cast<char>(ch);
            return SpecialKey::None;
        }
      }
      return std::nullopt;
#else
      unsigned char c;
      fd_set read_fds;
      struct timeval tv = {0, 100}; // 100 microseconds to avoid CPU overload
      
      FD_ZERO(&read_fds);
      FD_SET(STDIN_FILENO, &read_fds);
      
      int result = select(STDIN_FILENO + 1, &read_fds, nullptr, nullptr, &tv);
      if (result > 0 && FD_ISSET(STDIN_FILENO, &read_fds))
      {
        if (read(STDIN_FILENO, &c, 1) == -1) return std::nullopt;
        
        // Process escape sequences and special keys
        if (c == 27)
        {
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
          return SpecialKey::Escape;
        }
        else
        {
          // Handle Enter, Tab, Backspace, and printable ASCII characters
          switch (c)
          {
            case 10:
            case 13: return SpecialKey::Enter;
            case 9: return SpecialKey::Tab;
            case 8:
            case 127: return SpecialKey::Backspace;
            default:
              if (c >= 32 && c <= 126)
                return static_cast<char>(c);
              return SpecialKey::None;
          }
        }
      }
      return std::nullopt;
#endif
    }
    
#ifdef _WIN32
    HANDLE hStdin { nullptr };
    DWORD fdwSaveOldMode { 0 };
#else
    struct termios orig_termios;
#endif
    std::vector<KeyPressData> key_press_buffer;
    int buffer_idx = 0;
  };
  
}
