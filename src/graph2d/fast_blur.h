#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void fast_blur_gray(unsigned char *pix, unsigned int w, unsigned int h, int radius);
void fast_blur_rgb(unsigned char *pix, unsigned int w, unsigned int h, unsigned int comp, int radius);

#ifdef __cplusplus
}
#endif
