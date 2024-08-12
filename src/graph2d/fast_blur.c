// Stack Blur v1.0
//
// Author: Mario Klingemann <mario@quasimondo.com>
// http://incubator.quasimondo.com
// created Feburary 29, 2004
// C version updated and performance optimization by tntmonks(http://tntmonks.cnblogs.com)

// This is a compromise between Gaussian Blur and Box blur
// It creates much better looking blurs than Box Blur, but is
// 7x faster than my Gaussian Blur implementation.
//
// I called it Stack Blur because this describes best how this
// filter works internally: it creates a kind of moving stack
// of colors whilst scanning through the image. Thereby it
// just has to add one new block of color to the right side
// of the stack and remove the leftmost color. The remaining
// colors on the topmost layer of the stack are either added on
// or reduced by one, depending on if they are on the right or
// on the left side of the stack.
//
// If you are using this algorithm in your code please add
// the following line:
//
// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>

#include "fast_blur.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#endif

void fast_blur_gray(unsigned char *pix, unsigned int w, unsigned int h, int radius) {
    unsigned int wm = w - 1;
    unsigned int hm = h - 1;
    unsigned int imageSize = w * h;
    unsigned int div = radius + radius + 1;

    unsigned char *rgb = (unsigned char *)malloc(sizeof(unsigned char) * imageSize);
    unsigned char *r = rgb;
    int rsum, x, y, i, p, yp, yi, yw;

    unsigned int *vmin = (unsigned int *)malloc(MAX(w, h) * sizeof(unsigned int));

    int divsum = (div + 1) >> 1;
    divsum *= divsum;
    int *dv = (int *)malloc(256 * divsum * sizeof(int));
    for (i = 0; i < 256 * divsum; i++) {
        dv[i] = (i / divsum);
    }

    yw = yi = 0;

    int (*stack)[3] = (int(*)[3])malloc(div * 3 * sizeof(int));
    unsigned int stackpointer;
    unsigned int stackstart;
    int *sir;
    int rbs;
    int r1 = radius + 1;
    int routsum;
    int rinsum;

    for (y = 0; y < (int)h; y++) {
        rinsum = routsum = rsum = 0;
        for (i = -radius; i <= radius; i++) {
            p = yi + MIN((int)wm, MAX(i, 0));
            sir = stack[i + radius];
            sir[0] = pix[p];

            rbs = r1 - abs(i);
            rsum += sir[0] * rbs;
            if (i > 0) {
                rinsum += sir[0];
            } else {
                routsum += sir[0];
            }
        }
        stackpointer = radius;

        for (x = 0; x < (int)w; x++) {

            r[yi] = dv[rsum];

            rsum -= routsum;

            stackstart = stackpointer - radius + div;
            sir = stack[stackstart % div];

            routsum -= sir[0];

            if (y == 0) {
                vmin[x] = MIN(x + radius + 1, (int)wm);
            }
            p = yw + vmin[x];


            sir[2] = sir[1] = sir[0] = pix[p];
            rinsum += sir[0];

            rsum += rinsum;

            stackpointer = (stackpointer + 1) % div;
            sir = stack[stackpointer % div];

            routsum += sir[0];

            rinsum -= sir[0];

            yi++;
        }
        yw += w;
    }
    for (x = 0; x < (int)w; x++) {
        rinsum = routsum = rsum = 0;
        yp = -radius * w;
        for (i = -radius; i <= radius; i++) {
            yi = MAX(0, yp) + x;

            sir = stack[i + radius];

            sir[0] = r[yi];

            rbs = r1 - abs(i);

            rsum += r[yi] * rbs;

            if (i > 0) {
                rinsum += sir[0];
            } else {
                routsum += sir[0];
            }

            if (i < (int)hm) {
                yp += w;
            }
        }
        yi = x;
        stackpointer = radius;
        for (y = 0; y < (int)h; y++) {
            pix[yi] = dv[rsum];
            rsum -= routsum;

            stackstart = stackpointer - radius + div;
            sir = stack[stackstart % div];

            routsum -= sir[0];

            if (x == 0) {
                vmin[y] = MIN(y + r1, (int)hm) * w;
            }
            p = x + (int)vmin[y];

            sir[0] = r[p];

            rinsum += sir[0];

            rsum += rinsum;

            stackpointer = (stackpointer + 1) % div;
            sir = stack[stackpointer];

            routsum += sir[0];

            rinsum -= sir[0];

            yi += w;
        }
    }

    free(rgb);
    free(vmin);
    free(dv);
    free(stack);
}

void fast_blur_rgb(unsigned char *pix, unsigned int w, unsigned int h, unsigned int comp, int radius) {
    unsigned int wm = w - 1;
    unsigned int hm = h - 1;
    unsigned int imageSize = w * h;
    unsigned int div = radius + radius + 1;

    unsigned char *rgb = (unsigned char *)malloc(sizeof(unsigned char) * imageSize * 3);
    unsigned char *r = rgb;
    unsigned char *g = rgb + imageSize;
    unsigned char *b = rgb + imageSize * 2;
    int rsum, gsum, bsum, x, y, i, p, yp, yi, yw;

    unsigned int *vmin = (unsigned int *)malloc(MAX(w, h) * sizeof(unsigned int));

    int divsum = (div + 1) >> 1;
    divsum *= divsum;
    int *dv = (int *)malloc(256 * divsum * sizeof(int));
    for (i = 0; i < 256 * divsum; i++) {
        dv[i] = (i / divsum);
    }

    yw = yi = 0;

    int(*stack)[3] = (int(*)[3])malloc(div * 3 * sizeof(int));
    unsigned int stackpointer;
    unsigned int stackstart;
    int *sir;
    int rbs;
    int r1 = radius + 1;
    int routsum, goutsum, boutsum;
    int rinsum, ginsum, binsum;

    for (y = 0; y < (int)h; y++) {
        rinsum = ginsum = binsum = routsum = goutsum = boutsum = rsum = gsum = bsum = 0;
        for (i = -radius; i <= radius; i++) {
            p = yi + MIN((int)wm, MAX(i, 0));
            sir = stack[i + radius];
            sir[0] = pix[(p*comp)];
            sir[1] = pix[(p*comp) + 1];
            sir[2] = pix[(p*comp) + 2];

            rbs = r1 - abs(i);
            rsum += sir[0] * rbs;
            gsum += sir[1] * rbs;
            bsum += sir[2] * rbs;
            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
            }
        }
        stackpointer = radius;

        for (x = 0; x < (int)w; x++) {

            r[yi] = dv[rsum];
            g[yi] = dv[gsum];
            b[yi] = dv[bsum];

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;

            stackstart = stackpointer - radius + div;
            sir = stack[stackstart % div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];

            if (y == 0) {
                vmin[x] = MIN(x + radius + 1, (int)wm);
            }
            p = yw + vmin[x];


            sir[0] = pix[(p*comp)];
            sir[1] = pix[(p*comp) + 1];
            sir[2] = pix[(p*comp) + 2];
            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;

            stackpointer = (stackpointer + 1) % div;
            sir = stack[(stackpointer) % div];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];

            yi++;
        }
        yw += w;
    }
    for (x = 0; x < (int)w; x++) {
        rinsum = ginsum = binsum = routsum = goutsum = boutsum = rsum = gsum = bsum = 0;
        yp = -radius * w;
        for (i = -radius; i <= radius; i++) {
            yi = MAX(0, yp) + x;

            sir = stack[i + radius];

            sir[0] = r[yi];
            sir[1] = g[yi];
            sir[2] = b[yi];

            rbs = r1 - abs(i);

            rsum += r[yi] * rbs;
            gsum += g[yi] * rbs;
            bsum += b[yi] * rbs;

            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
            }

            if (i < (int)hm) {
                yp += w;
            }
        }
        yi = x;
        stackpointer = radius;
        for (y = 0; y < (int)h; y++) {
            pix[(yi*comp)] = dv[rsum];
            pix[(yi*comp) + 1] = dv[gsum];
            pix[(yi*comp) + 2] = dv[bsum];
            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;

            stackstart = stackpointer - radius + div;
            sir = stack[stackstart % div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];

            if (x == 0) {
                vmin[y] = MIN(y + r1, (int)hm) * w;
            }
            p = x + (int)vmin[y];

            sir[0] = r[p];
            sir[1] = g[p];
            sir[2] = b[p];

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;

            stackpointer = (stackpointer + 1) % div;
            sir = stack[stackpointer];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];

            yi += w;
        }
    }

    free(rgb);
    free(vmin);
    free(dv);
    free(stack);
}
