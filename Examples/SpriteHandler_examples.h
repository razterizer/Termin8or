//
//  SpriteHandler_tests.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-16.
//

#pragma once
#include "../SpriteHandler.h"
#include "../ScreenUtils.h"
#include "../Dynamics/CollisionHandler.h"
#include "../Dynamics/DynamicsSystem.h"
#include <Core/Benchmark.h>

bool use_dynamics_system = true;
bool dbg_draw_sprites = false;
bool dbg_draw_rigid_bodies = false;
bool dbg_draw_broad_phase = false;
bool dbg_draw_narrow_phase = false;
bool draw_sprites = true;

namespace sprite_handler
{

  void example1()
  {
    ScreenHandler<20, 40> sh;
    SpriteHandler sprh;
    keyboard::KeyPressDataPair kpdp;
    auto keyboard = std::make_unique<keyboard::StreamKeyboard>();
    
    dynamics::DynamicsSystem dyn_sys;
    dynamics::CollisionHandler coll_handler;
    
    rnd::srand_time();
    
    // //////////////////////////////
    
    auto* sprite0 = sprh.create_bitmap_sprite("spaceship");
    sprite0->layer_id = 4;
    sprite0->init(4, 5);
    sprite0->create_frame(0);
    sprite0->set_sprite_chars_from_strings(0,
      R"(  _  )",
      R"( /#\ )",
      R"([---])",
      R"(  #  )"
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
    sprite0->set_sprite_materials(0,
      0, 0, 0, 0, 0,
      0, 1, 1, 1, 0,
      1, 1, 1, 1, 1,
      0, 0, 0, 0, 0
    );
    ttl::Rectangle bb0 { 3, 2, 1, 1 };
    sprite0->clone_frame(1, 0);
    sprite0->set_sprite_chars(1, bb0, '#');
    sprite0->set_sprite_fg_colors(1, bb0, Color::Yellow);
    sprite0->set_sprite_bg_colors(1, bb0, Color::DarkYellow);
    sprite0->clone_frame(2, 0);
    sprite0->set_sprite_chars(2, bb0, '^');
    sprite0->set_sprite_fg_colors(2, bb0, Color::White);
    sprite0->set_sprite_bg_colors(2, bb0, Color::Transparent2);
    sprite0->func_calc_anim_frame = [](int sim_frame)
    {
      auto anim = sim_frame % 14;
      if (anim < 8)
        return 2;
      return anim % 2;
    };
    
    sprite0->pos = { 17, 8 };
    dyn_sys.add_rigid_body(sprite0, 1.f, std::nullopt, { -15.f, 2.5f }, { 6.f, 0.f });
    
    // ///////////////////////////////////////////////////////////
    
    auto* sprite1 = sprh.create_bitmap_sprite("alien");
    sprite1->layer_id = 5;
    sprite1->init(2, 3);
    sprite1->create_frame(0);
    sprite1->set_sprite_chars(0,
      ' ', 'Y', ' ',
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
    sprite1->set_sprite_materials(0,
      0, 1, 0,
      1, 1, 1
    );
    ttl::Rectangle bbL { 1, 0, 1, 1 }, bbR { 1, 2, 1, 1 };
    sprite1->clone_frame(1, 0);
    sprite1->set_sprite_chars(1, bbL, '#');
    sprite1->set_sprite_chars(1, bbR, '#');
    sprite1->set_sprite_fg_colors(1, bbL, Color::DarkYellow);
    sprite1->set_sprite_fg_colors(1, bbR, Color::DarkYellow);
    sprite1->func_calc_anim_frame = [](int sim_frame)
    {
      auto anim = sim_frame % 40;
      if (anim < 34)
        return 0;
      return anim % 2;
    };
    
    // ///////////////////////////////////////////////////////////
    
    std::array<std::tuple<Sprite*, float, float>, 16> asteroids;
    for (int a_idx = 0; a_idx < stlutils::sizeI(asteroids); ++a_idx)
    {
      auto* sprite2 = sprh.create_bitmap_sprite("asteroid" + std::to_string(a_idx));
      sprite2->pos.r = rnd::rand_int(0, sh.num_rows()-1);
      sprite2->pos.c = rnd::rand_int(0, sh.num_cols()-1);
      sprite2->layer_id = rnd::rand_select<int>({ 1, 3 });
      sprite2->init(1, 1);
      sprite2->create_frame(0);
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
      auto* sprite3 = sprh.create_bitmap_sprite("star" + std::to_string(s_idx));
      sprite3->pos.r = rnd::rand_int(0, sh.num_rows()-1);
      sprite3->pos.c = rnd::rand_int(0, sh.num_cols()-1);
      sprite3->layer_id = 0;
      sprite3->init(1, 1);
      sprite3->create_frame(0);
      char star_ch = rnd::rand_select<char>({ '.', '+' });
      sprite3->set_sprite_chars(0, star_ch);
      sprite3->set_sprite_fg_colors(0, rnd::rand_select<Color>({ Color::White, Color::White, Color::White, Color::White, Color::White, Color::Yellow, Color::Yellow, Color::Yellow, Color::Red, Color::Blue, Color::Blue, Color::Blue }));
      sprite3->set_sprite_bg_colors(0, Color::Transparent2);
      sprite3->create_frame(1);
      sprite3->set_sprite_chars(1, star_ch);
      sprite3->set_sprite_fg_colors(1, Color::Black);
      sprite3->set_sprite_bg_colors(1, Color::Transparent2);
      const int max_twinkle = 100;
      int twinkle_offs = rnd::rand_int(0, max_twinkle);
      sprite3->func_calc_anim_frame = [twinkle_offs](int sim_frame)
      {
        if ((sim_frame + twinkle_offs) % max_twinkle == 0)
          return 1;
        return 0;
      };
    }
    
    auto* sprite4 = sprh.create_bitmap_sprite("background");
    sprite4->layer_id = 2;
    sprite4->init(sh.num_rows(), sh.num_cols());
    sprite4->create_frame(0);
    // sprite4->save_frame(0, "background.tex");
    sprite4->load_frame(0, "background.tex");
    
    // ///////////////////////////////////////////////////////////
    //                        LET's GO !                        //
    // ///////////////////////////////////////////////////////////
        
    begin_screen();
    
    float dt = 0.01f;
    for (int i = -3; i < sh.num_rows(); ++i)
    {
      for (int j = -5; j < sh.num_cols(); ++j)
      {
        if (!use_dynamics_system)
        {
          sprite0->pos.r = i;
          sprite0->pos.c = i%2==0 ? j : 35-j;
        }
        
        int anim_frame = (i + 3)*44 + (j + 5);
        auto t = static_cast<float>(anim_frame)/(23.f*45.f);
        sprite1->pos.c = math::roundI(20.f + 10.f*std::cos(math::c_2pi * 7.13 * t));
        sprite1->pos.r = math::roundI(10.f + 5*std::sin(math::c_2pi * 10 * t));
        
        for (int a_idx = 0; a_idx < stlutils::sizeI(asteroids); ++a_idx)
        {
          auto& [ast_sprite, r_pos, r_vel] = asteroids[a_idx];
          r_pos += r_vel * dt;
          if (r_pos >= static_cast<float>(sh.num_rows()))
            r_pos = 0.f;
          ast_sprite->pos.r = math::roundI(r_pos);
        }
        
        if (use_dynamics_system)
          dyn_sys.update(anim_frame*dt*2.f, dt*2.f, anim_frame);
        return_cursor();
        sh.clear();
        if (dbg_draw_rigid_bodies)
          dyn_sys.draw_dbg(sh);
        if (dbg_draw_sprites)
          sprh.draw_dbg_pts(sh, anim_frame);
        if (draw_sprites)
          sprh.draw(sh, anim_frame);
        if (dbg_draw_sprites)
          sprh.draw_dbg_bb(sh, anim_frame);
        sh.print_screen_buffer(Color::Black);
        Delay::sleep(0'400'000);
        
        kpdp = keyboard->readKey();
        auto key = keyboard::get_char_key(kpdp.transient);
        auto lo_key = str::to_lower(key);
        if (lo_key == 'q' || sprite0->pos.r > sh.num_rows())
          goto quit;
      }
    }
    
quit:
    end_screen(sh);
  }
  
  void example2()
  {
    ScreenHandler<20, 40> sh;
    SpriteHandler sprh;
    keyboard::KeyPressDataPair kpdp;
    auto keyboard = std::make_unique<keyboard::StreamKeyboard>();
    
    dynamics::DynamicsSystem dyn_sys;
    dynamics::CollisionHandler coll_handler;
    
    rnd::srand_time();
    
    // //////////////////////////////
    
    auto* sprite0 = sprh.create_vector_sprite("spaceship");
    sprite0->layer_id = 1;
    sprite0->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite0->add_line_segment(0, { 2, 2 }, { -2, 0 }, 'o', { Color::Yellow, Color::Transparent2 }, 1);
    sprite0->add_line_segment(0, { -2, 0 }, { 2, -2 }, 'o', { Color::Yellow, Color::Transparent2 }, 1);
    sprite0->add_line_segment(0, { 2, -2 }, { 2, 2 }, 'o', { Color::Yellow, Color::Transparent2 }, 1);
    sprite0->set_rotation(0.f);
    sprite0->finalize_topology(0);
    auto* frame = sprite0->get_curr_local_frame(0);
    frame->fill_closed_polylines = true;
    frame->fill_char = '#';
    frame->fill_style = { Color::LightGray, Color::DarkGray };
    dyn_sys.add_rigid_body(sprite0, 4.f, std::nullopt, { 1.f, -3.f }, {}, 2.f);
    
    auto* sprite1 = sprh.create_vector_sprite("alien");
    sprite1->layer_id = 2;
    sprite1->pos = { math::roundI(sh.num_rows()*0.75f), math::roundI(sh.num_cols()*0.25f) };
    sprite1->add_line_segment(0, { 1, -0.8f }, { 1, 0.8f }, '"', { Color::Green, Color::Transparent2 }, 1);
    sprite1->add_line_segment(0, { 0, 0, }, { 0, 0 }, 'O', { Color::Cyan, Color::Transparent2 }, 1);
    dyn_sys.add_rigid_body(sprite1, 1.f, std::nullopt, { -5.5f, 8.5f }, {}, -1.f);
    
    coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
    
    // ///////////////////////////////////////////////////////////
    //                        LET's GO !                        //
    // ///////////////////////////////////////////////////////////
    
    const int delay = 20'000;
    const float dt = 1e-6f * delay;
    
    begin_screen();
    
    benchmark::tic();
    
    int anim_frame = 0;
    const int num_frames = 2000;
    int i;
    for (i = 0; i < num_frames; ++i)
    {
      if (use_dynamics_system)
      {
        dyn_sys.update(i*dt, dt, anim_frame);
        coll_handler.update();
      }
      else
      {
        float t = i / 800.f;
        float ang = t*360.f*2.f;
        sprite0->set_rotation(ang);
        sprite1->set_rotation(-ang*0.8f);
      }
      
      return_cursor();
      sh.clear();
      if (dbg_draw_rigid_bodies)
        dyn_sys.draw_dbg(sh);
      if (dbg_draw_sprites)
        sprh.draw_dbg_pts(sh, anim_frame);
      if (dbg_draw_narrow_phase)
        coll_handler.draw_dbg_narrow_phase(sh);
      if (draw_sprites)
        sprh.draw(sh, anim_frame);
      if (dbg_draw_sprites)
        sprh.draw_dbg_bb(sh, anim_frame);
      if (dbg_draw_broad_phase)
        coll_handler.draw_dbg_broad_phase(sh, 0);
      sh.print_screen_buffer(Color::Black);
      Delay::sleep(delay);
      
      kpdp = keyboard->readKey();
      auto key = keyboard::get_char_key(kpdp.transient);
      auto lo_key = str::to_lower(key);
      if (lo_key == 'q')
        goto quit;
    }
    
quit:
    auto dur_s = 1e-3f * benchmark::toc();
    end_screen(sh);
    
    auto fps = i / dur_s;
    std::cout << "FPS = " << fps << std::endl;
  }

}
