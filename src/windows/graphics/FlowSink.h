#pragma once
#include "windows/windows_header.h"
#include <vector>

class FlowLayoutSinkInterface
{
public:
    virtual HRESULT Prepare(UINT32 glyphCount) = 0;
    virtual HRESULT SetGlyphRun(
        float x,
        float y,
        UINT32 glyphCount,
        const UINT16* glyphIndices, // [glyphCount]
        const float* glyphAdvances, // [glyphCount]
        const DWRITE_GLYPH_OFFSET* glyphOffsets, // [glyphCount]
        ComPtr<IDWriteFontFace> fontFace,
        float fontEmSize,
        UINT8 bidiLevel,
        bool isSideways
    ) = 0;
};

class FlowLayoutSink : public FlowLayoutSinkInterface
{
public:
    FlowLayoutSink()
    { }

    HRESULT Reset();

    HRESULT Prepare(UINT32 glyphCount);

    HRESULT SetGlyphRun(
        float x,
        float y,
        UINT32 glyphCount,
        const UINT16* glyphIndices, // [glyphCount]
        const float* glyphAdvances, // [glyphCount]
        const DWRITE_GLYPH_OFFSET* glyphOffsets, // [glyphCount]
        ComPtr<IDWriteFontFace> fontFace,
        float fontEmSize,
        UINT8 bidiLevel,
        bool isSideways
        );

    HRESULT DrawGlyphRuns(
        IDWriteBitmapRenderTarget* renderTarget,
        IDWriteRenderingParams* renderingParams,
        COLORREF textColor
        ) const;

protected:
    // This glyph run is based off DWRITE_GLYPH_RUN
    // and is trivially convertable to it, but stores
    // pointers as relative indices instead instead
    // of raw pointers, which makes it more useful for
    // storing in a vector. Additionally, it stores
    // the x,y coordinate.

    struct CustomGlyphRun
    {
        CustomGlyphRun()
        :   fontFace(),
            fontEmSize(),
            glyphStart(),
            glyphCount(),
            bidiLevel(),
            isSideways(),
            x(),
            y()
        { }

        CustomGlyphRun(const CustomGlyphRun& b)
        {
            memcpy(this, &b, sizeof(*this));
            fontFace = b.fontFace;
        }

        CustomGlyphRun& operator=(const CustomGlyphRun& b)
        {
            if (this != &b)
            {
                // Define assignment operator in terms of destructor and
                // placement new constructor, paying heed to self assignment.
                this->~CustomGlyphRun();
                new(this) CustomGlyphRun(b);
            }
            return *this;
        }

        ~CustomGlyphRun()
        {
        }

        ComPtr<IDWriteFontFace> fontFace;
        float fontEmSize;
        float x;
        float y;
        UINT32 glyphStart;
        UINT32 glyphCount;
        UINT8 bidiLevel;
        bool isSideways;

        void Convert(
            const UINT16* glyphIndices,                 // [glyphCount]
            const float* glyphAdvances,                 // [glyphCount]
            const DWRITE_GLYPH_OFFSET* glyphOffsets,    // [glyphCount]
            OUT DWRITE_GLYPH_RUN* glyphRun
            ) const throw();
    };

    std::vector<CustomGlyphRun>         glyphRuns_;
    std::vector<UINT16>                 glyphIndices_;
    std::vector<float>                  glyphAdvances_;
    std::vector<DWRITE_GLYPH_OFFSET>    glyphOffsets_;
};
