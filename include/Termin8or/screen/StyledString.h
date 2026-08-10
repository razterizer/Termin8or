//
//  StyledString.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-03-21.
//

#pragma once
#include "Styles.h"
#include <Core/StlUtils.h>
#include <string>


namespace t8
{
  
  struct StyledString
  {
    std::string text;
    Style style;
    int width = 0;
  };
  
  using StyledStringVec = std::vector<StyledString>;
  
  inline int get_sstr_vec_width(const StyledStringVec& ss_vec)
  {
    return stlutils::sum<int>(ss_vec, [](const auto& ss) { return ss.width; });
  }
  
}
