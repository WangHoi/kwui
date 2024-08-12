#pragma once

class JuceImage
{
public:
    enum Format
    {
        ARGB,
        RGB,
        SingleChannel,
    };
    int pixelStride;
    int lineStride;

    static JuceImage fromSingleChannel(int w, int h, void* data, int stride);
    unsigned char* getLinePointer(int row) const { return data + lineStride * row; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getFormat() const { return format; }

private:
    int width;
    int height;
    Format format;

    unsigned char* data;
};

void applyStackBlur(JuceImage& img, int radius);
