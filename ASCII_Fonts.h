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
    Larry3D,
    SMSlant,
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
    int width = -1;
  };
  
  struct FontData
  {
    std::map<char, FontChar> font_chars;
    std::map<std::pair<char, char>, FontChar> font_chars_by_prev_char; // { ch_prev, ch_curr }.
    std::map<std::pair<char, char>, int> kernings;
    std::map<std::pair<char, char>, std::pair<int, int>> orderings;
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
  
  // Add the env variable RUNNING_FROM_XCODE to the Run section of the current scheme:
  //   Product -> Scheme -> Edit Scheme... ->  Run (Debug) -> Arguments -> Environmental Variables -> + -> "RUNNING_FROM_XCODE", true.
  // Perhaps script this procedure in the future.
  std::string get_path_to_font_data()
  {
    std::string font_data_path;
    const char* xcode_env = std::getenv("RUNNING_FROM_XCODE");
    if (xcode_env != nullptr)
      font_data_path = "../../../../../../../../Documents/xcode/lib/Terminal Text Lib/fonts";
    else
      font_data_path = "../../lib/Terminal Text Lib/fonts";
      
    return font_data_path;
  }

  FontDataColl load_font_data(const ColorScheme& colors, const std::string& path_to_font_data)
  {
    FontDataColl font_data;
    
    std::fstream txt_file;
   
    std::vector<std::string> filename_vec;
    filename_vec.emplace_back(path_to_font_data + "/font_data_larry3d.txt");
    filename_vec.emplace_back(path_to_font_data + "/font_data_smslant.txt");
    size_t num_fonts = filename_vec.size();
    //std::cout << filename << std::endl;
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
      std::cout << "Current working directory: " << cwd << std::endl;
    
    for (size_t font_idx = 0; font_idx < num_fonts; ++font_idx)
    {
      const auto& filename = filename_vec[font_idx];
      
      if (std::filesystem::exists(filename) && std::filesystem::is_regular_file(filename))
        std::cout << "File exists!" << std::endl;
      else
      {
        std::cerr << "Error: File does not exist" << std::endl;
        continue;
      }
      
      txt_file.open(filename, std::ios::in);
      
      if (!txt_file.is_open())
      {
        std::cerr << "Unable to open file." << std::endl;
        txt_file.close();
        continue;
      }
      
      // Reset file pointer to the beginning
      txt_file.seekg(0, std::ios::beg);
      if (txt_file.peek() == std::ifstream::traits_type::eof())
      {
        std::cerr << "File is empty." << std::endl;
        txt_file.close();
        continue;
      }
      
      auto& curr_font = font_data[static_cast<Font>(font_idx)];
      
      char ch = -1;
      char ch_prev = -1;
      char ch_next = -1;
      char kch0 = -1;
      char kch1 = -1;
      int kern = 0;
      char och0 = -1;
      char och1 = -1;
      int opri0 = 0;
      int opri1 = 0;
      int width = -1;
      FontPiece piece;
      int r, c;
      char fg[4];
      char bg[4];
      bool kerning = false;
      bool ordering = false;
      
      if (txt_file.is_open())
      {
        if (txt_file.eof())
        {
          std::cout << "End of file reached." << std::endl;
          txt_file.close();
          continue;
        }
        if (txt_file.fail())
        {
          std::cerr << "Non-fatal I/O error occurred." << std::endl;
          txt_file.close();
          continue;
        }
        if (txt_file.bad())
        {
          std::cerr << "Fatal I/O error occurred." << std::endl;
          txt_file.close();
          continue;
        }
      
        std::string line;
        while (std::getline(txt_file, line))
        {
          // Data section.
          if (sscanf(line.c_str(), "char: '%c', width: %i, char_prev: '%c'", &ch, &width, &ch_prev) == 3)
          {
            kerning = false;
            ordering = false;
          }
          else if (sscanf(line.c_str(), "char: '%c', width: %i", &ch, &width) == 2)
          {
            kerning = false;
            ordering = false;
            ch_prev = -1;
          }
          else if (strcmp(line.c_str(), "kerning:") == 0)
          {
            kerning = true;
            ordering = false;
            ch = -1;
            ch_prev = -1;
          }
          else if (strcmp(line.c_str(), "ordering:") == 0)
          {
            kerning = false;
            ordering = true;
            ch = -1;
            ch_prev = -1;
          }
          
          // Read data.
          if (kerning && sscanf(line.c_str(), "'%c' -> '%c' = %i", &kch0, &kch1, &kern) == 3)
            curr_font.kernings[{kch0, kch1}] = kern;
          else if (ordering && sscanf(line.c_str(), "'%c' -> '%c' = %i, %i", &och0, &och1, &opri0, &opri1) == 4)
            curr_font.orderings[{och0, och1}] = {opri0, opri1};
          else if (ch != -1)
          {
            FontChar* curr_char = nullptr;
            if (ch_prev != -1)
              curr_char = &curr_font.font_chars_by_prev_char[{ch_prev, ch}];
            else
              curr_char = &curr_font.font_chars[ch];
              
            if (curr_char != nullptr)
            {
              if (curr_char->width == -1)
                curr_char->width = width;
              if (line.size() > 0 && line[0] == '"')
                piece.part = line.substr(1, line.size() - 2);
              else if (sscanf(line.c_str(), "%i %i %s %s", &r, &c, fg, bg) == 4)
              {
                piece.r = r;
                piece.c = c;
                piece.fg_color = get_fg_color(fg, colors);
                piece.bg_color = get_bg_color(bg, colors);
                curr_char->font_pieces.emplace_back(piece);
              }
            }
          }
        }
      }
      
      txt_file.close();
    }
    return font_data;
  }

  // (r, c) : top left corner of text.
  // returns the relative start column (top left corner) for the next character.
  template<int NR, int NC>
  int draw_char(SpriteHandler<NR, NC>& sh, const FontData& curr_font,
                const char ch_prev, const char ch_curr, const char ch_next,
                int ch_curr_order,
                int r, int c,
                int custom_kerning = 0)
  {
    FontChar const * curr_char = nullptr;
    auto it_char_pc = curr_font.font_chars_by_prev_char.find({ch_prev, ch_curr});
    if (it_char_pc != curr_font.font_chars_by_prev_char.end())
      curr_char = &it_char_pc->second;
    else
    {
      auto it_char = curr_font.font_chars.find(ch_curr);
      if (it_char != curr_font.font_chars.end())
        curr_char = &it_char->second;
    }
      
    if (curr_char != nullptr)
    {
      for (const auto& piece : curr_char->font_pieces)
      {
        OrderedText t;
        t.str = piece.part;
        t.r = r + piece.r;
        t.c = c + piece.c;
        t.fg_color = piece.fg_color;
        t.bg_color = piece.bg_color;
        t.priority = ch_curr_order;
        sh.add_ordered_text(t);
      }
        
      int kerning = 0;
      if (ch_next != -1)
      {
        auto it_k = curr_font.kernings.find({ch_curr, ch_next});
        if (it_k != curr_font.kernings.end())
          kerning = it_k->second + custom_kerning;
      }
      
      return curr_char->width + kerning;
    }
    else
    {
      std::cerr << "Warning: Unrecognized character '" << ch_curr << "'" << std::endl;
    }
    
    return 0;
  }
  
  
  std::vector<std::pair<char, int>> sort_text(const std::string& text, const FontData& curr_font)
  {
    if (text.empty())
      return {};
      
    std::vector<std::pair<char, int>> ordered_text;
    ordered_text.emplace_back(text[0], 0);
    int curr_prio = 0;
    int prev_prio = 0;
    int num_chars = static_cast<int>(text.size());
    for (int ch_idx = 1; ch_idx < num_chars; ++ch_idx)
    {
      char ch_curr = text[ch_idx];
      char ch_prev = text[ch_idx - 1];
      
      auto it_o = curr_font.orderings.find({ ch_prev, ch_curr });
      if (it_o != curr_font.orderings.end())
      {
        auto [o0, o1] = it_o->second;
        if (o0 < o1)
          curr_prio = prev_prio + 1;
        else if (o0 < o1)
          curr_prio = prev_prio - 1;
        else
          curr_prio = prev_prio;
      }
      else
        curr_prio = prev_prio;
        
      ordered_text.emplace_back(ch_curr, curr_prio);
    }
    return ordered_text;
  }
  

  // (r, c) : top left corner of text.
  template<int NR, int NC>
  void draw_text(SpriteHandler<NR, NC>& sh, const FontDataColl& font_data, const std::string& text,
                 int r, int c, Font font, const std::vector<int>& custom_kerning = {})
  {
    int width = 0;
    int num_chars = static_cast<int>(text.size());
    int num_custom_kernings = static_cast<int>(custom_kerning.size());
    auto it_font = font_data.find(font);
    if (it_font != font_data.end())
    {
      const auto& curr_font = it_font->second;
      
      auto ordered_text = sort_text(text, curr_font);
      for (int ch_idx = 0; ch_idx < num_chars; ++ch_idx)
      {
        char ch_prev = ch_idx - 1 >= 0 ? text[ch_idx - 1] : -1;
        char ch_curr = text[ch_idx];
        char ch_next = ch_idx + 1 < num_chars ? text[ch_idx + 1] : -1;
        int ck = 0;
        if (ch_idx < num_custom_kernings)
          ck = custom_kerning[ch_idx];
        int ch_curr_order = ordered_text[ch_idx].second;
        
        width += draw_char(sh, curr_font,
          ch_prev, ch_curr, ch_next,
          ch_curr_order,
          r, c + width,
          ck);
      }
    
      sh.write_buffer_ordered();
    }
  }

}
