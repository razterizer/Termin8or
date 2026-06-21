//
//  MessageHandler.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-18.
//

#pragma once
#include "../screen/Color.h"
#include "../screen/GlyphString.h" // Unfortunate cost due to making GlyphString more conversion-strict.
#include "../ui/widget/TextBox.h"
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
  
  enum class MessageHandlerLevel { Guide, Warning, Fatal };
  
  template<typename StrT = std::string>
  class MessageHandler
  {
    using StrVecT = std::vector<StrT>;
  
    float trg_time = 0.f;
    bool message_empty = true;
    std::queue<std::pair<StrVecT, float>> messages_guide;
    std::queue<std::pair<StrVecT, float>> messages_warning;
    std::queue<std::pair<StrVecT, float>> messages_fatal;
    StrVecT curr_message;
    MessageHandlerLevel curr_level;
    float curr_duration_s = 1.5f;
    TextBox<StrT> tb;
    int str_len = 0;
    
    Color get_fg_color() const
    {
      switch (curr_level)
      {
        case MessageHandlerLevel::Guide:   return Color16::White;
        case MessageHandlerLevel::Warning: return Color16::White;
        case MessageHandlerLevel::Fatal:   return Color16::White;
        default: return Color16::Default;
      }
    }
    
    Color get_bg_color() const
    {
      switch (curr_level)
      {
        case MessageHandlerLevel::Guide:   return Color16::DarkBlue;
        case MessageHandlerLevel::Warning: return Color16::DarkYellow;
        case MessageHandlerLevel::Fatal:   return Color16::DarkRed;
        default: return Color16::Default;
      }
    }
    
    bool has_messages() const
    {
      return !messages_guide.empty() || !messages_warning.empty() || !messages_fatal.empty();
    }
    
    StrT make_text(std::string_view sv)
    {
      if constexpr (std::is_same_v<StrT, t8::GlyphString>)
        return t8::GlyphString::from_ascii(sv);
      else
        return StrT { std::string { sv } };
    }
    
    std::tuple<StrVecT, MessageHandlerLevel, float> fetch_next_message()
    {
      if (!messages_fatal.empty())
      {
        const auto& msg = messages_fatal.front();
        const auto ret = std::make_tuple(msg.first, MessageHandlerLevel::Fatal, msg.second);
        messages_fatal.pop();
        return ret;
      }
      if (!messages_warning.empty())
      {
        const auto& msg = messages_warning.front();
        const auto ret = std::make_tuple(msg.first, MessageHandlerLevel::Warning, msg.second);
        messages_warning.pop();
        return ret;
      }
      if (!messages_guide.empty())
      {
        const auto& msg = messages_guide.front();
        const auto ret = std::make_tuple(msg.first, MessageHandlerLevel::Guide, msg.second);
        messages_guide.pop();
        return ret;
      }
      return { { make_text("<Invalid Message>") } , MessageHandlerLevel::Guide, 0.5f };
    }
    
  public:
    void add_message(float time, const StrT& single_line_msg, MessageHandlerLevel lvl, float duration_s = 1.5f)
    {
      add_message_multi_line(time, { single_line_msg }, lvl, duration_s);
    }
  
    void add_message_multi_line(float time, const StrVecT& multi_line_msg, MessageHandlerLevel lvl, float duration_s = 1.5f)
    {
      switch (lvl)
      {
        case MessageHandlerLevel::Guide:
          messages_guide.push({ multi_line_msg, duration_s });
          break;
        case MessageHandlerLevel::Warning:
          messages_warning.push({ multi_line_msg, duration_s });
          break;
        case MessageHandlerLevel::Fatal:
          messages_fatal.push({ multi_line_msg, duration_s });
          break;
      }
    }
    
    template<int NR, int NC, typename CharT>
    void update(t8::ScreenHandler<NR, NC, CharT>& sh, float time, const MessageBoxDrawingArgs& args = {})
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
        tb.set_text(curr_message, {}, {}, str::Adjustment::Center);
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
