//
//  Texture.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-25.
//

#pragma once
#include "RC.h"
#include "Color.h"
#include "Styles.h"
#include "Rectangle.h"
#include <Core/TextIO.h>
#include <sstream>


namespace t8::drawing
{

  // size.r = 1, size.c = 1 yields a 1x1 texture.
  struct Textel
  {
    char ch = ' ';
    Color fg_color = Color::Default;
    Color bg_color = Color::Transparent2;
    int mat = 0;
    
    color::Style get_style() const { return { fg_color, bg_color }; }
    void set_style(const color::Style& style)
    {
      fg_color = style.fg_color;
      bg_color = style.bg_color;
    }
    
    std::string str() const { return std::string(1, ch); }
    
    // 0 to 15 is hexadecimal, then just continue down the alphabet.
    std::string mat_to_char_str()
    {
      if (0 <= mat && mat <= 9)
        return std::string(1, '0' + static_cast<char>(mat));
      else if (10 <= mat)
        return std::string(1, 'A' + static_cast<char>(mat) - 10);
      return std::string(1, '0');
    };
    
    bool operator==(const Textel& other) const
    {
      return this->ch == other.ch
        && this->fg_color == other.fg_color
        && this->bg_color == other.bg_color
        && this->mat == other.mat;
    }
  };
  
  struct Texture
  {
    RC size;
    int area = 0;
    std::vector<char> characters;
    std::vector<Color> fg_colors;
    std::vector<Color> bg_colors;
    std::vector<int> materials;
    
    Texture() = default;
    Texture(const RC& tex_size)
      : size(tex_size)
      , area(tex_size.r*tex_size.c)
      , characters(area, ' ')
      , fg_colors(area, Color::Default)
      , bg_colors(area, Color::Transparent2)
      , materials(area, 0)
    {}
    Texture(int tex_rows, int tex_cols)
      : size({ tex_rows, tex_cols })
      , area(tex_rows*tex_cols)
      , characters(area, ' ')
      , fg_colors(area, Color::Default)
      , bg_colors(area, Color::Transparent2)
      , materials(area, 0)
    {}
    Texture(const Texture& other)
      : size(other.size)
      , area(other.area)
      , characters(other.characters)
      , fg_colors(other.fg_colors)
      , bg_colors(other.bg_colors)
      , materials(other.materials)
    {}
    
    void init_materials(int mat)
    {
      stlutils::memset(materials, mat);
    }
    
    Textel operator()(int r, int c) const
    {
      if (!check_range(r, c))
        return {};
      Textel tex;
      int idx = r * size.c + c;
      tex.ch = characters[idx];
      tex.fg_color = fg_colors[idx];
      tex.bg_color = bg_colors[idx];
      tex.mat = materials[idx];
      return tex;
    }
    
    Textel operator()(const RC& pos) const
    {
      return operator()(pos.r, pos.c);
    }
    
    void set_textel(int r, int c, const Textel& textel)
    {
      if (!check_range(r, c))
        return;
      int idx = r * size.c + c;
      characters[idx] = textel.ch;
      fg_colors[idx] = textel.fg_color;
      bg_colors[idx] = textel.bg_color;
      materials[idx] = textel.mat;
    }
    
    void set_textel(const RC& pos, const Textel& textel)
    {
      set_textel(pos.r, pos.c, textel);
    }
    
    void set_textel_char(int r, int c, char ch)
    {
      if (!check_range(r, c))
        return;
      int idx = r * size.c + c;
      characters[idx] = ch;
    }
    
    void set_textel_char(const RC& pos, char ch)
    {
      set_textel_char(pos.r, pos.c, ch);
    }
    
    void set_textel_fg_color(int r, int c, Color fg_color)
    {
      if (!check_range(r, c))
        return;
      int idx = r * size.c + c;
      fg_colors[idx] = fg_color;
    }
    
    void set_textel_fg_color(const RC& pos, Color fg_color)
    {
      set_textel_fg_color(pos.r, pos.c, fg_color);
    }
    
    void set_textel_bg_color(int r, int c, Color bg_color)
    {
      if (!check_range(r, c))
        return;
      int idx = r * size.c + c;
      bg_colors[idx] = bg_color;
    }
    
    void set_textel_bg_color(const RC& pos, Color bg_color)
    {
      set_textel_bg_color(pos.r, pos.c, bg_color);
    }
    
    void set_textel_material(int r, int c, int mat)
    {
      if (!check_range(r, c))
        return;
      int idx = r * size.c + c;
      materials[idx] = mat;
    }
    
    void set_textel_material(const RC& pos, int mat)
    {
      set_textel_material(pos.r, pos.c, mat);
    }
    
    // File format:
    // size, chars, fg-colors, bg-colors, materials.
    //-------------
    // 3 11
    //
    // /|||::::~~~
    // |:|^..::o~~
    // II#&7/(8---
    //
    // 5A7A4T4BBFF
    // 56633AABBFF
    // TTT5T7T5BBF
    //
    // 122238223CC
    // 1222322255C
    // 2333111CC5T
    //
    // 00001111222
    // 01030110222
    // 44544344222
    
    bool load(const std::string& file_path)
    {
      std::vector<std::string> lines;
      
      auto f_char_to_color = [](char ch)
      {
        switch (ch)
        {
          case 't': return Color::Transparent;
          case 'T': return Color::Transparent2;
          case '*': return Color::Default;
          case '0': return Color::Black;
          case '1': return Color::DarkRed;
          case '2': return Color::DarkGreen;
          case '3': return Color::DarkYellow;
          case '4': return Color::DarkBlue;
          case '5': return Color::DarkMagenta;
          case '6': return Color::DarkCyan;
          case '7': return Color::LightGray;
          case '8': return Color::DarkGray;
          case '9': return Color::Red;
          case 'A': return Color::Green;
          case 'B': return Color::Yellow;
          case 'C': return Color::Blue;
          case 'D': return Color::Magenta;
          case 'E': return Color::Cyan;
          case 'F': return Color::White;
          default: return Color::Default;
        }
      };
      
      // 0 to 15 is hexadecimal, then just continue down the alphabet.
      auto f_char_to_mat = [](int ch) -> int
      {
        if ('0' <= ch && ch <= '9')
          return ch - '0';
        else if ('A' <= ch)
          return ch - 'A' + 10;
        return 0;
      };
      
      bool ret = TextIO::read_file(file_path, lines);
      if (!ret)
        return false;
      
      int section = 0;
      int r = 0;
      for (const auto& l : lines)
      {
        if (section == 0)
        {
          std::istringstream iss(l);
          iss >> size.r >> size.c;
          area = size.r * size.c;
          characters.resize(area, ' ');
          fg_colors.resize(area, Color::Default);
          bg_colors.resize(area, Color::Transparent2);
          materials.resize(area, -1);
          section = 1;
          r = 0;
        }
        else if (section == 1)
        {
          if (!l.empty())
          {
            for (int c = 0; c < size.c; ++c)
            {
              int idx = r * size.c + c;
              characters[idx] = l[c];
            }
            r++;
          }
          else if (r > 0)
          {
            section = 2;
            r = 0;
          }
        }
        else if (section == 2)
        {
          if (!l.empty())
          {
            for (int c = 0; c < size.c; ++c)
            {
              int idx = r * size.c + c;
              fg_colors[idx] = f_char_to_color(l[c]);
            }
            r++;
          }
          else if (r > 0)
          {
            section = 3;
            r = 0;
          }
        }
        else if (section == 3)
        {
          if (!l.empty())
          {
            for (int c = 0; c < size.c; ++c)
            {
              int idx = r * size.c + c;
              bg_colors[idx] = f_char_to_color(l[c]);
            }
            r++;
          }
          else if (r > 0)
          {
            section = 4;
            r = 0;
          }
        }
        else if (section == 4)
        {
          if (!l.empty())
          {
            for (int c = 0; c < size.c; ++c)
            {
              int idx = r * size.c + c;
              materials[idx] = f_char_to_mat(l[c]);
            }
            r++;
          }
          else if (r > 0)
          {
            section = 5;
            r = 0;
          }
        }
      }
      return true;
    }
    
    bool save(const std::string& file_path)
    {
      std::vector<std::string> lines;
      
      auto f_color_to_char = [](Color color)
      {
        switch (color)
        {
          case Color::Transparent:  return 't';
          case Color::Transparent2: return 'T';
          case Color::Default:      return '*';
          case Color::Black:        return '0';
          case Color::DarkRed:      return '1';
          case Color::DarkGreen:    return '2';
          case Color::DarkYellow:   return '3';
          case Color::DarkBlue:     return '4';
          case Color::DarkMagenta:  return '5';
          case Color::DarkCyan:     return '6';
          case Color::LightGray:    return '7';
          case Color::DarkGray:     return '8';
          case Color::Red:          return '9';
          case Color::Green:        return 'A';
          case Color::Yellow:       return 'B';
          case Color::Blue:         return 'C';
          case Color::Magenta:      return 'D';
          case Color::Cyan:         return 'E';
          case Color::White:        return 'F';
          default: return '*';
        }
      };
      
      // 0 to 15 is hexadecimal, then just continue down the alphabet.
      auto f_mat_to_char = [](int mat) -> char
      {
        if (0 <= mat && mat <= 9)
          return '0' + static_cast<char>(mat);
        else if (10 <= mat)
          return 'A' + static_cast<char>(mat) - 10;
        return '0';
      };
      
      std::ostringstream oss;
      oss << size.r << " " << size.c;
      lines.emplace_back(oss.str());
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += characters[idx];
        }
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += f_color_to_char(fg_colors[idx]);
        }
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += f_color_to_char(bg_colors[idx]);
        }
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += f_mat_to_char(materials[idx]);
        }
        lines.emplace_back(curr_line);
      }
      
      return TextIO::write_file(file_path, lines);
    }
    
    // +-+
    // | |
    // +-+
    // zero_area_is_one_char = false (default):
    // r_len = 3, c_len = 3
    // zero_area_is_one_char = true:
    // r_len = 2, c_len = 2
    // So that:
    // zero_area_is_one_char = true:
    // r_len = 0, c_len = 0
    // +
    
    Texture subset(const Rectangle& bb_subset, bool zero_area_is_one_char = false)
    {
      auto bb = bb_subset; //
      if (zero_area_is_one_char)
      {
        bb.r_len++;
        bb.c_len++;
      }
      
      Texture sub_texture(bb.size());
      for (int r = bb.r; r <= bb.r_len; ++r)
      {
        for (int c = bb.c; c <= bb.c_len; ++c)
        {
          int idx_from = r * size.c + c;
          int idx_to = (r - bb.r) * bb.c_len + (c - bb.c);
          sub_texture.characters[idx_to] = sub_texture.characters[idx_from];
          sub_texture.fg_colors[idx_to] = sub_texture.fg_colors[idx_from];
          sub_texture.bg_colors[idx_to] = sub_texture.bg_colors[idx_from];
          sub_texture.materials[idx_to] = sub_texture.materials[idx_to];
        }
      }
      
      return sub_texture;
    }
    
    bool empty() const { return size.r == 0 && size.c == 0; }
    
    void clear()
    {
      size = { -1, -1 };
      area = 0;
      characters.clear();
      fg_colors.clear();
      bg_colors.clear();
      materials.clear();
    }
    
  private:
    bool check_range(int r, int c) const
    {
      if (!math::in_range<int>(r, 0, size.r, Range::ClosedOpen))
        return false;
      if (!math::in_range<int>(c, 0, size.c, Range::ClosedOpen))
        return false;
      return true;
    }
  };

}
