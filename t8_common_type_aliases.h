//
//  t8_common_type_aliases.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2025-09-04.
//

#pragma once

// core types
using Color = t8::Color;
using Style = t8::color::Style;
using RC = t8::RC;
using Rectangle = t8::Rectangle;

// screen
template<int NR, int NC>
using ScreenHandler = t8::screen::ScreenHandler<NR, NC>;

// drawing
using Textel = t8::drawing::Textel;
using Texture = t8::drawing::Texture;

// input
using SpecialKey = t8::input::SpecialKey;
