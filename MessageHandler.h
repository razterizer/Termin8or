//
//  MessageHandler.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-18.
//

#pragma once
#include "Color.h"
#include "UI.h"
#include <execution>
#include <queue>

class MessageHandler
{
public:
  enum class Level { Guide, Warning, Fatal };
  
private:
  float trg_time = 0.f;
  bool message_empty = true;
  std::queue<std::pair<std::string, float>> messages_guide;
  std::queue<std::pair<std::string, float>> messages_warning;
  std::queue<std::pair<std::string, float>> messages_fatal;
  std::string curr_message;
  Level curr_level;
  float curr_duration_s = 1.5f;
  ui::TextBox tb;
  int str_len = 0;
  
  Color get_fg_color() const
  {
    switch (curr_level)
    {
      case Level::Guide:   return Color::White;
      case Level::Warning: return Color::White;
      case Level::Fatal:   return Color::White;
      default:             return Color::Default;
    }
  }
  
  Color get_bg_color() const
  {
    switch (curr_level)
    {
      case Level::Guide:   return Color::DarkBlue;
      case Level::Warning: return Color::DarkYellow;
      case Level::Fatal:   return Color::DarkRed;
      default:             return Color::Default;
    }
  }
  
  bool has_messages() const
  {
    return !messages_guide.empty() || !messages_warning.empty() || !messages_fatal.empty();
  }
  
  std::tuple<std::string, Level, float> fetch_next_message()
  {
    if (!messages_fatal.empty())
    {
      const auto& msg = messages_fatal.front();
      const auto ret = std::make_tuple(msg.first, Level::Fatal, msg.second);
      messages_fatal.pop();
      return ret;
    }
    if (!messages_warning.empty())
    {
      const auto& msg = messages_warning.front();
      const auto ret = std::make_tuple(msg.first, Level::Warning, msg.second);
      messages_warning.pop();
      return ret;
    }
    if (!messages_guide.empty())
    {
      const auto& msg = messages_guide.front();
      const auto ret = std::make_tuple(msg.first, Level::Guide, msg.second);
      messages_guide.pop();
      return ret;
    }
    return { "<Invalid Message>", Level::Guide, 0.5f };
  }
  
public:
  void add_message(float time, const std::string& msg, Level lvl, float duration_s = 1.5f)
  {
    switch (lvl)
    {
      case Level::Guide:
        messages_guide.push({ msg, duration_s });
        break;
      case Level::Warning:
        messages_warning.push({ msg, duration_s });
        break;
      case Level::Fatal:
        messages_fatal.push({ msg, duration_s });
        break;
    }
  }
  
  template<int NR, int NC>
  void update(SpriteHandler<NR, NC>& sh, float time, ui::VerticalAlignment v_align = ui::VerticalAlignment::CENTER, ui::HorizontalAlignment h_align = ui::HorizontalAlignment::CENTER, bool draw_box_outline = true, bool draw_box_bkg = true, int box_padding_ud = 0, int box_padding_lr = 1, drawing::OutlineType outline_type = drawing::OutlineType::Line, bool framed_mode = true)
  {
    if (message_empty && has_messages())
    {
      trg_time = time;
      std::tie(curr_message, curr_level, curr_duration_s) = fetch_next_message();
      str_len = static_cast<int>(curr_message.size());
      message_empty = false;
    }
    
    if (time - trg_time <= curr_duration_s && !curr_message.empty())
    {
      auto fg_color = get_fg_color();
      auto bg_color = get_bg_color();
      tb.set_text(curr_message);
      tb.calc_pre_draw(str::Adjustment::Center);
      tb.draw(sh, v_align, h_align, { fg_color, bg_color }, draw_box_outline, draw_box_bkg, box_padding_ud, box_padding_lr, std::nullopt, outline_type, framed_mode);
    }
    else
      message_empty = true;
  }
  
  bool has_message(float time) const { return !message_empty; }
  
  void clear_curr_message() { curr_message.clear(); }
};


