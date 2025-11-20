//
//  GameEngine.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2023-12-30.
//

#pragma once
#include "Logging.h"
#include "../input/Keyboard.h"
#include "../screen/ScreenCommands.h"
#include "../screen/ScreenUtils.h"
#include <Core/Delay.h>
#include <Core/Rand.h>
#include <Core/MathUtils.h>
#include <Core/FolderHelper.h>
#include <Core/OneShot.h>
#include <Core/Benchmark.h>
#include <chrono>
#include <sstream>

namespace t8x
{
  using Color16 = t8::Color16;
  using Style = t8::Style;
  using ButtonStyle = t8::ButtonStyle;
  using PromptStyle = t8::PromptStyle;
  using HiliteFGStyle = t8::HiliteFGStyle;


  struct GameEngineParams
  {
    bool enable_title_screen = true;
    bool enable_instructions_screen = true;
    bool enable_quit_confirm_screen = true;
    bool quit_confirm_unsaved_changes = false;
    bool enable_hiscores = true;
    bool enable_pause = true;
    bool enable_terminal_window_resize = true; // If true it will resize the terminal window if too small for the game screen.
    
    Color16 screen_bg_color_default = Color16::Default;
    Color16 screen_bg_color_title = Color16::Default;
    Color16 screen_bg_color_instructions = Color16::Default;
    
    std::optional<t8::Color16> screen_bg_color_paused = std::nullopt;
    t8::Style pause_info_style { t8::Color16::White, t8::Color16::DarkCyan };
    
    std::optional<t8::Color16> screen_bg_color_quit_confirm = t8::Color16::DarkCyan;
    Style quit_confirm_title_style { Color16::Black, Color16::DarkCyan };
    ButtonStyle quit_confirm_button_style { Color16::Black, Color16::DarkCyan, Color16::Cyan };
    Style quit_confirm_info_style { Color16::White, Color16::DarkCyan };
    
    std::optional<Color16> screen_bg_color_input_hiscore = Color16::DarkGray;
    Style input_hiscore_title_style { Color16::Green, Color16::Black };
    PromptStyle input_hiscore_prompt_style { Color16::Green, Color16::Black, Color16::DarkGreen };
    Style input_hiscore_info_style { Color16::DarkGreen, Color16::Black };
    
    std::optional<Color16> screen_bg_color_hiscores = Color16::DarkGray;
    Style hiscores_title_style { Color16::Green, Color16::Black };
    HiliteFGStyle hiscores_nr_style { Color16::Green, Color16::Black, Color16::Cyan };
    HiliteFGStyle hiscores_score_style { Color16::Green, Color16::Black, Color16::Cyan };
    HiliteFGStyle hiscores_name_style { Color16::Green, Color16::Black, Color16::Cyan };
    Style hiscores_info_style { Color16::DarkGreen, Color16::Black };
    
    Style game_over_line_0_style { Color16::DarkRed, Color16::White };
    Style game_over_line_1_style { Color16::DarkRed, Color16::Yellow };
    Style game_over_line_2_style { Color16::DarkRed, Color16::DarkYellow };
    Style game_over_line_3_style { Color16::DarkRed, Color16::Yellow };
    Style game_over_line_4_style { Color16::DarkRed, Color16::White };
    
    Style you_won_line_0_style { Color16::DarkBlue, Color16::Cyan };
    Style you_won_line_1_style { Color16::DarkBlue, Color16::DarkCyan };
    
    LogMode log_mode = LogMode::None;
    std::string xcode_log_path; // e.g. "../../../../../../../../Documents/xcode/Asciiroids/Asciiroids/bin/"
    std::string log_filename = "rec.txt";
    
    bool suppress_tty_output = false;
    bool suppress_tty_input = false;
    
    bool enable_benchmark = false;
    t8::DrawPolicy draw_policy = t8::DrawPolicy::MEASURE_SELECT;
  };
  
  template<int NR = 30, int NC = 80>
  class GameEngine
  {
    bool paused = false;
    bool show_title = true;
    bool show_instructions = false;
    bool show_quit_confirm = false;
    bool show_game_over = false;
    bool show_you_won = false;
    bool show_input_hiscore = false;
    bool show_hiscores = false;
    
    std::string_view path_to_exe; // Includes the <program>.exe file.
    std::string exe_file; // Only the <program>.exe file.
    std::string exe_path; // Excludes the <program>.exe file.
    GameEngineParams m_params;
    
    unsigned int curr_rnd_seed = 0;
    
    // Simulation delay.
    int sim_delay = 50'000; // 100'000 (10 FPS) // 60'000 (16.67 FPS);
    // Real-time FPS.
    float real_fps = 12.f; // 5
    
    float sim_dt_s = static_cast<float>(sim_delay) / 1e6f;
    float sim_time_s = 0.f;
    double real_time_s = 0.;
    double real_last_time_s = 0.;
    double real_dt_s = 0.;
    benchmark::TicTocTimer tictoc_game_engine;
    
    struct AnimCtrData
    {
      int anim_count_per_frame_count = 1;
      int anim_ctr = 0;
    };
    std::vector<AnimCtrData> anim_ctr_data;
    int frame_ctr = 0;
    int frame_ctr_measure = 0;
    
    YesNoButtons quit_confirm_button = YesNoButtons::No;
    
    std::vector<HiScoreItem> hiscore_list;
    int score = 0;
    HiScoreItem curr_score_item;
    int hiscore_caret_idx = 0;
    
    int term_win_rows = 0;
    int term_win_cols = 0;
    
    OneShot trg_update_halted, trg_update_resumed;
    
    bool handle_hiscores(const HiScoreItem& curr_hsi)
    {
      const int c_max_num_hiscores = 20;
      const std::string c_file_path = folder::join_file_path({ exe_path, "hiscores.txt" });
      
      // Read saved hiscores.
      std::vector<std::string> lines;
      if (std::filesystem::exists(c_file_path))
      {
        if (!TextIO::read_file(c_file_path, lines))
          return false;
      }
      
      // Import saved hiscores from vector of strings.
      hiscore_list.clear();
      for (const auto& hs_str : lines)
      {
        std::istringstream iss(hs_str);
        HiScoreItem hsi;
        iss >> hsi.name >> hsi.score;
        hiscore_list.emplace_back(hsi);
      }
      
      // Add current hiscore.
      hiscore_list.push_back(curr_hsi);
      
      // Sort hiscores.
      auto num_hiscores = static_cast<int>(hiscore_list.size());
      stlutils::sort(hiscore_list,
                     [](const auto& hsi_A, const auto& hsi_B) { return hsi_A.score > hsi_B.score; });
      if (num_hiscores >= 1)
        hiscore_list = stlutils::subset(hiscore_list, 0, std::min(num_hiscores, c_max_num_hiscores) - 1);
      else
      {
        std::cerr << "ERROR: Unknown error while saving hiscores!" << std::endl;
        return false;
      }
      
      // Export hiscores to vector of strings.
      lines.clear();
      for (const auto& hsi : hiscore_list)
      {
        std::ostringstream oss;
        oss << str::trim_ret(hsi.name) << " " << hsi.score;
        lines.emplace_back(oss.str());
      }
      
      // Save hiscores.
      if (!TextIO::write_file(c_file_path, lines))
        return false;
      
      return true;
    }
    
  protected:
    std::chrono::time_point<std::chrono::steady_clock> real_start_time_s;
    OneShot time_inited;
    
    t8::ScreenHandler<NR, NC> sh;
    
    Color bg_color = Color16::Default;
    
    t8::KeyPressDataPair kpdp;
    std::unique_ptr<t8::StreamKeyboard> keyboard;
    
    bool exit_requested = false;
    
    unsigned int get_curr_rnd_seed() const { return curr_rnd_seed; }
    void set_curr_rnd_seed(unsigned int new_seed)
    {
      curr_rnd_seed = new_seed;
      rnd::srand(curr_rnd_seed);
    }
    
    int& ref_score() { return score; }
    
    double get_real_time_s() const { return real_time_s; }
    double get_real_dt_s() const { return real_dt_s; }
    
    float get_sim_time_s() const { return sim_time_s; }
    float get_sim_dt_s() const { return sim_dt_s; }
    
    void set_anim_rate(int anim_channel, int val)
    {
      if (anim_channel < 0)
        return;
      anim_channel++; // Reserving the true index 0 for system animations.
      auto& ad = stlutils::at_growing(anim_ctr_data, anim_channel);
      ad.anim_count_per_frame_count = std::max(1, val);
    }
    int get_anim_rate(int anim_channel)
    {
      if (anim_channel < 0)
        return 1;
      anim_channel++; // Reserving the true index 0 for system animations.
      auto& ad = stlutils::at_growing(anim_ctr_data, anim_channel);
      return ad.anim_count_per_frame_count;
    }
    int get_frame_count() const { return frame_ctr; }
    int get_frame_count_measure() const { return frame_ctr_measure; }
    int get_anim_count(int anim_channel)
    {
      if (anim_channel < 0)
        return 1;
      anim_channel++; // Reserving the true index 0 for system animations.
      auto& ad = stlutils::at_growing(anim_ctr_data, anim_channel);
      return ad.anim_ctr;
    }
    
    std::string get_exe_folder() const { return exe_path; }
    
    void enable_quit_confirm_screen(bool enable)
    {
      m_params.enable_quit_confirm_screen = enable;
    }
    
    // Callbacks
    virtual void update() = 0;
    virtual void draw_title() {}
    virtual void draw_instructions() {}
    virtual void on_quit() {}
    virtual void on_exit_title() {}
    virtual void on_exit_instructions() {}
    virtual void on_enter_game_loop() {}
    virtual void on_exit_game_loop() {}
    virtual void on_enter_game_over() {}
    virtual void on_halt_game_loop() {}
    virtual void on_resume_game_loop() {}
    virtual void on_exit_game_over() {}
    virtual void on_enter_you_won() {}
    virtual void on_exit_you_won() {}
    virtual void on_enter_input_hiscore() {}
    virtual void on_exit_input_hiscore() {}
    virtual void on_enter_hiscores() {}
    virtual void on_enter_paused() {}
    virtual void on_exit_paused() {}
    
  public:
    GameEngine(std::string_view exe_full_path,
               const GameEngineParams& params)
      : path_to_exe(exe_full_path)
      , m_params(params)
      , anim_ctr_data(1)
    {
      std::tie(exe_path, exe_file) = folder::split_file_path(std::string(path_to_exe));
      show_title = params.enable_title_screen;
      if (!show_title)
        show_instructions = params.enable_instructions_screen;
    }
    
    virtual ~GameEngine() = default;
    
    void init()
    {
      if (exit_requested)
        return;
      
      if (!m_params.suppress_tty_input)
      {
        keyboard = std::make_unique<t8::StreamKeyboard>();
        keyboard->set_held_buffer_size_from_fps(real_fps);
      }
      
      t8::begin_screen();
      
      if (m_params.enable_terminal_window_resize)
      {
        std::tie(term_win_rows, term_win_cols) = t8::get_terminal_window_size();
        int new_rows = term_win_rows;
        int new_cols = term_win_cols;
        math::maximize(new_rows, NR + 1);
        math::maximize(new_cols, NC);
        t8::resize_terminal_window(new_rows, new_cols);
      }
      
      //nodelay(stdscr, TRUE);
      
      curr_rnd_seed = rnd::srand_time();
      
      t8x::setup_logging(m_params.log_mode, get_exe_folder(), m_params.xcode_log_path, m_params.log_filename, curr_rnd_seed);
      
      if (m_params.enable_benchmark)
        benchmark::tic(tictoc_game_engine);
      
      if (time_inited.once())
        real_start_time_s = std::chrono::steady_clock::now();
    }
    
    virtual void generate_data() = 0;
    
    void run()
    {
      if (exit_requested)
        return;
      
      // RT-Loop
      t8::clear_screen();
      on_enter_game_loop();
      auto update_func = std::bind(&GameEngine::engine_update, this);
      Delay::update_loop(real_fps, update_func);
      on_exit_game_loop();
    }
    
    float get_real_fps() const { return real_fps; }
    void set_real_fps(float fps_val)
    {
      if (keyboard != nullptr)
        keyboard->set_held_buffer_size_from_fps(fps_val);
      anim_ctr_data[0].anim_count_per_frame_count = math::roundI(fps_val / 5);
      real_fps = fps_val;
    }
    int get_sim_delay_us() const { return sim_delay; }
    // Used for dynamics and stuff.
    void set_sim_delay_us(float delay_us)
    {
      sim_delay = math::roundI(delay_us);
      sim_dt_s = static_cast<float>(sim_delay) / 1e6f;
    }
    
    void set_state_game_over() { show_game_over = true; }
    void set_state_you_won() { show_you_won = true; }
    
    void request_exit() { exit_requested = true; }
    
    void set_screen_bg_color_default(Color16 bg_color) { m_params.screen_bg_color_default = bg_color; }
    
  private:
    void pre_quit()
    {
      float dur_s = 0.f;
      if (m_params.enable_benchmark)
        dur_s = 1e-3f * benchmark::toc(tictoc_game_engine);
    
      end_screen(sh);
      
      if (m_params.enable_terminal_window_resize)
        if (term_win_rows > 0 && term_win_cols > 0)
          t8::resize_terminal_window(term_win_rows, term_win_cols);
          
      if (m_params.enable_benchmark && 0.f < dur_s)
      {
        auto avg_fps = frame_ctr / dur_s;
        std::cout << "Goal FPS = " << real_fps << std::endl;
        std::cout << "Average FPS = " << avg_fps << std::endl;
        std::cout << "# Full Redraws = " << sh.get_num_full_redraws() << std::endl;
        std::cout << "# Partial Redraws = " << sh.get_num_partial_redraws() << std::endl;
      }
      
      on_quit();
    }
    
    bool engine_update()
    {
      if (exit_requested)
        return false;
      
      if (time_inited.was_triggered())
      {
        auto curr_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = curr_time - real_start_time_s;
        real_last_time_s = real_time_s;
        real_time_s = elapsed_seconds.count();
        real_dt_s = real_time_s - real_last_time_s;
      }
      
      if (!m_params.suppress_tty_output)
        t8::return_cursor();
      sh.clear();
      
      t8x::update_log_stream(m_params.log_mode, kpdp, keyboard.get(), get_frame_count());
      auto key = t8::get_char_key(kpdp.transient);
      auto lo_key = str::to_lower(key);
      auto quit = lo_key == 'q';
      auto pause = lo_key == 'p';
      
      if (quit)
      {
        math::toggle(show_quit_confirm);
        quit_confirm_button = YesNoButtons::No;
      }
      else if (m_params.enable_pause && pause)
      {
        math::toggle(paused);
        if (paused)
          on_enter_paused();
        else
          on_exit_paused();
      }
      
      if (!m_params.enable_quit_confirm_screen && quit)
      {
        pre_quit();
        return false;
      }
      else if (show_quit_confirm && !show_hiscores && !show_input_hiscore)
      {
        bg_color = m_params.screen_bg_color_quit_confirm.value_or(bg_color);
        
        std::vector<std::string> titles;
        if (m_params.quit_confirm_unsaved_changes)
          titles.emplace_back("You have unsaved changes!");
        titles.emplace_back("Are you sure you want to quit?");
        
        auto special_key = t8::get_special_key(kpdp.transient);
        
        draw_confirm(sh, titles, quit_confirm_button,
                     m_params.quit_confirm_title_style,
                     m_params.quit_confirm_button_style,
                     m_params.quit_confirm_info_style);
        if (special_key == t8::SpecialKey::Left)
          quit_confirm_button = YesNoButtons::Yes;
        else if (special_key == t8::SpecialKey::Right)
          quit_confirm_button = YesNoButtons::No;
        
        if (special_key == t8::SpecialKey::Enter)
        {
          if (quit_confirm_button == YesNoButtons::Yes)
          {
            pre_quit();
            return false;
          }
          else
            show_quit_confirm = false;
        }
      }
      else
      {
        bg_color = m_params.screen_bg_color_default;
        if (m_params.enable_title_screen && show_title)
        {
          bg_color = m_params.screen_bg_color_title;
          draw_title();
          if (key == ' ')
          {
            on_exit_title();
            show_title = false;
            if (m_params.enable_instructions_screen)
              show_instructions = true;
          }
        }
        else if (m_params.enable_instructions_screen && show_instructions)
        {
          bg_color = m_params.screen_bg_color_instructions;
          draw_instructions();
          if (key == ' ')
          {
            on_exit_instructions();
            show_instructions = false;
          }
        }
        else if (show_game_over)
        {
          if (game_over_timer == 0)
            draw_game_over(sh, 0.1f*(7.f/real_fps),
                           m_params.game_over_line_0_style,
                           m_params.game_over_line_1_style,
                           m_params.game_over_line_2_style,
                           m_params.game_over_line_3_style,
                           m_params.game_over_line_4_style);
          else
          {
            game_over_timer--;
            if (game_over_timer == 0)
            {
              on_enter_game_over();
              timestamp_game_over = real_time_s;
            }
          }
          
          update();
          
          if (m_params.enable_hiscores && key == ' ' &&
              (real_time_s - timestamp_game_over > c_min_time_game_over))
          {
            on_exit_game_over();
            show_game_over = false;
            show_input_hiscore = true;
            curr_score_item.init(score);
            hiscore_caret_idx = 0;
            on_enter_input_hiscore();
          }
        }
        else if (show_you_won)
        {
          if (you_won_timer == 0)
            draw_you_won(sh, 0.07f*(7.f/real_fps),
                         m_params.you_won_line_0_style,
                         m_params.you_won_line_1_style);
          else
          {
            you_won_timer--;
            if (you_won_timer == 0)
            {
              on_enter_you_won();
              timestamp_you_won = real_time_s;
            }
          }
          
          update();
          
          if (m_params.enable_hiscores && key == ' ' &&
              (real_time_s - timestamp_you_won > c_min_time_you_won))
          {
            on_exit_you_won();
            show_you_won = false;
            show_input_hiscore = true;
            curr_score_item.init(score);
            hiscore_caret_idx = 0;
            on_enter_input_hiscore();
          }
        }
        else if (show_input_hiscore)
        {
          bg_color = m_params.screen_bg_color_input_hiscore.value_or(bg_color);
          if (draw_input_hiscore(sh, kpdp.transient, curr_score_item, hiscore_caret_idx, anim_ctr_data[0].anim_ctr,
                                 m_params.input_hiscore_title_style,
                                 m_params.input_hiscore_prompt_style,
                                 m_params.input_hiscore_info_style))
          {
            on_exit_input_hiscore();
            handle_hiscores(curr_score_item);
            show_input_hiscore = false;
            show_hiscores = true;
            on_enter_hiscores();
          }
        }
        else if (show_hiscores)
        {
          bg_color = m_params.screen_bg_color_hiscores.value_or(bg_color);
          draw_hiscores(sh, hiscore_list,
                        m_params.hiscores_title_style,
                        m_params.hiscores_nr_style,
                        m_params.hiscores_score_style,
                        m_params.hiscores_name_style,
                        m_params.hiscores_info_style);
          
          if (key == ' ' || quit)
          {
            pre_quit();
            return false;
          }
        }
        else if (paused)
        {
          bg_color = m_params.screen_bg_color_paused.value_or(bg_color);
          draw_paused(sh, anim_ctr_data[0].anim_ctr, m_params.pause_info_style);
        }
        else
          update();
      }
      
      if (!m_params.suppress_tty_output)
      {
        sh.print_screen_buffer(bg_color, m_params.draw_policy);
        //sh.print_screen_buffer_chars();
        //sh.print_screen_buffer_fg_colors();
        //sh.print_screen_buffer_bg_colors();
      }
      
      ///
      
      frame_ctr++;
      if (frame_ctr % anim_ctr_data[0].anim_count_per_frame_count == 0)
        anim_ctr_data[0].anim_ctr++;
      
      if (!show_title && !show_instructions && !show_quit_confirm && !show_input_hiscore && !show_hiscores && !paused)
      {
        if (trg_update_resumed.once())
        {
          on_resume_game_loop();
          trg_update_halted.reset();
        }
        
        frame_ctr_measure++;
        
        for (size_t ad_idx = 0; ad_idx < anim_ctr_data.size(); ++ad_idx)
          if (frame_ctr_measure % anim_ctr_data[ad_idx].anim_count_per_frame_count == 0)
            anim_ctr_data[ad_idx].anim_ctr++;
        
        sim_time_s += sim_dt_s;
      }
      else
      {
        if (trg_update_halted.once())
        {
          on_halt_game_loop();
          trg_update_resumed.reset();
        }
      }
      
      if (t8x::log_finished)
        exit(EXIT_SUCCESS);
      
      return true;
    }
  };

}
