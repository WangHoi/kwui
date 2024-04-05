#include "TextFlowX.h"
#include "base/EncodingManager.h"
#include "src/utils/SkUTF.h"
#include <vector>

namespace xskia {

static inline bool is_breaking_whitespace(SkUnichar c) {
    switch (c) {
    case 0x0020: // SPACE
        //case 0x00A0: // NO-BREAK SPACE
    case 0x1680: // OGHAM SPACE MARK
    case 0x180E: // MONGOLIAN VOWEL SEPARATOR
    case 0x2000: // EN QUAD
    case 0x2001: // EM QUAD
    case 0x2002: // EN SPACE (nut)
    case 0x2003: // EM SPACE (mutton)
    case 0x2004: // THREE-PER-EM SPACE (thick space)
    case 0x2005: // FOUR-PER-EM SPACE (mid space)
    case 0x2006: // SIX-PER-EM SPACE
    case 0x2007: // FIGURE SPACE
    case 0x2008: // PUNCTUATION SPACE
    case 0x2009: // THIN SPACE
    case 0x200A: // HAIR SPACE
    case 0x200B: // ZERO WIDTH SPACE
    case 0x202F: // NARROW NO-BREAK SPACE
    case 0x205F: // MEDIUM MATHEMATICAL SPACE
    case 0x3000: // IDEOGRAPHIC SPACE
        //case 0xFEFF: // ZERO WIDTH NO-BREAK SPACE
        return true;
    default:
        return false;
    }
}

static size_t linebreak(const char text[], const char stop[],
    const SkFont& font, SkScalar width,
    SkScalar* advance,
    size_t* trailing)
{
    SkScalar accumulatedWidth = 0;
    int glyphIndex = 0;
    const char* start = text;
    const char* word_start = text;
    bool prevWS = true;
    *trailing = 0;

    while (text < stop) {
        const char* prevText = text;
        SkUnichar uni = SkUTF::NextUTF8(&text, stop);
        accumulatedWidth += advance[glyphIndex++];
        bool currWS = is_breaking_whitespace(uni);

        if (!currWS && prevWS) {
            word_start = prevText;
        }
        prevWS = currWS;

        if (width < accumulatedWidth) {
            if (currWS) {
                // eat the rest of the whitespace
                const char* next = text;
                while (next < stop && is_breaking_whitespace(SkUTF::NextUTF8(&next, stop))) {
                    text = next;
                }
                if (trailing) {
                    *trailing = text - prevText;
                }
            } else {
                // backup until a whitespace (or 1 char)
                if (word_start == start) {
                    if (prevText > start) {
                        text = prevText;
                    }
                } else {
                    text = word_start;
                }
            }
            break;
        }
    }

    return text - start;
}

GlyphRunX::GlyphRunX(SkFont font, const SkFontMetrics& fm, absl::Span<SkGlyphID> glyphs, float left, absl::Span<SkScalar> advances)
    : font_(font)
{
    glyphs_.assign(glyphs.begin(), glyphs.end());
    positions_.resize(advances.size());
    float x = left;
    for (size_t i = 0; i < advances.length(); ++i) {
        positions_[i].fX = x;
        x += advances[i];
    }
    bounds_.bottom = -fm.fAscent + fm.fDescent + fm.fLeading;
    bounds_.left = left;
    bounds_.right = x;
}
scene2d::RectF GlyphRunX::boundingRect()
{
    return bounds_;
}

TextFlowX::TextFlowX(const std::string& text,
	const char* font_family,
	style::FontStyle font_style,
	style::FontWeight font_weight,
	float font_size)
	: font_(SkTypeface::MakeFromName(font_family, SkFontStyle()), font_size)
	, text_(text)
{
	font_.getMetrics(&font_metrics_);
}

TextFlowX::~TextFlowX() {}

style::FontMetrics TextFlowX::fontMetrics()
{
	style::FontMetrics fm;
	fm.ascent = -font_metrics_.fAscent;
	fm.descent = font_metrics_.fDescent;
	fm.line_gap = font_metrics_.fLeading;
	fm.cap_height = font_metrics_.fCapHeight;
	fm.x_height = font_metrics_.fXHeight;
	fm.underline_offset = font_metrics_.fUnderlinePosition;
	fm.underline_thickness = font_metrics_.fUnderlineThickness;
	fm.line_through_offset = font_metrics_.fStrikeoutPosition;
	fm.line_through_thickness = font_metrics_.fStrikeoutThickness;
	return fm;
}
std::tuple<float, float> TextFlowX::measureWidth()
{
	return std::make_tuple(0.0f, 1000.0f);
}
void TextFlowX::flowText(graph2d::TextFlowSourceInterface* source, graph2d::TextFlowSinkInterface* sink)
{
	bool first = true;
	float line_height = -font_metrics_.fAscent + font_metrics_.fDescent + font_metrics_.fLeading;
	//source->getCurrentLine(line_height, left, width, empty);
	size_t glyph_count = font_.countText(text_.c_str(), text_.length(), SkTextEncoding::kUTF8);
	std::vector<SkGlyphID> glyphs;
	glyphs.resize(glyph_count);
	std::vector<SkScalar> advances;
	advances.resize(glyph_count);
	font_.textToGlyphs(text_.c_str(), text_.length(), SkTextEncoding::kUTF8, glyphs.data(), glyph_count);
	font_.getWidthsBounds(glyphs.data(), glyph_count, advances.data(), nullptr, nullptr);

    sink->prepare(glyph_count);

    const char* utf8 = text_.c_str();
    size_t utf8Bytes = text_.length();
    size_t glyphOffset = 0;
    size_t utf8Offset = 0;
    float left;
    float width;
    bool empty;
    style::LineBox* line;
    while (0 < utf8Bytes) {
        if (first) {
            first = false;
            line = source->getCurrentLine(line_height, left, width, empty);
        } else {
            line = source->getNextLine(line_height, left, width);
        }
        size_t bytesCollapsed;
        size_t bytesConsumed = linebreak(utf8, utf8 + utf8Bytes, font_, width,
            advances.data() + glyphOffset, &bytesCollapsed);
        size_t bytesVisible = bytesConsumed - bytesCollapsed;

        size_t numGlyphs = SkUTF::CountUTF8(utf8, bytesVisible);
        float line_advance = font_.measureText(utf8, bytesVisible, SkTextEncoding::kUTF8);
        auto run = std::make_unique<GlyphRunX>(font_, font_metrics_, absl::MakeSpan(glyphs.data() + glyphOffset, numGlyphs),
            left, absl::MakeSpan(advances.data() + glyphOffset, numGlyphs));
        sink->addGlyphRun(line, scene2d::PointF(left, 0.0f), std::move(run));

        glyphOffset += SkUTF::CountUTF8(utf8, bytesConsumed);
        utf8Offset += bytesConsumed;
        utf8 += bytesConsumed;
        utf8Bytes -= bytesConsumed;

    }

}

}