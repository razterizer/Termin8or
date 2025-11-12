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
  bool show_help = false;
  
  for (int i = 1; i < argc; ++i)
  {
    if (std::strcmp(argv[i], "--help") == 0)
      show_help = true;
    else if (std::strcmp(argv[i], "--disable_dynamics_system") == 0)
      use_dynamics_system = false;
    else if (std::strcmp(argv[i], "--dbg_draw_sprites") == 0)
      dbg_draw_sprites = true;
    else if (std::strcmp(argv[i], "--dbg_draw_rigid_bodies") == 0)
      dbg_draw_rigid_bodies = true;
    else if (std::strcmp(argv[i], "--dbg_draw_broad_phase") == 0)
      dbg_draw_broad_phase = true;
    else if (std::strcmp(argv[i], "--dbg_draw_narrow_phase") == 0)
      dbg_draw_narrow_phase = true;
    else if (std::strcmp(argv[i], "--hide_sprites") == 0)
      draw_sprites = false;
  }
  
  if (show_help)
  {
    std::cout << "examples --help |" << std::endl;
    std::cout << "   [--disable_dynamics_system]" << std::endl;
    std::cout << "   [--dbg_draw_sprites]" << std::endl;
    std::cout << "   [--dbg_draw_rigid_bodies]" << std::endl;
    std::cout << "   [--dbg_draw_broad_phase]" << std::endl;
    std::cout << "   [--dbg_draw_narrow_phase]" << std::endl;
    std::cout << "   [--hide_sprites]" << std::endl;
  
    return 0;
  }

  std::cout << "### SpriteHandler Examples ###" << std::endl;
  sprite_handler::example1();
  sprite_handler::example2();
  std::cout << "### Keyboard Examples ###" << std::endl;
  keyboard::example1();
  
  return 0;
}
