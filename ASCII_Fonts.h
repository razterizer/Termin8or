//
//  ASCII_Fonts.h
//  Terminal Text Lib
//
//  Created by Rasmus Anthin on 2023-11-15.
//

#pragma once
#include "SpriteHandler.h"
#include "Text.h"

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <filesystem>
#include <unistd.h>


namespace ASCII_Fonts
{

  enum class Font
  {
    Larry3D
  };

  struct ColorScheme
  {
    Text::Color fg_internal = Text::Color::White;
    Text::Color bg_internal = Text::Color::DarkGray;
    Text::Color fg_side_h = Text::Color::White;
    Text::Color bg_side_h = Text::Color::Black;
    Text::Color fg_side_v = Text::Color::LightGray;
    Text::Color bg_side_v = Text::Color::White;
    Text::Color fg_dot_internal = Text::Color::White;
    Text::Color bg_dot_internal = Text::Color::Red;
    Text::Color fg_dot_side_h = Text::Color::White;
    Text::Color bg_dot_side_h = Text::Color::DarkRed;
    Text::Color fg_dot_side_v = Text::Color::LightGray;
    Text::Color bg_dot_side_v = Text::Color::White;
  };

  struct FontPiece
  {
    std::string part;
    int r = 0;
    int c = 0;
    Text::Color fg_color = Text::Color::Black;
    Text::Color bg_color = Text::Color::Transparent;
  };

  struct FontChar
  {
    std::vector<FontPiece> font_pieces;
    int width = 0;
  };
  
  struct FontData
  {
    std::map<char, FontChar> font_chars;
    std::map<std::pair<char, char>, int> kernings;
  };

  using FontDataColl = std::map<Font, FontData>;

  Text::Color get_fg_color(const std::string& col_type, const ColorScheme& colors)
  {
    if (col_type == "T")
      return Text::Color::Transparent;
    if (col_type == "T2")
      return Text::Color::Transparent2;
    if (col_type == "I")
      return colors.fg_internal;
    if (col_type == "SH")
      return colors.fg_side_h;
    if (col_type == "SV")
      return colors.fg_side_v;
    if (col_type == "DI")
      return colors.fg_dot_internal;
    if (col_type == "DSH")
      return colors.fg_dot_side_h;
    if (col_type == "DSV")
      return colors.fg_dot_side_v;
    return Text::Color::Transparent;
  }

  Text::Color get_bg_color(const std::string& col_type, const ColorScheme& colors)
  {
    if (col_type == "T")
      return Text::Color::Transparent;
    if (col_type == "T2")
      return Text::Color::Transparent2;
    if (col_type == "I")
      return colors.bg_internal;
    if (col_type == "SH")
      return colors.bg_side_h;
    if (col_type == "SV")
      return colors.bg_side_v;
    if (col_type == "DI")
      return colors.bg_dot_internal;
    if (col_type == "DSH")
      return colors.bg_dot_side_h;
    if (col_type == "DSV")
      return colors.bg_dot_side_v;
    return Text::Color::Transparent;
  }

  FontDataColl load_font_data(const ColorScheme& colors, const std::string& path_to_font_data)
  {
    FontDataColl font_data;
    
    std::fstream txt_file;
   
    std::string filename = path_to_font_data + "/font_data_larry3d.txt";
    //std::cout << filename << std::endl;
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
      std::cout << "Current working directory: " << cwd << std::endl;
    
    if (std::filesystem::exists(filename) && std::filesystem::is_regular_file(filename))
      std::cout << "File exists!" << std::endl;
    else
      std::cerr << "Error: File does not exist" << std::endl;
    
    txt_file.open(filename, std::ios::in);
    
    if (!txt_file.is_open())
    {
      std::cerr << "Unable to open file." << std::endl;
      txt_file.close();
      return font_data;
    }
    
    // Reset file pointer to the beginning
    txt_file.seekg(0, std::ios::beg);
    if (txt_file.peek() == std::ifstream::traits_type::eof())
    {
      std::cerr << "File is empty." << std::endl;
      txt_file.close();
      return font_data;
    }
    
    auto& curr_font = font_data[Font::Larry3D];
    
    char ch = -1;
    char kch0 = -1;
    char kch1 = -1;
    int kern = 0;
    int width = -1;
    FontPiece piece;
    int r, c;
    char fg[4];
    char bg[4];
    
    if (txt_file.is_open())
    {
      if (txt_file.eof())
        std::cout << "End of file reached." << std::endl;
      if (txt_file.fail())
        std::cerr << "Non-fatal I/O error occurred." << std::endl;
      if (txt_file.bad())
        std::cerr << "Fatal I/O error occurred." << std::endl;
      
      std::string line;
      while (std::getline(txt_file, line))
      {
        sscanf(line.c_str(), "char: '%c', width: %i", &ch, &width);
        if (sscanf(line.c_str(), "kerning: '%c' -> '%c' = %i", &kch0, &kch1, &kern) == 3)
        {
          ch = -1;
          curr_font.kernings[{kch0, kch1}] = kern;
        }
        if (ch != -1)
        {
          auto& curr_char = curr_font.font_chars[ch];
          curr_char.width = width;
          if (line.size() > 0 && line[0] == '"')
          {
            piece.part = line.substr(1, line.size() - 2);
          }
          else if (sscanf(line.c_str(), "%i %i %s %s", &r, &c, fg, bg) == 4)
          {
            piece.r = r;
            piece.c = c;
            piece.fg_color = get_fg_color(fg, colors);
            piece.bg_color = get_bg_color(bg, colors);
            curr_char.font_pieces.emplace_back(piece);
          }
        }
      }
    }
    
    txt_file.close();
    return font_data;
  }

  // (r, c) : top left corner of text.
  // returns the relative start column (top left corner) for the next character.
  template<int NR, int NC>
  int draw_char(SpriteHandler<NR, NC>& sh, const FontDataColl& font_data, const char ch_curr, const char ch_next, int r, int c, Font font)
  {
    auto it_font = font_data.find(font);
    if (it_font != font_data.end())
    {
      const auto& curr_font = it_font->second;
      
      auto it_char = curr_font.font_chars.find(ch_curr);
      if (it_char != curr_font.font_chars.end())
      {
        const auto& curr_char = it_char->second;
        for (const auto& piece : curr_char.font_pieces)
          sh.write_buffer(piece.part, r + piece.r, c + piece.c, piece.fg_color, piece.bg_color);
        
        int kerning = 0;
        if (ch_next != -1)
        {
          auto it_k = curr_font.kernings.find({ch_curr, ch_next});
          if (it_k != curr_font.kernings.end())
            kerning = it_k->second;
        }
        
        return curr_char.width + kerning;
      }
      else
      {
        std::cerr << "Warning: Unrecognized character '" << ch_curr << "'" << std::endl;
      }
    }
    
    return 0;
  }

  // (r, c) : top left corner of text.
  template<int NR, int NC>
  void draw_text(SpriteHandler<NR, NC>& sh, const FontDataColl& font_data, const std::string& text, int r, int c, Font font)
  {
    int width = 0;
    int num_chars = static_cast<int>(text.size());
    for (int ch_idx = 0; ch_idx < num_chars; ++ch_idx)
    {
      char ch_curr = text[ch_idx];
      char ch_next = ch_idx + 1 < num_chars ? text[ch_idx + 1] : -1;
      width += draw_char(sh, font_data, ch_curr, ch_next, r, c + width, font);
    }
  }

}
