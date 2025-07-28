//
//  RectangleHelper.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2025-07-27.
//

#pragma once

#include "Rectangle.h"
#include <Core/StringHelper.h>
#include <optional>

namespace ttl
{
  
  std::string rectangle_to_str(const Rectangle& rec)
  {
    return "{ { " + std::to_string(rec.r) + ", " + std::to_string(rec.c) + " }, { " + std::to_string(rec.r_len) + ", " + std::to_string(rec.c_len) + " } }";
  }
  
  std::optional<Rectangle> str_to_rectangle(const std::string& str)
  {
    auto tokens = str::tokenize(str, { ',', ' ' }, { '{', '}' });
    if (tokens.size() != 4)
      return std::nullopt;
    Rectangle rec;
    rec.r = std::atoi(tokens[0].c_str());
    rec.c = std::atoi(tokens[1].c_str());
    rec.r_len = std::atoi(tokens[2].c_str());
    rec.c_len = std::atoi(tokens[3].c_str());
    return rec;
  }

}
