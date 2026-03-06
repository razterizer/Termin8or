//
//  TermHelper.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-01-19.
//

#pragma once
#include <Core/Utf8.h>
#include <Core/System.h>
#include <Core/Term.h>
#include <Core/MathUtils.h>
#include <Core/StlUtils.h>
#include <array>


namespace t8
{
  
  namespace term
  {
    inline constexpr char32_t none32 = 0xFFFFFFFFu; // char32_t is unsigned.
    inline constexpr char none = -0x80;
    
    inline bool force_ascii_fallback = false;
    
    inline ::term::TermMode m_term_mode;
  
    inline void init_locale()
    {
      static bool once [[maybe_unused]] = []()
      {
        std::setlocale(LC_CTYPE, "");
        return true;
      }();
    }
    
    namespace impl
    {
      // Where wcwidth == 1 is incorrect.
      inline constexpr std::array<stlutils::Range<char32_t>, 10> mac_not_singlecol =
      {{
        { 0x13F, 0x140 }, // Latin Extended-A
        { 0xA86, 0xA86 }, // Gujarati
        { 0xA8D, 0xA94 }, // Gujarati
        { 0xC58, 0xC59 }, // Telugu
        { 0xE33, 0xE33 }, // Thai
        { 0x2630, 0x2637 }, // Miscellaneous Symbols
        { 0x268A, 0x268F }, // Miscellaneous Symbols
        { 0x111C2, 0x111C3 }, // Sharada
        { 0x114A5, 0x114A5 }, // Tirhuta
        { 0x11D46, 0x11D46 }, // Masaram Gondi
      }};
      // Missing single width/cell/column glyphs.
      inline constexpr std::array<stlutils::Range<char32_t>, 143> mac_no_singlecol_glyph =
      {{
        { 0x560, 0x560 }, // Armenian
        { 0x588, 0x588 }, // Armenian
        { 0x5EF, 0x5EF }, // Hebrew
        { 0x860, 0x88E }, // Syriac Supplement - Arabic Extended-B
        { 0x8B5, 0x8B5 }, // Arabic Extended-A
        { 0x8BF, 0x8C7 }, // Arabic Extended-A
        { 0x8C9, 0x8C9 }, // Arabic Extended-A
        { 0x9FC, 0x9FC }, // Bengali
        { 0xA76, 0xA76 }, // Gurmukhi
        { 0xDE6, 0xDEF }, // Sinhala
        { 0xE86, 0xE86 }, // Lao
        { 0xE89, 0xE89 }, // Lao
        { 0xE8C, 0xE8C }, // Lao
        { 0xE8E, 0xE93 }, // Lao
        { 0xE98, 0xE98 }, // Lao
        { 0xEA0, 0xEA0 }, // Lao
        { 0xEA8, 0xEA9 }, // Lao
        { 0xEAC, 0xEAC }, // Lao
        { 0xEDE, 0xEDF }, // Lao
        { 0x10C7, 0x10C7 }, // Georgian
        { 0x10CD, 0x10CD }, // Georgian
        { 0x16F1, 0x16F8 }, // Runic
        { 0x1B4C, 0x1B4C }, // Balinese
        { 0x1B7D, 0x1B7E }, // Balinese
        { 0x1C80, 0x1C88 }, // Cyrillic Extended-C
        { 0x1CE9, 0x1CF1 }, // Vedic Extensions
        { 0x1CF6, 0x1CFA }, // Vedic Extensions (merged)
        { 0x20C0, 0x20C0 }, // Currency Symbols
        { 0x2187, 0x2188 }, // Number Forms
        { 0x218A, 0x218B }, // Number Forms
        { 0x2700, 0x2700 }, // Dingbats
        { 0x275F, 0x2760 }, // Dingbats
        { 0x27CE, 0x27CF }, // Miscellaneous Mathematical Symbols-A
        { 0x2B4D, 0x2B4F }, // Miscellaneous Symbols and Arrows
        { 0x2B56, 0x2B57 }, // Miscellaneous Symbols and Arrows
        { 0x2B59, 0x2B5F }, // Miscellaneous Symbols and Arrows
        { 0x2B66, 0x2B81 }, // Miscellaneous Symbols and Arrows
        { 0x2B84, 0x2B8F }, // Miscellaneous Symbols and Arrows
        { 0x2B92, 0x2B94 }, // Miscellaneous Symbols and Arrows
        { 0x2B97, 0x2BFF }, // Miscellaneous Symbols and Arrows
        { 0x2C2F, 0x2C2F }, // Glagolitic
        { 0x2C5F, 0x2C5F }, // Glagolitic
        { 0x2D00, 0x2D2D }, // Georgian Supplememnt
        { 0x2E19, 0x2E19 }, // Supplemental Punctuation
        { 0x2E32, 0x2E39 }, // Supplemental Punctuation
        { 0x2E3C, 0x2E3F }, // Supplemental Punctuation
        { 0x2E42, 0x2E5D }, // Supplemental Punctuation
        { 0xA78F, 0xA78F }, // Latin Extended-D
        { 0xA794, 0xA79F }, // Latin Extended-D
        { 0xA7AB, 0xA7AD }, // Latin Extended-D
        { 0xA7B0, 0xA7B4 }, // Latin Extended-D
        { 0xA7B6, 0xA7F7 }, // Latin Extended-D
        { 0xA8F2, 0xA8FD }, // Devanagari Extended
        { 0xAB30, 0xAB52 }, // Latin Extended-E
        { 0xAB54, 0xAB6B }, // Latin Extended-E
        { 0xD7B0, 0xF6D4 }, // Hangul Jamo Extended-B + Private Use Area
        { 0xF6D9, 0xF801 }, // Private Use Area
        { 0xF80D, 0xF81D }, // Private Use Area
        { 0xF828, 0xF83C }, // Private Use Area
        { 0xF83E, 0xF83F }, // Private Use Area
        { 0xF851, 0xF85E }, // Private Use Area
        { 0xF86C, 0xF881 }, // Private Use Area
        { 0xF884, 0xF8B6 }, // Private Use Area
        { 0xF8C1, 0xF8E4 }, // Private Use Area
        { 0xFBC2, 0xFBC2 }, // Arabic Presentation Forms-A
        { 0xFD40, 0xFD4F }, // Arabic Presentation Forms-A
        { 0xFFA0, 0xFFA0 }, // Halfwidth and Fullwidth Forms
        { 0x10144, 0x10145 }, // Ancient Greek Numbers
        { 0x10176, 0x10178 }, // Ancient Greek Numbers
        { 0x10183, 0x10183 }, // Ancient Greek Numbers
        { 0x10187, 0x10189 }, // Ancient Greek Numbers
        { 0x1018B, 0x1018E }, // Ancient Greek Numbers
        { 0x1019C, 0x101A0 }, // Ancient Symbols
        { 0x101D0, 0x101FC }, // Phaistos Disc
        { 0x10570, 0x105BC }, // Vithkuqi
        { 0x10780, 0x107BA }, // Latin Extended-F
        { 0x10A34, 0x10A35 }, // Kharoshthi
        { 0x10A48, 0x10A48 }, // Kharoshthi
        { 0x10E60, 0x10E7E }, // Rumi Numeral Symbols
        { 0x10F00, 0x10FF6 }, // Old Sogdian + Sogdian + Old Uyghur + Chorasmian + Elymaic
        { 0x11147, 0x11147 }, // Chakma
        { 0x111E1, 0x111F4 }, // Sinhala Archaic Numbers
        { 0x1123F, 0x11240 }, // Khojki
        { 0x1145A, 0x1145A }, // Newa
        { 0x11460, 0x11461 }, // Newa
        { 0x116B9, 0x116B9 }, // Takri
        { 0x11740, 0x11746 }, // Ahom
        { 0x11800, 0x1183B }, // Dogra
        { 0x11900, 0x11AA2 }, // Dives Akuru + Nandinagari + Zanabazar Square + Soyombo
        { 0x11B00, 0x11B09 }, // Devanagari Extended-A
        { 0x11EE0, 0x11FB0 }, // Makasar + Kawi + Lisu Supplement
        { 0x12F90, 0x12FF2 }, // Cypro-Minoan
        { 0x1342F, 0x14646 }, // Egyptian Hieroglyphs + Egyptian Hieroglyph Format Controls + Anatolian Hieroglyphs
        { 0x16A70, 0x16AC9 }, // Tangsa
        { 0x16E40, 0x16E9A }, // Medefaidrin
        { 0x1CF50, 0x1D0F5 }, // Znamenny Musical Notation + Byzantine Musical Symbols
        { 0x1D109, 0x1D10F }, // Musical Symbols
        { 0x1D113, 0x1D11D }, // Musical Symbols
        { 0x1D11F, 0x1D120 }, // Musical Symbols
        { 0x1D123, 0x1D129 }, // Musical Symbols
        { 0x1D12C, 0x1D191 }, // Musical Symbols
        { 0x1D194, 0x1D1A5 }, // Musical Symbols
        { 0x1D1A9, 0x1D1C6 }, // Musical Symbols
        { 0x1D1CF, 0x1D1EA }, // Musical Symbols
        { 0x1D200, 0x1D2F3 }, // Ancient Greek Musical Notation + Kaktovik Numerals + Mayan Numerals
        { 0x1D372, 0x1D378 }, // Counting Rod Numerals
        { 0x1D800, 0x1E06D }, // Sutton SignWriting + Latin Extended-G + Cyrillic Extended-D
        { 0x1E290, 0x1E2AD }, // Toto
        { 0x1E4D0, 0x1E7FE }, // Nag Mundari + Ethiopic Extended-B
        { 0x1EC71, 0x1EEBB }, // Indic Siyaq Numbers + Ottoman Siyaq Numbers + Arabic Mathematical Alphabetic Symbols
        { 0x1F0BF, 0x1F0BF }, // Playing Cards
        { 0x1F322, 0x1F323 }, // Miscellaneous Symbols and Pictographs
        { 0x1F394, 0x1F395 }, // Miscellaneous Symbols and Pictographs
        { 0x1F398, 0x1F398 }, // Miscellaneous Symbols and Pictographs
        { 0x1F39C, 0x1F39D }, // Miscellaneous Symbols and Pictographs
        { 0x1F3F1, 0x1F3F2 }, // Miscellaneous Symbols and Pictographs
        { 0x1F3F6, 0x1F3F6 }, // Miscellaneous Symbols and Pictographs
        { 0x1F4FE, 0x1F548 }, // Miscellaneous Symbols and Pictographs
        { 0x1F54F, 0x1F56E }, // Miscellaneous Symbols and Pictographs
        { 0x1F571, 0x1F572 }, // Miscellaneous Symbols and Pictographs
        { 0x1F57B, 0x1F586 }, // Miscellaneous Symbols and Pictographs
        { 0x1F588, 0x1F589 }, // Miscellaneous Symbols and Pictographs
        { 0x1F58E, 0x1F58F }, // Miscellaneous Symbols and Pictographs
        { 0x1F591, 0x1F5A3 }, // Miscellaneous Symbols and Pictographs
        { 0x1F5A6, 0x1F5A7 }, // Miscellaneous Symbols and Pictographs
        { 0x1F5A9, 0x1F5B0 }, // Miscellaneous Symbols and Pictographs
        { 0x1F5B3, 0x1F5BB }, // Miscellaneous Symbols and Pictographs
        { 0x1F5BD, 0x1F5C1 }, // Miscellaneous Symbols and Pictographs
        { 0x1F5C5, 0x1F5D0 }, // Miscellaneous Symbols and Pictographs
        { 0x1F5D4, 0x1F5DB }, // Miscellaneous Symbols and Pictographs
        { 0x1F5DF, 0x1F5E0 }, // Miscellaneous Symbols and Pictographs
        { 0x1F5E2, 0x1F5E2 }, // Miscellaneous Symbols and Pictographs
        { 0x1F5E4, 0x1F5E7 }, // Miscellaneous Symbols and Pictographs
        { 0x1F5E9, 0x1F5EE }, // Miscellaneous Symbols and Pictographs
        { 0x1F5F0, 0x1F5F2 }, // Miscellaneous Symbols and Pictographs
        { 0x1F5F4, 0x1F5F9 }, // Miscellaneous Symbols and Pictographs
        { 0x1F650, 0x1F67F }, // Ornamental Dingbats
        { 0x1F6C6, 0x1F6CA }, // Transport and Map Symbols
        { 0x1F6D3, 0x1F6D4 }, // Transport and Map Symbols
        { 0x1F6E6, 0x1F6E8 }, // Transport and Map Symbols
        { 0x1F6EA, 0x1F6EA }, // Transport and Map Symbols
        { 0x1F6F1, 0x1F6F2 }, // Transport and Map Symbols
        { 0x1F774, 0x10FFFD }, // Alchemical Symbols + Geometric Shapes Extended + Supplemental Arrows-C + Supplemental Symbols and Pictographs + Chess Symbols + Symbols for Legacy Computing + Supplementary Private Use Area-A + Supplementary Private Use Area-B
      }};
      inline constexpr std::array<stlutils::Range<char32_t>, 2> emojis =
      {{
        { 0x1F321, 0x1F5FA }, // Miscellaneous Symbols and Pictographs
        { 0x1F6C6, 0x1F6F3 }, // Transport and Map Symbols
      }};
      // Correct and printable single width/cell/column glyphs.
      inline constexpr std::array<stlutils::Range<char32_t>, 115> wincmd_singlecol_glyph_consolas =
      {{
        { 0x20, 0x7E }, // Basic Latin
        { 0xA0, 0xFF }, // Latin-1 Supplement
        { 0x370, 0x377 }, // Greek and Coptic
        { 0x37A, 0x37F }, // Greek and Coptic
        { 0x384, 0x38A }, // Greek and Coptic
        { 0x38C, 0x38C }, // Greek and Coptic
        { 0x38E, 0x3FF }, // Greek and Coptic
        { 0x531, 0x556 }, // Armenian
        { 0x559, 0x55F }, // Armenian
        { 0x561, 0x587 }, // Armenian
        { 0x589, 0x58A }, // Armenian
        { 0x58D, 0x58F }, // Armenian
        { 0x1D00, 0x1DBF }, // Phonetic Extensions + Phonetic Extensions Supplement
        { 0x1DC0, 0x1DCA }, // Combining Diacritical Marks Supplement
        { 0x1DFE, 0x1EFF }, // Combining Diacritical Marks Supplement + Latin Extended Additional
        { 0x1F00, 0x1F15 }, // Greek Extended
        { 0x1F18, 0x1F1D }, // Greek Extended
        { 0x1F20, 0x1F45 }, // Greek Extended
        { 0x1F48, 0x1F4D }, // Greek Extended
        { 0x1F50, 0x1F57 }, // Greek Extended
        { 0x1F59, 0x1F59 }, // Greek Extended
        { 0x1F5B, 0x1F5B }, // Greek Extended
        { 0x1F5D, 0x1F5D }, // Greek Extended
        { 0x1F5F, 0x1FB4 }, // Greek Extended
        { 0x1FB6, 0x1FC4 }, // Greek Extended
        { 0x1FC6, 0x1FD3 }, // Greek Extended
        { 0x1FD6, 0x1FDB }, // Greek Extended
        { 0x1FDD, 0x1FEF }, // Greek Extended
        { 0x1FF2, 0x1FF4 }, // Greek Extended
        { 0x1FF6, 0x1FFE }, // Greek Extended
        { 0x2000, 0x2010 }, // General Punctuation
        { 0x2012, 0x2022 }, // General Punctuation
        { 0x2024, 0x2024 }, // General Punctuation
        { 0x2026, 0x2026 }, // General Punctuation
        { 0x2012, 0x2022 }, // General Punctuation
        { 0x202F, 0x2030 }, // General Punctuation
        { 0x2032, 0x2034 }, // General Punctuation
        { 0x2039, 0x203A }, // General Punctuation
        { 0x203C, 0x203E }, // General Punctuation
        { 0x2043, 0x2044 }, // General Punctuation
        { 0x205E, 0x205F }, // General Punctuation
        { 0x2070, 0x2071 }, // Superscripts and Subscripts
        { 0x2074, 0x208E }, // Superscripts and Subscripts
        { 0x2090, 0x2094 }, // Superscripts and Subscripts
        { 0x20A0, 0x20BF }, // Currency Symbols
        { 0x20DD, 0x20DD }, // Combining Diacritical Marks for Symbols
        { 0x20F0, 0x20F0 }, // Combining Diacritical Marks for Symbols
        { 0x2105, 0x2105 }, // Letterlike Symbols
        { 0x2113, 0x2113 }, // Letterlike Symbols
        { 0x2116, 0x2117 }, // Letterlike Symbols
        { 0x2122, 0x2122 }, // Letterlike Symbols
        { 0x2126, 0x2126 }, // Letterlike Symbols
        { 0x212E, 0x212E }, // Letterlike Symbols
        { 0x2132, 0x2132 }, // Letterlike Symbols
        { 0x214D, 0x214E }, // Letterlike Symbols
        { 0x2153, 0x215E }, // Number Forms
        { 0x2183, 0x2184 }, // Number Forms
        { 0x2190, 0x2195 }, // Arrows
        { 0x21A8, 0x21A8 }, // Arrows
        { 0x2202, 0x2202 }, // Mathematical Operators
        { 0x2206, 0x2206 }, // Mathematical Operators
        { 0x220F, 0x220F }, // Mathematical Operators
        { 0x2211, 0x2212 }, // Mathematical Operators
        { 0x2215, 0x2215 }, // Mathematical Operators
        { 0x2219, 0x221A }, // Mathematical Operators
        { 0x221E, 0x221F }, // Mathematical Operators
        { 0x2229, 0x2229 }, // Mathematical Operators
        { 0x222B, 0x222B }, // Mathematical Operators
        { 0x2248, 0x2248 }, // Mathematical Operators
        { 0x2260, 0x2261 }, // Mathematical Operators
        { 0x2264, 0x2265 }, // Mathematical Operators
        { 0x2302, 0x2302 }, // Miscellaneous Technical
        { 0x2310, 0x2310 }, // Miscellaneous Technical
        { 0x2320, 0x2321 }, // Miscellaneous Technical
        { 0x2302, 0x2302 }, // Miscellaneous Technical
        { 0x2460, 0x2473 }, // Enclosed Alphanumerics
        { 0x24EA, 0x24F4 }, // Enclosed Alphanumerics
        { 0x24FF, 0x24FF }, // Enclosed Alphanumerics
        { 0x2500, 0x257F }, // Box Drawing
        { 0x2580, 0x2580 }, // Block Elements
        { 0x2584, 0x2584 }, // Block Elements
        { 0x2588, 0x2588 }, // Block Elements
        { 0x258C, 0x258C }, // Block Elements
        { 0x2590, 0x2593 }, // Block Elements
        { 0x25A0, 0x25A1 }, // Geometric Shapes
        { 0x25AA, 0x25AC }, // Geometric Shapes
        { 0x25B2, 0x25B2 }, // Geometric Shapes
        { 0x25B4, 0x25B4 }, // Geometric Shapes
        { 0x25B8, 0x25B8 }, // Geometric Shapes
        { 0x25BA, 0x25BA }, // Geometric Shapes
        { 0x25BC, 0x25BC }, // Geometric Shapes
        { 0x25BE, 0x25BE }, // Geometric Shapes
        { 0x25C2, 0x25C2 }, // Geometric Shapes
        { 0x25C4, 0x25C4 }, // Geometric Shapes
        { 0x25CA, 0x25CC }, // Geometric Shapes
        { 0x25CF, 0x25CF }, // Geometric Shapes
        { 0x25D8, 0x25D9 }, // Geometric Shapes
        { 0x25E6, 0x25E6 }, // Geometric Shapes
        { 0x263A, 0x263C }, // Miscellaneous Symbols
        { 0x2640, 0x2640 }, // Miscellaneous Symbols
        { 0x2642, 0x2642 }, // Miscellaneous Symbols
        { 0x2660, 0x2660 }, // Miscellaneous Symbols
        { 0x2663, 0x2663 }, // Miscellaneous Symbols
        { 0x2665, 0x2666 }, // Miscellaneous Symbols
        { 0x266A, 0x266B }, // Miscellaneous Symbols
        { 0x266F, 0x266F }, // Miscellaneous Symbols
        { 0x2736, 0x2736 }, // Dingbats
        { 0x2776, 0x277F }, // Dingbats
        { 0x2C60, 0x2C7F }, // Latin Extended-C
        { 0xA717, 0xA721 }, // Modifier Tone Letters + Latin Extended-D
        { 0xA788, 0xA78C }, // Latin Extended-D
        { 0xFB00, 0xFB06 }, // Alphabetic Presentation Forms
        { 0xFB13, 0xFB17 }, // Alphabetic Presentation Forms
        { 0xFE20, 0xFE23 }, // Combining Half Marks
        { 0xFFFC, 0xFFFC }, // Specials (0xFFFD : questionmark-box)
      }};
      // Correct and printable single width/cell/column glyphs.
      inline constexpr std::array<stlutils::Range<char32_t>, 85> wincmd_singlecol_glyph_lucida_console =
      {{
        { 0x20, 0x7E }, // Basic Latin
        { 0xA0, 0xFF }, // Latin-1 Supplement
        { 0x100, 0x17F }, // Latin Extended-A
        { 0x192, 0x192 }, // Latin Extended-B
        { 0x1FA, 0x1FF }, // Latin Extended-B
        { 0x218, 0x21B }, // Latin Extended-B
        { 0x2C6, 0x2C7 }, // Spacing Modifier Letters
        { 0x2C9, 0x2C9 }, // Spacing Modifier Letters
        { 0x2D8, 0x2DD }, // Spacing Modifier Letters
        { 0x37E, 0x37E }, // Greek and Coptic
        { 0x384, 0x38A }, // Greek and Coptic
        { 0x38C, 0x38C }, // Greek and Coptic
        { 0x38E, 0x3A1 }, // Greek and Coptic
        { 0x3A3, 0x3CE }, // Greek and Coptic
        { 0x401, 0x40C }, // Cyrillic
        { 0x40E, 0x44F }, // Cyrillic
        { 0x451, 0x45C }, // Cyrillic
        { 0x45E, 0x45F }, // Cyrillic
        { 0x490, 0x491 }, // Cyrillic
        { 0x1E80, 0x1E85 }, // Latin Extended Additional
        { 0x1EF2, 0x1EF3 }, // Latin Extended Additional
        { 0x2013, 0x2015 }, // General Punctuation
        { 0x2017, 0x201A }, // General Punctuation
        { 0x201C, 0x201E }, // General Punctuation
        { 0x2020, 0x2022 }, // General Punctuation
        { 0x2026, 0x2026 }, // General Punctuation
        { 0x2030, 0x2030 }, // General Punctuation
        { 0x2039, 0x203A }, // General Punctuation
        { 0x203C, 0x203C }, // General Punctuation
        { 0x203E, 0x203E }, // General Punctuation
        { 0x2044, 0x2044 }, // General Punctuation
        { 0x207F, 0x207F }, // Superscripts and Subscripts
        { 0x20A3, 0x20A4 }, // Currency Symbols
        { 0x20A7, 0x20A7 }, // Currency Symbols
        { 0x20AC, 0x20AC }, // Currency Symbols
        { 0x2116, 0x2116 }, // Letterlike Symbols
        { 0x2122, 0x2122 }, // Letterlike Symbols
        { 0x2126, 0x2126 }, // Letterlike Symbols
        { 0x215B, 0x215E }, // Number Forms
        { 0x2190, 0x2195 }, // Arrows
        { 0x21A8, 0x21A8 }, // Arrows
        { 0x2202, 0x2202 }, // Mathematical Operators
        { 0x2206, 0x2206 }, // Mathematical Operators
        { 0x220F, 0x220F }, // Mathematical Operators
        { 0x2211, 0x2212 }, // Mathematical Operators
        { 0x2219, 0x221A }, // Mathematical Operators
        { 0x221E, 0x221F }, // Mathematical Operators
        { 0x2229, 0x2229 }, // Mathematical Operators
        { 0x222B, 0x222B }, // Mathematical Operators
        { 0x2248, 0x2248 }, // Mathematical Operators
        { 0x2260, 0x2261 }, // Mathematical Operators
        { 0x2264, 0x2265 }, // Mathematical Operators
        { 0x2302, 0x2302 }, // Miscellaneous Technical
        { 0x2310, 0x2310 }, // Miscellaneous Technical
        { 0x2320, 0x2321 }, // Miscellaneous Technical
        { 0x2500, 0x2500 }, // Box Drawing
        { 0x2502, 0x2502 }, // Box Drawing
        { 0x250C, 0x250C }, // Box Drawing
        { 0x2510, 0x2510 }, // Box Drawing
        { 0x2514, 0x2514 }, // Box Drawing
        { 0x2518, 0x2518 }, // Box Drawing
        { 0x251C, 0x251C }, // Box Drawing
        { 0x2524, 0x2524 }, // Box Drawing
        { 0x252C, 0x252C }, // Box Drawing
        { 0x2534, 0x2534 }, // Box Drawing
        { 0x253C, 0x253C }, // Box Drawing
        { 0x2550, 0x256C }, // Box Drawing
        { 0x2580, 0x2580 }, // Block Elements
        { 0x2584, 0x2584 }, // Block Elements
        { 0x2588, 0x2588 }, // Block Elements
        { 0x258C, 0x258C }, // Block Elements
        { 0x2590, 0x2593 }, // Block Elements
        { 0x25A0, 0x25A0 }, // Geometric Shapes
        { 0x25AC, 0x25AC }, // Geometric Shapes
        { 0x25B2, 0x25B2 }, // Geometric Shapes
        { 0x25BA, 0x25BA }, // Geometric Shapes
        { 0x25BC, 0x25BC }, // Geometric Shapes
        { 0x25C4, 0x25C4 }, // Geometric Shapes
        { 0x25CA, 0x25CB }, // Geometric Shapes
        { 0x25D8, 0x25D9 }, // Geometric Shapes
        { 0x25A0, 0x25A0 }, // Geometric Shapes
        { 0x263A, 0x263C }, // Geometric Shapes
        { 0x2640, 0x2640 }, // Geometric Shapes
        { 0x2642, 0x2642 }, // Geometric Shapes
        //{ 0xFFFD, 0xFFFD }, // Specials (0xFFFD : questionmark-box)
      }};
      // Correct and printable single width/cell/column glyphs.
      inline const std::vector<stlutils::Range<char32_t>> wincmd_singlecol_glyph_cascadia_mono =
      {{
        { 0x20, 0x7E }, // Basic Latin
        { 0xA0, 0xFF }, // Latin-1 Supplement
        { 0x186, 0x186 }, // Latin Extended-B
        { 0x18E, 0x190 }, // Latin Extended-B
        { 0x192, 0x192 }, // Latin Extended-B
        { 0x194, 0x194 }, // Latin Extended-B
        { 0x196, 0x196 }, // Latin Extended-B
        { 0x19A, 0x19B }, // Latin Extended-B
        { 0x19D, 0x19D }, // Latin Extended-B
        { 0x1A0, 0x1A1 }, // Latin Extended-B
        { 0x1A9, 0x1A9 }, // Latin Extended-B
        { 0x1AF, 0x1B2 }, // Latin Extended-B
        { 0x1B7, 0x1B7 }, // Latin Extended-B
        { 0x1CD, 0x1CE }, // Latin Extended-B
        { 0x1DD, 0x1DD }, // Latin Extended-B
        { 0x1E4, 0x1E7 }, // Latin Extended-B
        { 0x1EA, 0x1EB }, // Latin Extended-B
        { 0x1F0, 0x1F0 }, // Latin Extended-B
        { 0x1FA, 0x1FF }, // Latin Extended-B
        { 0x218, 0x21B }, // Latin Extended-B
        { 0x21E, 0x21F }, // Latin Extended-B
        { 0x232, 0x233 }, // Latin Extended-B
        { 0x237, 0x237 }, // Latin Extended-B
        { 0x23A, 0x23B }, // Latin Extended-B
        { 0x23D, 0x23E }, // Latin Extended-B
        { 0x245, 0x245 }, // Latin Extended-B
        { 0x251, 0x251 }, // IPA Extensions
        { 0x254, 0x254 }, // IPA Extensions
        { 0x259, 0x259 }, // IPA Extensions
        { 0x25B, 0x25B }, // IPA Extensions
        { 0x262, 0x263 }, // IPA Extensions
        { 0x269, 0x26C }, // IPA Extensions
        { 0x272, 0x272 }, // IPA Extensions
        { 0x283, 0x283 }, // IPA Extensions
        { 0x28A, 0x28C }, // IPA Extensions
        { 0x292, 0x292 }, // IPA Extensions
        { 0x294, 0x295 }, // IPA Extensions
        { 0x29F, 0x29F }, // IPA Extensions
        { 0x2A7, 0x2A7 }, // IPA Extensions
        { 0x2B7, 0x2B8 }, // Spacing Modifier Letters
        { 0x2BB, 0x2BC }, // Spacing Modifier Letters
        { 0x2C0, 0x2C0 }, // Spacing Modifier Letters
        { 0x2C6, 0x2C7 }, // Spacing Modifier Letters
        { 0x2C9, 0x2C9 }, // Spacing Modifier Letters
        { 0x2D0, 0x2D0 }, // Spacing Modifier Letters
        { 0x2D8, 0x2DD }, // Spacing Modifier Letters
        { 0x300, 0x304 }, // Combining Diacritical Marks
        { 0x306, 0x30C }, // Combining Diacritical Marks
        { 0x312, 0x313 }, // Combining Diacritical Marks
        { 0x315, 0x315 }, // Combining Diacritical Marks
        { 0x31B, 0x31B }, // Combining Diacritical Marks
        { 0x323, 0x323 }, // Combining Diacritical Marks
        { 0x325, 0x328 }, // Combining Diacritical Marks
        { 0x331, 0x332 }, // Combining Diacritical Marks
        { 0x340, 0x342 }, // Combining Diacritical Marks
        { 0x374, 0x375 }, // Greek and Coptic
        { 0x37E, 0x37E }, // Greek and Coptic
        { 0x384, 0x38A }, // Greek and Coptic
        { 0x38C, 0x38C }, // Greek and Coptic
        { 0x38E, 0x3A1 }, // Greek and Coptic
        { 0x3A3, 0x3CF }, // Greek and Coptic
        { 0x3D7, 0x3D7 }, // Greek and Coptic
        { 0x400, 0x45F }, // Cyrillic
        { 0x490, 0x493 }, // Cyrillic
        { 0x496, 0x497 }, // Cyrillic
        { 0x49A, 0x49B }, // Cyrillic
        { 0x4A2, 0x4A3 }, // Cyrillic
        { 0x4AE, 0x4B3 }, // Cyrillic
        { 0x4B6, 0x4B7 }, // Cyrillic
        { 0x4BA, 0x4BB }, // Cyrillic
        { 0x4C0, 0x4C0 }, // Cyrillic
        { 0x4CF, 0x4CF }, // Cyrillic
        { 0x4D8, 0x4D9 }, // Cyrillic
        { 0x4E2, 0x4E3 }, // Cyrillic
        { 0x4E8, 0x4E9 }, // Cyrillic
        { 0x4EE, 0x4EF }, // Cyrillic
        { 0x1DBB, 0x1DBB }, // Phonetic Extensions Supplement
        { 0x1DBF, 0x1DBF }, // Phonetic Extensions Supplement
        { 0x1E24, 0x1E25 }, // Latin Extended Additional
        { 0x1E30, 0x1E30 }, // Latin Extended Additional
        { 0x1E32, 0x1E37 }, // Latin Extended Additional
        { 0x1E3A, 0x1E3B }, // Latin Extended Additional
        { 0x1E48, 0x1E49 }, // Latin Extended Additional
        { 0x1E50, 0x1E53 }, // Latin Extended Additional
        { 0x1E5A, 0x1E5B }, // Latin Extended Additional
        { 0x1E62, 0x1E63 }, // Latin Extended Additional
        { 0x1E6E, 0x1E6E }, // Latin Extended Additional
        { 0x1E80, 0x1E85 }, // Latin Extended Additional
        { 0x1E9E, 0x1E9E }, // Latin Extended Additional
        { 0x1EA0, 0x1EF9 }, // Latin Extended Additional
        { 0x2008, 0x2008 }, // General Punctuation
        { 0x2010, 0x2011 }, // General Punctuation
        { 0x2013, 0x2015 }, // General Punctuation
        { 0x2017, 0x201E }, // General Punctuation
        { 0x2020, 0x2022 }, // General Punctuation
        { 0x2024, 0x2024 }, // General Punctuation
        { 0x2026, 0x2026 }, // General Punctuation
        { 0x2028, 0x2028 }, // General Punctuation
        { 0x2030, 0x2030 }, // General Punctuation
        { 0x2032, 0x2033 }, // General Punctuation
        { 0x2039, 0x203A }, // General Punctuation
        { 0x203C, 0x203C }, // General Punctuation
        { 0x203E, 0x203E }, // General Punctuation
        { 0x2044, 0x2044 }, // General Punctuation
        { 0x2070, 0x2070 }, // Superscripts and Subscripts
        { 0x2074, 0x2079 }, // Superscripts and Subscripts
        { 0x207F, 0x2089 }, // Superscripts and Subscripts
        { 0x20A0, 0x20A1 }, // Currency Symbols
        { 0x20A3, 0x20A4 }, // Currency Symbols
        { 0x20A6, 0x20AE }, // Currency Symbols
        { 0x20B1, 0x20B2 }, // Currency Symbols
        { 0x20B4, 0x20B5 }, // Currency Symbols
        { 0x20B8, 0x20BA }, // Currency Symbols
        { 0x20BC, 0x20BE }, // Currency Symbols
        { 0x2105, 0x2105 }, // Letterlike Symbols
        { 0x2113, 0x2113 }, // Letterlike Symbols
        { 0x2116, 0x2117 }, // Letterlike Symbols
        { 0x211E, 0x211E }, // Letterlike Symbols
        { 0x2122, 0x2122 }, // Letterlike Symbols
        { 0x2126, 0x2126 }, // Letterlike Symbols
        { 0x212E, 0x212E }, // Letterlike Symbols
        { 0x215B, 0x215E }, // Number Forms
        { 0x2190, 0x2195 }, // Arrows
        { 0x21A8, 0x21A8 }, // Arrows
        { 0x21B2, 0x21B2 }, // Arrows
        { 0x21E1, 0x21E1 }, // Arrows
        { 0x21E3, 0x21E3 }, // Arrows
        { 0x2202, 0x2202 }, // Mathematical Operators
        { 0x2206, 0x2206 }, // Mathematical Operators
        { 0x220F, 0x220F }, // Mathematical Operators
        { 0x2211, 0x2212 }, // Mathematical Operators
        { 0x2215, 0x2215 }, // Mathematical Operators
        { 0x2219, 0x221A }, // Mathematical Operators
        { 0x221E, 0x221F }, // Mathematical Operators
        { 0x2229, 0x2229 }, // Mathematical Operators
        { 0x222B, 0x222B }, // Mathematical Operators
        { 0x2248, 0x2248 }, // Mathematical Operators
        { 0x2260, 0x2265 }, // Mathematical Operators
        // ... to be continued ...
      }};
    }
    
    template<typename CharT>
    inline constexpr bool is_printable_ascii(CharT cp) noexcept
    {
      return math::in_r_c<CharT>(cp, 0x20, 0x7E);
    }
    
    inline bool use_ansi_renderer()
    {
      return ::term::use_ansi_renderer(m_term_mode);
    }
    
    inline void emit_text(std::string_view sv_utf8, std::string_view sv_bytes_for_legacy = {})
    {
      ::term::emit_text(m_term_mode, sv_utf8, sv_bytes_for_legacy);
    }
    
    // We assume single column and rely on encoder fallback.
    inline bool is_single_column(char32_t cp)
    {
      if (!term::use_ansi_renderer())
      {
        // Treat as single-column if we can encode it to CP437 (or ASCII).
        // If you don’t have a predicate, just allow and rely on encoding fallback.
        return cp <= 0x10FFFF;
      }
      
      if (cp > 0x10FFFF)
        return false;
      
#ifdef _WIN32
      // If not cmd, assume single-column and rely on fallback policy elsewhere.
      return cp <= 0x10FFFF;
#else
      init_locale();
      wchar_t wc = static_cast<wchar_t>(cp);
      int w = ::wcwidth(wc);
      return w == 1; //w <= 1;
#endif
    }
    
    inline bool can_render_single_column_cp(char32_t cp, bool allow_emojis)
    {
      // Reject invalid.
      if (cp > 0x10FFFF)
        return false;
      if (0xD800 <= cp && cp <= 0xDFFF)
        return false;
      
      if (m_term_mode.is_conhost_like)
      {
        // WinAPI buffer is BMP-only.
        if (cp > 0xFFFF)
          return false;
          
        // Conservative whitelist: box + block + ASCII.
        const bool ascii = 0x20 <= cp && cp <= 0x7E;
        const bool box   = 0x2500 <= cp && cp <= 0x257F;
        const bool block = 0x2580 <= cp && cp <= 0x259F;
        
        switch (m_term_mode.win_font_class)
        {
          case ::term::WinFontClass::Unknown:
            return ascii;
          case ::term::WinFontClass::RasterTerminal:
            assert(!m_term_mode.truetype_font);
            return ascii;
          case ::term::WinFontClass::LucidaConsole:
            return stlutils::in_ranges<char32_t>(cp, impl::wincmd_singlecol_glyph_lucida_console);
          case ::term::WinFontClass::Consolas:
            return stlutils::in_ranges<char32_t>(cp, impl::wincmd_singlecol_glyph_consolas);
          case ::term::WinFontClass::Cascadia:
            return stlutils::in_ranges<char32_t>(cp, impl::wincmd_singlecol_glyph_cascadia_mono);
          case ::term::WinFontClass::UnknownTrueType:
            //return ascii; // Perhaps too restrictive.
            return ascii || box || block; // More practical.
        }
        return true;
      }
      
      if (!allow_emojis && stlutils::in_ranges<char32_t>(cp, impl::emojis))
        return false;
      
#ifdef __APPLE__
      // mac_not_singlecol : Where wcwidth == 1 is incorrect.
      // mac_no_singlecol_glyph : Missing single width/cell/column glyphs.
      if (!is_single_column(cp))
        return false;
      if (stlutils::in_ranges<char32_t>(cp, impl::mac_not_singlecol))
        return false;
      if (stlutils::in_ranges<char32_t>(cp, impl::mac_no_singlecol_glyph))
        return false;
#endif
      return is_single_column(cp);
    }
    
    inline char32_t get_renderable_char32(char32_t cp)
    {
      return can_render_single_column_cp(cp, false) ? cp : none32;
    }
    
    inline char32_t resolve_single_width_glyph(char32_t preferred, char fallback = none)
    {
      if (can_render_single_column_cp(preferred, false))
        return preferred;
          
      // Fallback (treat fallback as ASCII only).
      if (fallback != none)
      {
        unsigned char fb = static_cast<unsigned char>(fallback);
        if (fb <= 0x7F && is_single_column(static_cast<char32_t>(fb)))
          return static_cast<char32_t>(fb);
      }
      
      return none32;
    }
    
    template<typename CharT>
    inline CharT resolve_single_width_glyph(char32_t preferred,
                                            char fallback = none)
    {
      auto f_handle_ascii = [preferred, fallback]() -> char
      {
        if (preferred <= 0x7F)
          return static_cast<char>(preferred);
        if (fallback != none && static_cast<unsigned char>(fallback) <= 0x7F)
          return fallback;
        return '?';
      };
    
      if constexpr (std::is_same_v<CharT, char>)
        return f_handle_ascii();
      if constexpr (std::is_same_v<CharT, char32_t>)
      {
        if (force_ascii_fallback)
          return static_cast<char32_t>(static_cast<unsigned char>(f_handle_ascii()));
        return term::resolve_single_width_glyph(preferred, fallback);
      }
      
      static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                    "ERROR in ScreenHandler::resolve_single_width_glyph() : Unsupported CharT!");
      return static_cast<CharT>('?');
    }
    
    inline std::string encode_single_width_glyph(char32_t preferred,
                                                 char fallback = none)
    {
      char32_t cp = resolve_single_width_glyph(preferred, fallback);
      
      return utf8::encode_char32_utf8(cp);
    }
    
    template<typename CharT>
    inline std::string encode_single_width_glyph(char32_t preferred,
                                                 char fallback = none)
    {
      auto f_handle_ascii = [preferred, fallback]() -> std::string
      {
        if (preferred <= 0x7F)
          return std::string(1, static_cast<char>(preferred));
        if (fallback != none && static_cast<unsigned char>(fallback) <= 0x7F)
          return std::string(1, fallback);
        return "?";
      };
    
      if constexpr (std::is_same_v<CharT, char>)
        return f_handle_ascii();
      if constexpr (std::is_same_v<CharT, char32_t>)
      {
        if (force_ascii_fallback)
          return f_handle_ascii();
        return term::encode_single_width_glyph(preferred, fallback);
      }
      
      static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                    "ERROR in ScreenHandler::encode_single_width_glyph() : Unsupported CharT!");
      return "?";
    }
    
  }
  
}
