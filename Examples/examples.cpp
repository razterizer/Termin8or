//
//  examples.cpp
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-16.
//

#include "SpriteHandler_examples.h"
#include "Keyboard_examples.h"
#include <iostream>


int main(int argc, char** argv)
{
  std::cout << "### SpriteHandler Examples ###" << std::endl;
  sprite_handler::example1();
  sprite_handler::example2();
  std::cout << "### Keyboard Examples ###" << std::endl;
  keyboard::example1();
  
  return 0;
}
