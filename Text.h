#pragma once
#include <string>
#include <vector>
#include <iostream>

// Color                    | Foreground | Background
//--------------------------+------------+-------------
// Default                  | \033[39m   | \033[49m
// Black                    | \033[30m   | \033[40m
// Dark red                 | \033[31m   | \033[41m
// Dark green               | \033[32m   | \033[42m
// Dark yellow (Orange-ish) | \033[33m   | \033[43m
// Dark blue                | \033[34m   | \033[44m
// Dark magenta             | \033[35m   | \033[45m
// Dark cyan                | \033[36m   | \033[46m
// Light gray               | \033[37m   | \033[47m
// Dark gray                | \033[90m   | \033[100m
// Red                      | \033[91m   | \033[101m
// Green                    | \033[92m   | \033[102m
// Yellow                   | \033[93m   | \033[103m
// Blue                     | \033[94m   | \033[104m
// Magenta                  | \033[95m   | \033[105m
// Cyan                     | \033[96m   | \033[106m
// White                    | \033[97m   | \033[107m
//
// Reset : \033[0m

class Text
{

  template<typename EnumType>
  constexpr int to_int(EnumType e) const noexcept
  {
    return static_cast<int>(e);
  }

public:
  Text()
  {
    std::ios_base::sync_with_stdio(false);
  }

  enum class Color
  {
    Transparent = -1,
    Transparent2 = -2,
    Default = 0,
    Black,
    DarkRed,
    DarkGreen,
    DarkYellow,
    DarkBlue,
    DarkMagenta,
    DarkCyan,
    LightGray,
    DarkGray,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
  };

  std::string get_color_string(Color text_color, Color bg_color = Color::Default) const
  {
    if (bg_color == Color::Transparent || bg_color == Color::Transparent2)
      bg_color = Color::Default;
    std::string fg = "\033[";
    std::string bg = "\033[";
    if (text_color == Color::Default)
      fg += "39";
    else if (Color::Default < text_color && text_color <= Color::LightGray)
      fg += std::to_string(to_int(text_color) + 29);
    else
      fg += std::to_string(to_int(text_color) + 81);

    if (bg_color == Color::Default)
      bg += "49";
    else if (Color::Default < bg_color && bg_color <= Color::LightGray)
      bg += std::to_string(to_int(bg_color) + 39);
    else
      bg += std::to_string(to_int(bg_color) + 91);

    fg += "m";
    bg += "m";
    return fg + bg;
  }

  void print(const std::string& text, Color text_color, Color bg_color = Color::Default) const
  {
    std::string output = get_color_string(text_color, bg_color) + text + "\033[0m";
    //printf("%s", output.c_str());
    std::cout << output;
  }

  void print_line(const std::string& text, Color text_color, Color bg_color = Color::Default) const
  {
    print(text, text_color, bg_color);
    //printf("\n");
    std::cout << "\n";
  }

  void print_char(char c, Color text_color, Color bg_color = Color::Default) const
  {
    std::string output = get_color_string(text_color, bg_color) + c;
    //printf("%s", output.c_str());
    std::cout << output;
  }

  void print_complex(const std::vector<std::tuple<char, Color, Color>>& text)
  {
    size_t n = text.size();
    std::string output;
    for (size_t i = 0; i < n; ++i)
    {
      const auto& ti = text[i];
      char c = std::get<0>(ti);
      auto fg_color = std::get<1>(ti);
      auto bg_color = std::get<2>(ti);
      auto col_str = get_color_string(fg_color, c == '\n' ? Color::Default : bg_color);
      std::string char_str(1, c);
      output += col_str + char_str;
    }
    output += "\033[0m";
    //printf("%s", output.c_str());
    std::cout << output;
  }

  void print_reset() const
  {
    //printf("%s", "\033[0m");
    std::cout << "\033[0m";
  }
};

