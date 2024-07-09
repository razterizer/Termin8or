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

class MessageHandler
{
public:
  enum class Level { Guide, Warning, Fatal };
  
private:
  const float duration = 1.5f; //4.f;
  float trg_time = 0.f;
  bool message_empty = true;
  std::vector<std::pair<std::string, Level>> messages;
  std::string curr_message;
  ui::TextBox tb;
  int str_len = 0;
  Level curr_level;
  
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
  
public:
  void add_message(float time, const std::string& msg, Level lvl)
  {
    messages.emplace_back(msg, lvl);
    std::sort(messages.begin(), messages.end(),
      [](const auto& msgA, const auto& msgB) { return msgA.second < msgB.second;} );
  }
  
  template<int NR, int NC>
  void update(SpriteHandler<NR, NC>& sh, float time, ui::VerticalAlignment v_align = ui::VerticalAlignment::CENTER, ui::HorizontalAlignment h_align = ui::HorizontalAlignment::CENTER, bool draw_box_outline = true, bool draw_box_bkg = true, int box_padding_ud = 0, int box_padding_lr = 1, drawing::OutlineType outline_type = drawing::OutlineType::Line)
  {
    if (message_empty && !messages.empty())
    {
      trg_time = time;
      std::tie(curr_message, curr_level) = messages.back();
      messages.pop_back();
      str_len = static_cast<int>(curr_message.size());
      message_empty = false;
    }
    
    if (time - trg_time <= duration && !curr_message.empty())
    {
      auto fg_color = get_fg_color();
      auto bg_color = get_bg_color();
      tb.set_text(curr_message);
      tb.calc_pre_draw(str::Adjustment::Center);
      tb.draw(sh, v_align, h_align, { fg_color, bg_color }, draw_box_outline, draw_box_bkg, box_padding_ud, box_padding_lr, std::nullopt, outline_type);
    }
    else
      message_empty = true;
  }
  
  bool has_message(float time) const { return !message_empty; }
};


