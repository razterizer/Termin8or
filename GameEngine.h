//
//  GameEngine.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2023-12-30.
//

#pragma once
#include "Keyboard.h"
#include "ScreenUtils.h"
#include <Core/Delay.h>
#include <Core/Rand.h>
#include <Core/Math.h>
#include <Core/FolderHelper.h>
#include <Core/OneShot.h>
#include <chrono>
#include <sstream>

struct GameEngineParams
{
  bool enable_title_screen = true;
  bool enable_instructions_screen = true;
  bool enable_quit_confirm_screen = true;
  bool quit_confirm_unsaved_changes = false;
  bool enable_hiscores = true;
  bool enable_pause = true;
  bool enable_terminal_window_resize = true; // If true it will resize the terminal window if too small for the game screen.
  
  Color screen_bg_color_default = Color::Default;
  Color screen_bg_color_title = Color::Default;
  Color screen_bg_color_instructions = Color::Default;
  
  std::optional<Color> screen_bg_color_paused = std::nullopt;
  
  std::optional<Color> screen_bg_color_quit_confirm = Color::DarkCyan;
  styles::Style quit_confirm_title_style { Color::Black, Color::DarkCyan };
  styles::ButtonStyle quit_confirm_button_style { Color::Black, Color::DarkCyan, Color::Cyan };
  styles::Style quit_confirm_info_style { Color::White, Color::DarkCyan };
  
  std::optional<Color> screen_bg_color_input_hiscore = Color::DarkGray;
  styles::Style input_hiscore_title_style { Color::Green, Color::Black };
  styles::PromptStyle input_hiscore_prompt_style { Color::Green, Color::Black, Color::DarkGreen };
  styles::Style input_hiscore_info_style { Color::DarkGreen, Color::Black };
  
  std::optional<Color> screen_bg_color_hiscores = Color::DarkGray;
  styles::Style hiscores_title_style { Color::Green, Color::Black };
  styles::HiliteFGStyle hiscores_nr_style { Color::Green, Color::Black, Color::Cyan };
  styles::HiliteFGStyle hiscores_score_style { Color::Green, Color::Black, Color::Cyan };
  styles::HiliteFGStyle hiscores_name_style { Color::Green, Color::Black, Color::Cyan };
  styles::Style hiscores_info_style { Color::DarkGreen, Color::Black };
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
  
  // Simulation delay.
  int sim_delay = 50'000; // 100'000 (10 FPS) // 60'000 (16.67 FPS);
  // Real-time FPS.
  float real_fps = 12.f; // 5
  
  float sim_dt_s = static_cast<float>(sim_delay) / 1e6f;
  float sim_time_s = 0.f;
  double real_time_s = 0.;
  double real_last_time_s = 0.;
  double real_dt_s = 0.;
  
  YesNoButtons quit_confirm_button = YesNoButtons::No;
  
  std::vector<HiScoreItem> hiscore_list;
  int score = 0;
  HiScoreItem curr_score_item;
  int hiscore_caret_idx = 0;
  
  int term_win_rows = 0;
  int term_win_cols = 0;
  
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
  
  Text t;
  ScreenHandler<NR, NC> sh;
  
  Color bg_color = Color::Default;
  
  int anim_ctr = 0;
  
  keyboard::KeyPressData kpd;
  std::unique_ptr<keyboard::StreamKeyboard> keyboard;
  
  bool exit_requested = false;
  
  unsigned int curr_rnd_seed = 0;
  
  void set_real_fps(float fps_val) { real_fps = fps_val; }
  
  // Used for dynamics and stuff.
  void set_sim_delay_us(float delay_us)
  {
    sim_delay = math::roundI(delay_us);
    sim_dt_s = static_cast<float>(sim_delay) / 1e6f;
  }
  
  int& ref_score() { return score; }
  
  double get_real_time_s() const { return real_time_s; }
  double get_real_dt_s() const { return real_dt_s; }
  
  float get_sim_time_s() const { return sim_time_s; }
  float get_sim_dt_s() const { return sim_dt_s; }
  
  std::string get_exe_folder() const { return exe_path; }
  
  void enable_quit_confirm_screen(bool enable)
  {
    m_params.enable_quit_confirm_screen = enable;
  }
  
  // Callbacks
  virtual void update() = 0;
  virtual void draw_title() = 0;
  virtual void draw_instructions() = 0;
  virtual void on_quit() {}
  virtual void on_exit_title() {}
  virtual void on_exit_instructions() {}
  virtual void on_enter_game_over() {}
  virtual void on_exit_game_over() {}
  virtual void on_enter_you_won() {}
  virtual void on_exit_you_won() {}
  virtual void on_enter_input_hiscore() {}
  virtual void on_exit_input_hiscore() {}
  virtual void on_enter_hiscores() {}
  
public:
  GameEngine(std::string_view exe_full_path,
             const GameEngineParams& params)
    : path_to_exe(exe_full_path)
    , m_params(params)
  {
    std::tie(exe_path, exe_file) = folder::split_file_path(std::string(path_to_exe));
  }
  
  virtual ~GameEngine() = default;

  void init()
  {
    if (exit_requested)
      return;
    
    keyboard = std::make_unique<keyboard::StreamKeyboard>();
    
    begin_screen(sh, t);
    
    if (m_params.enable_terminal_window_resize)
    {
      std::tie(term_win_rows, term_win_cols) = get_terminal_window_size();
      int new_rows = term_win_rows;
      int new_cols = term_win_cols;
      math::maximize(new_rows, NR + 1);
      math::maximize(new_cols, NC);
      resize_terminal_window(new_rows, new_cols);
    }
    
    //nodelay(stdscr, TRUE);
    
    curr_rnd_seed = rnd::srand_time();
    
    if (time_inited.once())
      real_start_time_s = std::chrono::steady_clock::now();
  }
  
  virtual void generate_data() = 0;
  
  void run()
  {
    if (exit_requested)
      return;
      
    // RT-Loop
    clear_screen();
    auto update_func = std::bind(&GameEngine::engine_update, this);
    Delay::update_loop(real_fps, update_func);
  }
  
  float get_real_fps() const { return real_fps; }
  int get_sim_delay_us() const { return sim_delay; }
  
  void set_state_game_over() { show_game_over = true; }
  void set_state_you_won() { show_you_won = true; }
  
  void request_exit() { exit_requested = true; }
  
private:
  void pre_quit()
  {
    end_screen();
    if (m_params.enable_terminal_window_resize)
      if (term_win_rows > 0 && term_win_cols > 0)
        resize_terminal_window(term_win_rows, term_win_cols);
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
  
    return_cursor();
    sh.clear();
    
    kpd = keyboard->readKey();
    auto key = keyboard::get_char_key(kpd);
    auto lo_key = str::to_lower(key);
    auto quit = lo_key == 'q';
    auto pause = lo_key == 'p';
    
    if (quit)
    {
      math::toggle(show_quit_confirm);
      quit_confirm_button = YesNoButtons::No;
    }
    else if (m_params.enable_pause && pause)
      math::toggle(paused);
      
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
      
      auto special_key = keyboard::get_special_key(kpd);
      
      draw_confirm(sh, titles, quit_confirm_button,
                   m_params.quit_confirm_title_style,
                   m_params.quit_confirm_button_style,
                   m_params.quit_confirm_info_style);
      if (special_key == keyboard::SpecialKey::Left)
        quit_confirm_button = YesNoButtons::Yes;
      else if (special_key == keyboard::SpecialKey::Right)
        quit_confirm_button = YesNoButtons::No;
      
      if (special_key == keyboard::SpecialKey::Enter)
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
          show_instructions = true;
        }
      }
      else if (m_params.enable_instructions_screen & show_instructions)
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
          draw_game_over(sh);
        else
        {
          game_over_timer--;
          if (game_over_timer == 0)
            on_enter_game_over();
        }
        
        update();
          
        if (m_params.enable_hiscores && key == ' ')
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
          draw_you_won(sh);
        else
        {
          you_won_timer--;
          if (you_won_timer == 0)
            on_enter_you_won();
        }
        
        update();
        
        if (m_params.enable_hiscores && key == ' ')
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
        if (draw_input_hiscore(sh, kpd, curr_score_item, hiscore_caret_idx, anim_ctr,
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
        draw_paused(sh, anim_ctr);
      }
      else
        update();
    }
      
    sh.print_screen_buffer(t, bg_color);
    //sh.print_screen_buffer_chars();
    //sh.print_screen_buffer_fg_colors();
    //sh.print_screen_buffer_bg_colors();
    
  ///
    
    anim_ctr++;
    
    sim_time_s += sim_dt_s;
    
    return true;
  }
};
