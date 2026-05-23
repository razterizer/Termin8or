//
//  ascii_helpers.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-23.
//

#pragma once
#include <Core/MathUtils.h>

namespace t8
{

  template<typename CharT>
  inline constexpr bool is_printable_ascii(CharT cp) noexcept
  {
    return math::in_r_c<CharT>(cp, 0x20, 0x7E);
  }
  
  template<typename CharT>
  inline constexpr bool is_ascii(CharT cp) noexcept
  {
    return math::in_r_c<CharT>(cp, 0x00, 0x7F);
  }

}
