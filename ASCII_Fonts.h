//
//  ASCII_Fonts.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2023-11-15.
//

#pragma once
#include "SpriteHandler.h"
#include "Styles.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <filesystem>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <Core/Utils.h>
#include <Core/FolderHelper.h>

//#define DEBUG_PRINT


namespace ASCII_Fonts
{

  enum class Font
  {
    Larry3D,
    SMSlant,
    Avatar,
  };

  struct ColorScheme
  {
    styles::Style internal { Color::White, Color::DarkGray };
    styles::Style side_h { Color::White, Color::Black };
    styles::Style side_v { Color::LightGray, Color::White };
    styles::Style dot_internal { Color::White, Color::Red };
    styles::Style dot_side_h { Color::White, Color::DarkRed };
    styles::Style dot_side_v { Color::LightGray, Color::White };
  };

  struct FontPiece
  {
    std::string part;
    int r = 0;
    int c = 0;
    std::string fg, bg;
    int prio = 0;
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
    bool treat_lower_case_as_upper_case = false;
  };

  using FontDataColl = std::map<Font, FontData>;

  Color get_fg_color(const std::string& col_type, const ColorScheme& colors)
  {
    if (col_type == "T")
      return Color::Transparent;
    if (col_type == "T2")
      return Color::Transparent2;
    if (col_type == "I")
      return colors.internal.fg_color;
    if (col_type == "SH")
      return colors.side_h.fg_color;
    if (col_type == "SV")
      return colors.side_v.fg_color;
    if (col_type == "DI")
      return colors.dot_internal.fg_color;
    if (col_type == "DSH")
      return colors.dot_side_h.fg_color;
    if (col_type == "DSV")
      return colors.dot_side_v.fg_color;
    return Color::Transparent;
  }

  Color get_bg_color(const std::string& col_type, const ColorScheme& colors)
  {
    if (col_type == "T")
      return Color::Transparent;
    if (col_type == "T2")
      return Color::Transparent2;
    if (col_type == "I")
      return colors.internal.bg_color;
    if (col_type == "SH")
      return colors.side_h.bg_color;
    if (col_type == "SV")
      return colors.side_v.bg_color;
    if (col_type == "DI")
      return colors.dot_internal.bg_color;
    if (col_type == "DSH")
      return colors.dot_side_h.bg_color;
    if (col_type == "DSV")
      return colors.dot_side_v.bg_color;
    return Color::Transparent;
  }
  
  // Add the env variable RUNNING_FROM_XCODE to the Run section of the current scheme:
  //   Product -> Scheme -> Edit Scheme... ->  Run (Debug) -> Arguments -> Environmental Variables -> + -> "RUNNING_FROM_XCODE", true.
  // Perhaps script this procedure in the future.
  std::string get_path_to_font_data(const std::string& exe_folder)
  {
#ifdef _WIN32
    // Assume the font files are copied via post-build script to local bin / target folder.
    // E.g. xcopy $(SolutionDir)\..\..\lib\Termin8or\fonts\* $(TargetDir)\fonts\ / Y
    return folder::get_exe_dir() + "\\fonts\\";
#else
    std::string font_data_path;
    const char* xcode_env = std::getenv("RUNNING_FROM_XCODE");
    if (xcode_env != nullptr)
      font_data_path = "../../../../../../../../Documents/xcode/lib/Termin8or/fonts";
    else
    {
      //font_data_path = "../../lib/Termin8or/fonts";
      font_data_path = folder::join_path({ exe_folder, "fonts" });
    }
      
    return font_data_path;
#endif
  }

  FontDataColl load_font_data(const std::string& path_to_font_data)
  {
    FontDataColl font_data;
    
    std::fstream txt_file;
   
    std::vector<std::string> filename_vec;
    filename_vec.emplace_back(path_to_font_data + "/font_data_larry3d.txt");
    filename_vec.emplace_back(path_to_font_data + "/font_data_smslant.txt");
    filename_vec.emplace_back(path_to_font_data + "/font_data_avatar.txt");
    size_t num_fonts = filename_vec.size();
    //std::cout << filename << std::endl;
#ifdef DEBUG_PRINT
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
      std::cout << "Current working directory: " << cwd << std::endl;
#endif
    
    for (size_t font_idx = 0; font_idx < num_fonts; ++font_idx)
    {
      const auto& filename = filename_vec[font_idx];
      
      if (std::filesystem::exists(filename) && std::filesystem::is_regular_file(filename))
      {
#ifdef DEBUG_PRINT
        std::cout << "File exists!" << std::endl;
#endif
      }
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
      std::vector<char> ch_prev_vec;
      char kch0 = -1;
      char kch1 = -1;
      int kern = 0;
      char och0 = -1;
      char och1 = -1;
      int opri0 = 0;
      int opri1 = 0;
      int width = -1;
      FontPiece piece;
      bool kerning = false;
      bool ordering = false;
      bool treat_lower_case_as_upper_case = false;
      
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
        
        const std::vector<char> delim { ':', ',', ' ' };
        const std::vector<char> scope_delim { '\'' };
      
        std::string line;
        while (std::getline(txt_file, line))
        {
          // Data section.
          if (line.starts_with("treat_lower_case_as_upper_case"))
          {
            auto tokens = str::tokenize(line, { ' ', '=' });
            if (tokens.size() != 2)
              std::cerr << "Error in ASCII_Fonts::load_font_data() : Expected two tokens, not " << tokens.size() << "!" << std::endl;
            if (str::to_lower(tokens[1]) == "true")
              treat_lower_case_as_upper_case = true;
          }
          else if (line.starts_with("char:"))
          {
            kerning = false;
            ordering = false;
            ch_prev = -1;
            ch_prev_vec.clear();
            
            auto tokens = str::tokenize(line, delim, scope_delim);
            
            auto num_tokens = static_cast<int>(tokens.size());
            for (int tk_idx = 0; tk_idx < num_tokens; tk_idx += 2)
            {
              const auto& tk_name = tokens[tk_idx + 0];
              const auto& tk_val  = tokens[tk_idx + 1];
              if (tk_name == "char")
              {
                if (tk_val.size() == 1)
                  ch = tk_val[0];
                else
                  std::cerr << "Error in ASCII_Fonts::load_font_data() : 'char:' value token \"" << tk_val << "\" is not of size 1!" << std::endl;
              }
              else if (tk_name == "width")
              {
                std::istringstream iss(tk_val);
                iss >> width;
              }
              else if (tk_name == "char_prev")
              {
                if (tk_val.size() == 1)
                {
                  ch_prev = tk_val[0];
                  for (int i = 1; tk_idx + 1 + i < num_tokens; ++i)
                  {
                    const auto& tk = tokens[tk_idx + 1 + i];
                    if (tk.size() != 1)
                    {
                      std::cerr << "Error in ASCII_Fonts::load_font_data() : 'ch_prev' value token \"" << tk << "\" is not of size 1!" << std::endl;
                      continue;
                    }
                    auto ch_i = tk[0];
                    ch_prev_vec.emplace_back(ch_i);
                  }
                  tk_idx = num_tokens;
                }
                else
                  std::cerr << "Error in ASCII_Fonts::load_font_data() : 'char_prev:' value token \"" << tk_val << "\" is not of size 1!" << std::endl;
              }
            }
          }
          else if (strcmp(line.c_str(), "kerning:") == 0)
          {
            kerning = true;
            ordering = false;
            ch = -1;
            ch_prev = -1;
            ch_prev_vec.clear();
          }
          else if (strcmp(line.c_str(), "ordering:") == 0)
          {
            kerning = false;
            ordering = true;
            ch = -1;
            ch_prev = -1;
            ch_prev_vec.clear();
          }
          
          // Read data.
          if (kerning && utils::sscanf(line.c_str(), "'%c' -> '%c' = %i", &kch0, &kch1, &kern) == 3)
            curr_font.kernings[{kch0, kch1}] = kern;
          else if (ordering && utils::sscanf(line.c_str(), "'%c' -> '%c' = %i, %i", &och0, &och1, &opri0, &opri1) == 4)
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
              else
              {
                std::istringstream iss2(line);
                int r, c;
                std::string fg, bg;
                int glyph_part_prio = 0;

                if (iss2 >> r >> c >> fg >> bg)
                {
                  piece.r = r;
                  piece.c = c;
                  piece.fg = fg;
                  piece.bg = bg;
                  piece.prio = 0;

                  if (iss2 >> glyph_part_prio)
                    piece.prio = glyph_part_prio;

                  curr_char->font_pieces.emplace_back(piece);
                }
              }
            }
            if (!ch_prev_vec.empty())
            {
              for (char ch_p : ch_prev_vec)
                curr_font.font_chars_by_prev_char[{ch_p, ch}] = *curr_char;
            }
          }
        }
      }
      
      curr_font.treat_lower_case_as_upper_case = treat_lower_case_as_upper_case;
      
      txt_file.close();
    }
    return font_data;
  }

  // (r, c) : top left corner of text.
  // returns the relative start column (top left corner) for the next character.
  template<int NR, int NC>
  int draw_char(SpriteHandler<NR, NC>& sh, const FontData& curr_font, const ColorScheme& colors,
                char ch_prev, char ch_curr, char ch_next,
                int ch_curr_order,
                int r, int c,
                int custom_kerning = 0)
  {
    if (curr_font.treat_lower_case_as_upper_case)
    {
      ch_prev = str::to_upper(ch_prev);
      ch_curr = str::to_upper(ch_curr);
      ch_next = str::to_upper(ch_next);
    }
    
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
        t.style =
        {
          get_fg_color(piece.fg, colors),
          get_bg_color(piece.bg, colors)
        };
        t.priority = ch_curr_order + piece.prio;
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
      prev_prio = curr_prio;
    
      char ch_curr = text[ch_idx];
      char ch_prev = text[ch_idx - 1];
      
      if (curr_font.treat_lower_case_as_upper_case)
      {
        ch_prev = str::to_upper(ch_prev);
        ch_curr = str::to_upper(ch_curr);
      }
      
      auto it_o = curr_font.orderings.find({ ch_prev, ch_curr });
      if (it_o != curr_font.orderings.end())
      {
        auto [o0, o1] = it_o->second;
        auto diff = o1 - o0;
        curr_prio = prev_prio + diff;
      }
      else
        curr_prio = prev_prio;
        
      ordered_text.emplace_back(ch_curr, curr_prio);
    }
    return ordered_text;
  }
  

  // (r, c) : top left corner of text.
  template<int NR, int NC>
  void draw_text(SpriteHandler<NR, NC>& sh, const FontDataColl& font_data, const ColorScheme& colors,
                 const std::string& text,
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
        
        width += draw_char(sh, curr_font, colors,
          ch_prev, ch_curr, ch_next,
          ch_curr_order,
          r, c + width,
          ck);
      }
    
      sh.write_buffer_ordered();
    }
  }

}
