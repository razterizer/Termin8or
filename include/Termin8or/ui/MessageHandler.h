//
//  MessageHandler.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-18.
//

#pragma once
#include "../screen/Color.h"
#include "../ui/UI.h"
#include <execution>
#include <queue>


namespace t8x
{
  
  struct MessageBoxDrawingArgs
  {
    bool draw_box_outline = true;
    bool draw_box_bkg = true;
    int box_padding_ud = 0;
    int box_padding_lr = 1;
    OutlineType outline_type = OutlineType::Line;
    VerticalAlignment v_align = VerticalAlignment::CENTER;
    HorizontalAlignment h_align = HorizontalAlignment::CENTER;
    int v_align_offs = 0;
    int h_align_offs = 0;
    bool framed_mode = true;
  };
  
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
    TextBox tb;
    int str_len = 0;
    
    Color get_fg_color() const
    {
      switch (curr_level)
      {
        case Level::Guide:   return Color16::White;
        case Level::Warning: return Color16::White;
        case Level::Fatal:   return Color16::White;
        default:             return Color16::Default;
      }
    }
    
    Color get_bg_color() const
    {
      switch (curr_level)
      {
        case Level::Guide:   return Color16::DarkBlue;
        case Level::Warning: return Color16::DarkYellow;
        case Level::Fatal:   return Color16::DarkRed;
        default:             return Color16::Default;
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
    void update(t8::ScreenHandler<NR, NC>& sh, float time, const MessageBoxDrawingArgs& args = {})
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
        TextBoxDrawingArgsAlign aargs;
        aargs.v_align = args.v_align;
        aargs.h_align = args.h_align;
        aargs.v_align_offs = args.v_align_offs;
        aargs.h_align_offs = args.h_align_offs;
        aargs.base.box_style = { fg_color, bg_color };
        aargs.base.draw_box_outline = args.draw_box_outline;
        aargs.base.draw_box_bkg = args.draw_box_bkg;
        aargs.base.box_padding_ud = args.box_padding_ud;
        aargs.base.box_padding_lr = args.box_padding_lr;
        aargs.base.box_outline_style = std::nullopt;
        aargs.base.outline_type = args.outline_type;
        aargs.framed_mode = args.framed_mode;
        tb.draw(sh, aargs);
      }
      else
        message_empty = true;
    }
    
    bool has_message(float time) const { return !message_empty; }
    
    void clear_curr_message() { curr_message.clear(); }
  };
  
}
