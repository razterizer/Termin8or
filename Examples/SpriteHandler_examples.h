//
//  SpriteHandler_tests.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-16.
//

#pragma once
#include "../SpriteHandler.h"
#include "../ScreenUtils.h"


namespace sprite_handler
{

  void example1()
  {
    ScreenHandler<20, 40> sh;
    Text tt;
    SpriteHandler sprh;
    
    clear_screen();
    return_cursor();
    hide_cursor();
    
    auto* sprite0 = sprh.create_sprite("spaceship");
    sprite0->init(4, 5);
    sprite0->init_frame(0);
    sprite0->set_sprite_chars(0,
      ' ', ' ', '_', ' ', ' ',
      ' ', '/', '#', '\\', ' ',
      '[', '-', '-', '-', ']',
      ' ', '{', '#', '}', ' '
    );
    sprite0->set_sprite_fg_colors(0,
      Color::Default, Color::Default, Color::White, Color::Default, Color::Default,
      Color::Default, Color::Red, Color::Magenta, Color::Red, Color::Default,
      Color::Green, Color::Cyan, Color::Cyan, Color::Cyan, Color::Green,
      Color::Default, Color::Yellow, Color::Magenta, Color::Yellow, Color::Default
    );
    sprite0->set_sprite_bg_colors(0,
      Color::Transparent2, Color::Transparent2, Color::Transparent2, Color::Transparent2, Color::Transparent2,
      Color::Transparent2, Color::Transparent2, Color::DarkMagenta, Color::Transparent2, Color::Transparent2,
      Color::Transparent2, Color::DarkCyan, Color::DarkCyan, Color::DarkCyan, Color::Transparent2,
      Color::Transparent2, Color::Transparent2, Color::DarkMagenta, Color::Transparent2, Color::Transparent2
    );
    sprite0->init_frame(1);
    sprite0->set_sprite_chars(1,
      ' ', ' ', '_', ' ', ' ',
      ' ', '/', '#', '\\', ' ',
      '[', '-', '-', '-', ']',
      ' ', '{', '#', '}', ' '
    );
    sprite0->set_sprite_fg_colors(1,
      Color::Default, Color::Default, Color::White, Color::Default, Color::Default,
      Color::Default, Color::Red, Color::Magenta, Color::Red, Color::Default,
      Color::Green, Color::Cyan, Color::Cyan, Color::Cyan, Color::Green,
      Color::Default, Color::Yellow, Color::Yellow, Color::Yellow, Color::Default
    );
    sprite0->set_sprite_bg_colors(1,
      Color::Transparent2, Color::Transparent2, Color::Transparent2, Color::Transparent2, Color::Transparent2,
      Color::Transparent2, Color::Transparent2, Color::DarkMagenta, Color::Transparent2, Color::Transparent2,
      Color::Transparent2, Color::DarkCyan, Color::DarkCyan, Color::DarkCyan, Color::Transparent2,
      Color::Transparent2, Color::Transparent2, Color::DarkYellow, Color::Transparent2, Color::Transparent2
    );
    sprite0->func_frame_to_texture = [](int anim_frame)
    {
      auto anim = anim_frame % 14;
      if (anim < 8)
        return 0;
      return anim % 2;
    };
    
    auto* sprite1 = sprh.create_sprite("alien");
    sprite1->init(2, 3);
    sprite1->init_frame(0);
    sprite1->set_sprite_chars(0,
      ' ', '^', ' ',
      '%', 'U', '%'
    );
    sprite1->set_sprite_fg_colors(0,
      Color::Default, Color::Green, Color::Default,
      Color::Yellow, Color::DarkYellow, Color::Yellow
    );
    sprite1->set_sprite_bg_colors(0,
      Color::Transparent2, Color::Transparent2, Color::Transparent2,
      Color::Transparent2, Color::DarkYellow, Color::Transparent2
    );
    sprite1->init_frame(1);
    sprite1->set_sprite_chars(1,
      ' ', '^', ' ',
      '#', 'U', '#'
    );
    sprite1->set_sprite_fg_colors(1,
      Color::Default, Color::Green, Color::Default,
      Color::DarkYellow, Color::DarkYellow, Color::DarkYellow
    );
    sprite1->set_sprite_bg_colors(1,
      Color::Transparent2, Color::Transparent2, Color::Transparent2,
      Color::Transparent2, Color::DarkYellow, Color::Transparent2
    );
    sprite1->func_frame_to_texture = [](int anim_frame)
    {
      auto anim = anim_frame % 40;
      if (anim < 34)
        return 0;
      return anim % 2;
    };
    
    for (int i = -3; i < 20; ++i)
    {
      for (int j = -5; j < 40; ++j)
      {
        sprite0->pos.r = i;
        sprite0->pos.c = i%2==0 ? j : 35-j;
        
        int anim_frame = (i + 3)*44 + (j + 5);
        auto t = static_cast<float>(anim_frame)/(23.f*45.f);
        sprite1->pos.c = math::roundI(20.f + 6.f*std::cos(math::c_2pi * 7 * t));
        sprite1->pos.r = math::roundI(10.f + 4*std::sin(math::c_2pi * 10 * t));
        
        return_cursor();
        sh.clear();
        sprh.draw(sh, anim_frame);
        sh.print_screen_buffer(tt, Color::Black);
        Delay::sleep(0'200'000);
      }
    }
  }

}
