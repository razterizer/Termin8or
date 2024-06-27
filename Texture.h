//
//  Texture.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-25.
//

#pragma once
#include "RC.h"
#include "Color.h"
#include <Core/TextIO.h>


namespace drawing
{

  // size.r = 1, size.c = 1 yields a 1x1 texture.
  struct Textel
  {
    char ch = ' ';
    Color fg_color = Color::Default;
    Color bg_color = Color::Transparent2;
    int mat = -1;
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
      , materials(area, -1)
    {}
    Texture(int tex_rows, int tex_cols)
      : size({ tex_rows, tex_cols })
      , area(tex_rows*tex_cols)
      , fg_colors(area, Color::Default)
      , bg_colors(area, Color::Transparent2)
      , materials(area, -1)
    {}
    
    Textel operator()(int r, int c) const
    {
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
    
    void set_textel(const Texel& textel, int r, int c)
    {
      int idx = r * size.c + c;
      characters[idx] = textel.ch;
      fg_colors[idx] = textel.fg_color;
      bg_colors[idx] = textel.bg_color;
      materials[idx] = textel.mat;
    }
    
    void set_textel(const Textel& textel, const RC& pos)
    {
      set_textel(textel, pos.r, pos.c);
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
          section = 1;
          r = 0;
        }
        else if (section == 1)
        {
          if (!l.empty())
          {
            for (int c = 0; c <= size.c; ++c)
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
            for (int c = 0; c <= size.c; ++c)
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
            for (int c = 0; c <= size.c; ++c)
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
            for (int c = 0; c <= size.c; ++c)
            {
              int idx = r * size.c + c;
              materials[idx] = l[c] - 48; // #FIXME: Use hex to int conversion instead!
            }
            r++;
          }
          else if (r > 0)
          {
            section = 3;
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
      
      std::ostringstream oss;
      oss << size.r << " " << size.c;
      lines.emplace_back(oss.str());
      lines.emplace_back("");
      for (int r = 0; r <= size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c <= size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += characters[idx];
        }
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r <= size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c <= size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += f_color_to_char(fg_colors[idx]);
        }
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r <= size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c <= size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += f_color_to_char(bg_colors[idx]);
        }
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r <= size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c <= size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += std::to_string(materials[idx]); // #FIXME: Use hex to int conversion instead!
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
    
    Texture subset(const ttl::Rectangle& bb_subset, bool zero_area_is_one_char = false)
    {
      Texture sub_texture;
      
      auto bb = bb_subset; //
      if (zero_area_is_one_char)
      {
        bb.r_len++;
        bb.c_len++;
      }
      
      sub_texture.size = bb.size();
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
  };

}
