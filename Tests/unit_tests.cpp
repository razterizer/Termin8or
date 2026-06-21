//
//  unit_tests.cpp
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-09-22.
//

#include "Rectangle_tests.h"
#include "Color_tests.h"
#include "Glyph_tests.h"
#include "GlyphString_tests.h"
#include "TextureFile_tests.h"
#include <iostream>


int main(int argc, char** argv)
{
  std::cout << "### Rectangle Tests ###" << std::endl;
  rectangle::unit_tests();
  std::cout << "### Color Tests ###" << std::endl;
  color::unit_tests();
  std::cout << "### Glyph Tests ###" << std::endl;
  glyph::unit_tests();
  std::cout << "### GlyphString Tests ###" << std::endl;
  glyph_string::unit_tests();
  std::cout << "### TextureFile Tests ###" << std::endl;
  texture_file::unit_tests();
  
  return 0;
}
