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
    keyboard::KeyPressData kpd;
    auto keyboard = std::make_unique<keyboard::StreamKeyboard>();
    
    rnd::srand_time();
    
    // //////////////////////////////
    
    auto* sprite0 = sprh.create_sprite("spaceship");
    sprite0->layer_id = 4;
    sprite0->init(4, 5);
    sprite0->init_frame(0);
    sprite0->set_sprite_chars_from_strings(0,
      "  _  ",
      " /#\\ ",
      "[---]",
      "  #  "
    );
    sprite0->set_sprite_fg_colors(0,
      0, 0, 16, 0, 0,
      0, 16, 16, 13, 0,
      11, 15, 15, 15, 11,
      0, 12, 14, 12, 0
    );
    sprite0->set_sprite_bg_colors(0,
      -2, -2, -2, -2, -2,
      -2, -2, 13, -2, -2,
      1, 7, 7, 7, 1,
      -2, -2, 14, -2, -2
    );
    sprite0->init_frame(1);
    sprite0->set_sprite_chars_from_strings(1,
      "  _  ",
      " /#\\ ",
      "[---]",
      "  #  "
    );
    sprite0->set_sprite_fg_colors(1,
      0, 0, 16, 0, 0,
      0, 16, 16, 13, 0,
      11, 15, 15, 15, 11,
      0, 12, 12, 12, 0
    );
    sprite0->set_sprite_bg_colors(1,
      -2, -2, -2, -2, -2,
      -2, -2, 13, -2, -2,
      1, 7, 7, 7, 1,
      -2, -2, 4, -2, -2
    );
    sprite0->init_frame(2);
    sprite0->set_sprite_chars_from_strings(2,
      "  _  ",
      " /#\\ ",
      "[---]",
      "  ^  "
    );
    sprite0->set_sprite_fg_colors(2,
      0, 0, 16, 0, 0,
      0, 16, 16, 13, 0,
      11, 15, 15, 15, 11,
      0, 12, 16, 12, 0
    );
    sprite0->set_sprite_bg_colors(2,
      -2, -2, -2, -2, -2,
      -2, -2, 13, -2, -2,
      1, 7, 7, 7, 1,
      -2, -2, -2, -2, -2
    );
    sprite0->func_frame_to_texture = [](int anim_frame)
    {
      auto anim = anim_frame % 14;
      if (anim < 8)
        return 2;
      return anim % 2;
    };
    
    // ///////////////////////////////////////////////////////////
    
    auto* sprite1 = sprh.create_sprite("alien");
    sprite1->layer_id = 5;
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
    
    // ///////////////////////////////////////////////////////////
    
    std::array<std::tuple<Sprite*, float, float>, 16> asteroids;
    for (int a_idx = 0; a_idx < asteroids.size(); ++a_idx)
    {
      auto* sprite2 = sprh.create_sprite("asteroid" + std::to_string(a_idx));
      sprite2->pos.r = rnd::rand_int(0, sh.num_rows()-1);
      sprite2->pos.c = rnd::rand_int(0, sh.num_cols()-1);
      sprite2->layer_id = rnd::rand_select<int>({ 1, 3 });
      sprite2->init(1, 1);
      sprite2->init_frame(0);
      sprite2->set_sprite_chars(0, '@');
      sprite2->set_sprite_fg_colors(0, Color::DarkGray);
      sprite2->set_sprite_bg_colors(0, Color::Transparent2);
      // sprite, r_pos, r_vel.
      asteroids[a_idx] =
      {
        sprite2,
        static_cast<float>(sprite2->pos.r),
        rnd::rand_float(0.8f, 2.f)
      };
    }
    
    for (int s_idx = 0; s_idx < 20; ++s_idx)
    {
      auto* sprite3 = sprh.create_sprite("star" + std::to_string(s_idx));
      sprite3->pos.r = rnd::rand_int(0, sh.num_rows()-1);
      sprite3->pos.c = rnd::rand_int(0, sh.num_cols()-1);
      sprite3->layer_id = 0;
      sprite3->init(1, 1);
      sprite3->init_frame(0);
      char star_ch = rnd::rand_select<char>({ '.', '+' });
      sprite3->set_sprite_chars(0, star_ch);
      sprite3->set_sprite_fg_colors(0, rnd::rand_select<Color>({ Color::White, Color::White, Color::White, Color::White, Color::White, Color::Yellow, Color::Yellow, Color::Yellow, Color::Red, Color::Blue, Color::Blue, Color::Blue }));
      sprite3->set_sprite_bg_colors(0, Color::Transparent2);
      sprite3->init_frame(1);
      sprite3->set_sprite_chars(1, star_ch);
      sprite3->set_sprite_fg_colors(1, Color::Black);
      sprite3->set_sprite_bg_colors(1, Color::Transparent2);
      const int max_twinkle = 100;
      int twinkle_offs = rnd::rand_int(0, max_twinkle);
      sprite3->func_frame_to_texture = [twinkle_offs](int anim_frame)
      {
        return 1 - (anim_frame + twinkle_offs) % max_twinkle;
      };
    }
    
    auto* sprite4 = sprh.create_sprite("background");
    sprite4->layer_id = 2;
    sprite4->init(sh.num_rows(), sh.num_cols());
    sprite4->init_frame(0);
    sprite4->set_sprite_chars_from_strings(0,
      R"(                                        )",
      R"(                                        )",
      R"(                                        )",
      R"(                                        )",
      R"(                                        )",
      R"(                                        )",
      R"(                                        )",
      R"(                                        )",
      R"(                                        )",
      R"(:::::::                                 )",
      R"(~~~.~=.::%%                             )",
      R"(ou.~~~~~,.~%%%%                         )",
      R"(898o.~~~+~~~~o8%%%                      )",
      R"(9883o..,~~~~ooO9Wm**                    )",
      R"(8i8o.~~.~~~"%oOoO%8w**                  )",
      R"(86o>+~~~~~%OooOOoww~<)oo                )",
      R"(8Oo+~+~~<%%OOo~~mm~~~/o\o               )",
      R"(3~~oo~~%%M%68OooO~mm~|(9\o              )",
      R"(3S0~o~~p%MM%99oOOo8ooo\6)o              )",
      R"(8%~o~~%WW%SSooOOOoooo~~~~~i             )"
    );
    sprite4->set_sprite_fg_colors(0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      3, 3, 11, 11, 11, 11, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      15, 15, 15, 3, 15, 13, 3, 11, 11, 15, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      13, 7, 3, 15, 15, 15, 15, 15, 15, 3, 15, 7, 7, 15, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      3, 13, 3, 13, 3, 15, 15, 15, 7, 15, 15, 15, 15, 13, 3, 15, 15, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      13, 3, 3, 11, 13, 3, 3, 15, 15, 15, 15, 15, 13, 13, 5, 13, 3, 11, 15, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      3, 11, 3, 13, 3, 15, 15, 3, 15, 15, 15, 13, 7, 13, 5, 13, 5, 7, 3, 15, 13, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      3, 3, 13, 7, 7, 15, 15, 15, 15, 15, 7, 5, 13, 13, 5, 5, 13, 15, 15, 15, 7, 4, 4, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      3, 5, 13, 7, 15, 7, 15, 15, 7, 7, 7, 5, 5, 13, 15, 15, 11, 11, 15, 15, 15, 4, 12, 4, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      11, 15, 15, 13, 13, 15, 15, 7, 7, 3, 7, 3, 3, 5, 13, 13, 5, 15, 11, 11, 15, 4, 12, 12, 4, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      11, 7, 7, 15, 13, 15, 15, 5, 7, 3, 3, 7, 13, 13, 13, 5, 5, 13, 3, 13, 13, 13, 4, 12, 4, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      3, 7, 15, 13, 15, 15, 7, 3, 3, 7, 7, 7, 13, 13, 5, 5, 13, 13, 13, 15, 15, 15, 15, 15, 11, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ); // G:11, DG:3, DC:7, C:15, B:13, DB:5, DC:7, DG:3, G:11
    
/*
C:15 : '~', 'w', ','
B:13 : 'o', '9', '"', '='
DB:5 : 'O', 'p', '*'
DC:7 : 'S', '0', '<', '>', '%', '+', 'u'
DG:3 : 'W', '6', '8', '.'
G:11 : 'i', 'M', '3', 'm', ':'
*/
    sprite4->set_sprite_bg_colors(0,
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      8, 16, 16, 16, 8, 7, 3, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      7, 7, 7, 8, 7, 15, 8, 3, 13, 13, 13, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      7, 3, 8, 7, 7, 7, 7, 7, 8, 8, 7, 3, 13, 13, 13, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      11, 11, 11, 7, 8, 7, 7, 7, 8, 7, 7, 7, 7, 7, 11, 3, 13, 13, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      11, 11, 11, 3, 7, 8, 8, 8, 7, 7, 7, 7, 7, 7, 11, 11, 11, 15, 7, 7, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      11, 3, 11, 7, 8, 7, 7, 8, 7, 7, 7, 15, 15, 7, 11, 7, 11, 15, 11, 15, 7, 7, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      11, 13, 7, 8, 8, 7, 7, 7, 7, 7, 15, 11, 7, 7, 11, 11, 7, 15, 15, 7, 8, 5, 13, 7, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      11, 11, 7, 8, 7, 8, 7, 7, 8, 15, 15, 11, 11, 7, 7, 7, 15, 15, 7, 7, 7, 5, 13, 5, 7, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      3, 7, 7, 7, 7, 7, 7, 15, 15, 11, 15, 13, 11, 11, 7, 7, 11, 7, 15, 15, 7, 5, 13, 13, 5, 7, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      3, 13, 5, 7, 7, 7, 7, 3, 15, 11, 11, 15, 11, 11, 7, 11, 11, 7, 11, 7, 7, 7, 5, 13, 5, 7, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      11, 15, 7, 7, 7, 7, 15, 11, 11, 15, 13, 13, 7, 7, 11, 11, 11, 7, 7, 7, 7, 7, 7, 3, 3, 11, 3, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
    );
    
/*
B:13 : 'S', '6'
DB:5 : '0'
C:15 : '%', 'm', 'w'
DC:7 : '~', 'o'
G:11 : '8', 'W', 'O', 'M', '9'
DG:3 : '3', 'p', 'i'
LG:8 : '>', '<', '+', '.', ','

*/
    
    // ///////////////////////////////////////////////////////////
    //                        LET's GO !                        //
    // ///////////////////////////////////////////////////////////
    
    
    begin_screen(sh, tt);
    
    float dt = 0.01f;
    for (int i = -3; i < sh.num_rows(); ++i)
    {
      for (int j = -5; j < sh.num_cols(); ++j)
      {
        sprite0->pos.r = i;
        sprite0->pos.c = i%2==0 ? j : 35-j;
        
        int anim_frame = (i + 3)*44 + (j + 5);
        auto t = static_cast<float>(anim_frame)/(23.f*45.f);
        sprite1->pos.c = math::roundI(20.f + 10.f*std::cos(math::c_2pi * 7.13 * t));
        sprite1->pos.r = math::roundI(10.f + 5*std::sin(math::c_2pi * 10 * t));
        
        for (int a_idx = 0; a_idx < asteroids.size(); ++a_idx)
        {
          auto& [ast_sprite, r_pos, r_vel] = asteroids[a_idx];
          r_pos += r_vel * dt;
          if (r_pos >= static_cast<float>(sh.num_rows()))
            r_pos = 0.f;
          ast_sprite->pos.r = math::roundI(r_pos);
        }
        
        return_cursor();
        sh.clear();
        sprh.draw(sh, anim_frame);
        sh.print_screen_buffer(tt, Color::Black);
        Delay::sleep(0'200'000);
        
        kpd = keyboard->readKey();
        auto key = keyboard::get_char_key(kpd);
        auto lo_key = str::to_lower(key);
        if (lo_key == 'q')
          goto quit;
      }
    }
    
quit:
    end_screen();
  }

}
