//
//  MessageHandler.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-18.
//

#pragma once
#include "Text.h"
#include <execution>

class MessageHandler
{
public:
  enum class Level { Guide, Warning, Fatal };
  
private:
  const float duration = 1.5f; //4.f;
  float trg_time = 0.f;
  bool message_empty = true;
  int r = 15;
  int c = 0;
  std::vector<std::pair<std::string, Level>> messages;
  std::string curr_message;
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
  void update(SpriteHandler<NR, NC>& sh, float time, bool boxed = false)
  {
    if (message_empty && !messages.empty())
    {
      trg_time = time;
      std::tie(curr_message, curr_level) = messages.back();
      messages.pop_back();
      str_len = static_cast<int>(curr_message.size());
      message_empty = false;
      c = std::round((NC - str_len)/2.f);
    }
    
    if (time - trg_time <= duration && !curr_message.empty())
    {
      auto fg_color = get_fg_color();
      sh.write_buffer(curr_message, r, c, fg_color, get_bg_color());
      if (boxed)
        drawing::draw_box(sh, r-1, c-2, 2, str_len + 3, drawing::OutlineType::Line, { fg_color, get_bg_color() }, { fg_color, get_bg_color() });
    }
    else
      message_empty = true;
  }
  
  bool has_message(float time) const { return !message_empty; }
};


