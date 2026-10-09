#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include "3dstypes.h"
#include "3dslodepng.h"
#include "3dssplash.h"

#define SCREEN_WIDTH    400
#define SCREEN_HEIGHT   240

// The artwork is built into the emulator. data/splash_logo.png and
// data/splash_background.png are turned into these symbols by the
// Makefile (the %.png.o rule), so changing the images only takes
// replacing those files and running make.
extern "C"
{
    extern const u8 splash_logo_png[];
    extern const u8 splash_logo_png_end[];
    extern const u8 splash_background_png[];
    extern const u8 splash_background_png_end[];
}

// ---- Tweaks ----------------------------------------------------------

// The animation advances once every this many frames (2 = 30 fps).
#define SPLASH_FRAME_INTERVAL       2

// Vertical centre of the logo when it is at rest.
#define LOGO_CENTER_Y               120

// The background moves up one pixel every this many steps.
#define BACKGROUND_SCROLL_STEPS     1

// How much the background is darkened (0 = not at all, 255 = black).
#define BACKGROUND_DIM              110

// Colours of the gradient used when the background can’t be decoded.
#define GRADIENT_TOP                0x2A2F45
#define GRADIENT_BOTTOM             0x0B0D16

// Images with more pixels than this are not loaded.
#define MAX_IMAGE_PIXELS            (2 * 1024 * 1024)

// ----------------------------------------------------------------------


// An image in the layout of the 3DS framebuffer: one column of
// <height> pixels per x, bottom pixel first, each pixel as RGBA8
// with red in the highest byte. Pixel (x, y) is at
// pixels[x * height + (height - 1 - y)].
typedef struct
{
    u32     *pixels;
    int     width;
    int     height;
} SSplashImage;

static bool         splashActive = false;
static int          splashStep = 0;
static int          splashSkipped = 0;

static SSplashImage splashLogo = { NULL, 0, 0 };
static SSplashImage splashBackground = { NULL, 0, 0 };
static bool         splashShowGradient = false;
static u32          splashGradient[SCREEN_HEIGHT];      // one column, framebuffer order


static void splashFreeImage(SSplashImage *image)
{
    free(image->pixels);
    image->pixels = NULL;
    image->width = 0;
    image->height = 0;
}


//---------------------------------------------------------
// Converts RGBA bytes (as decoded by lodepng) to the
// framebuffer layout, scaling the image down (nearest
// neighbour) if it doesn't fit the screen and darkening it
// by dim/255. Transparent pixels are kept transparent when
// keepAlpha is set, otherwise they are blended against black.
//---------------------------------------------------------
static bool splashConvert(SSplashImage *out, const unsigned char *rgba,
    unsigned srcWidth, unsigned srcHeight, int dim, bool keepAlpha, bool fitScreen)
{
    out->pixels = NULL;
    out->width = out->height = 0;

    if (!rgba || !srcWidth || !srcHeight || (u64)srcWidth * srcHeight > MAX_IMAGE_PIXELS)
        return false;

    int width = srcWidth;
    int height = srcHeight;
    if (fitScreen && (width > SCREEN_WIDTH || height > SCREEN_HEIGHT))
    {
        if ((u64)srcWidth * SCREEN_HEIGHT > (u64)srcHeight * SCREEN_WIDTH)
        {
            width = SCREEN_WIDTH;
            height = (u64)srcHeight * SCREEN_WIDTH / srcWidth;
        }
        else
        {
            height = SCREEN_HEIGHT;
            width = (u64)srcWidth * SCREEN_HEIGHT / srcHeight;
        }
        if (width < 1) width = 1;
        if (height < 1) height = 1;
    }

    u32 *pixels = (u32 *) malloc((size_t)width * height * sizeof(u32));
    if (!pixels)
        return false;

    int keep = 255 - dim;
    if (keep < 0) keep = 0;

    for (int x = 0; x < width; x++)
    {
        int sx = (int)((u64)x * srcWidth / width);
        for (int y = 0; y < height; y++)
        {
            int sy = (int)((u64)y * srcHeight / height);
            const unsigned char *p = rgba + ((size_t)sy * srcWidth + sx) * 4;
            u32 r = p[0], g = p[1], b = p[2], a = p[3];

            if (!keepAlpha)
            {
                r = r * a / 255;
                g = g * a / 255;
                b = b * a / 255;
                a = 255;
            }
            if (dim)
            {
                r = r * keep / 255;
                g = g * keep / 255;
                b = b * keep / 255;
            }
            pixels[x * height + (height - 1 - y)] = (r << 24) | (g << 16) | (b << 8) | a;
        }
    }

    out->pixels = pixels;
    out->width = width;
    out->height = height;
    return true;
}


//---------------------------------------------------------
// Decodes a PNG that is built into the emulator. Returns
// false if it isn't a usable PNG.
//---------------------------------------------------------
static bool splashLoad(SSplashImage *image, const u8 *pngStart, const u8 *pngEnd, int dim, bool keepAlpha, bool fitScreen)
{
    unsigned char *rgba = NULL;
    unsigned width = 0, height = 0;

    if (lodepng_decode32(&rgba, &width, &height, pngStart, (size_t)(pngEnd - pngStart)))
    {
        free(rgba);
        return false;
    }
    bool ok = splashConvert(image, rgba, width, height, dim, keepAlpha, fitScreen);
    free(rgba);
    return ok;
}


static void splashBuildGradient()
{
    int r0 = (GRADIENT_TOP >> 16) & 0xff, g0 = (GRADIENT_TOP >> 8) & 0xff, b0 = GRADIENT_TOP & 0xff;
    int r1 = (GRADIENT_BOTTOM >> 16) & 0xff, g1 = (GRADIENT_BOTTOM >> 8) & 0xff, b1 = GRADIENT_BOTTOM & 0xff;

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        u32 r = r0 + (r1 - r0) * y / (SCREEN_HEIGHT - 1);
        u32 g = g0 + (g1 - g0) * y / (SCREEN_HEIGHT - 1);
        u32 b = b0 + (b1 - b0) * y / (SCREEN_HEIGHT - 1);
        splashGradient[SCREEN_HEIGHT - 1 - y] = (r << 24) | (g << 16) | (b << 8) | 0xff;
    }
}


//---------------------------------------------------------
// Draws one frame of the animation into the top screen's
// framebuffer.
//---------------------------------------------------------
static void splashDrawFrame(u32 *fb, int step)
{
    // Background
    //
    if (splashBackground.pixels)
    {
        const SSplashImage *bg = &splashBackground;
        int scroll = (step / BACKGROUND_SCROLL_STEPS) % bg->height;

        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            const u32 *src = bg->pixels + (x % bg->width) * bg->height;
            u32 *dst = fb + x * SCREEN_HEIGHT;
            int row = scroll;
            for (int y = 0; y < SCREEN_HEIGHT; y++)
            {
                dst[SCREEN_HEIGHT - 1 - y] = src[bg->height - 1 - row];
                if (++row == bg->height)
                    row = 0;
            }
        }
    }
    else if (splashShowGradient)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
            memcpy(fb + x * SCREEN_HEIGHT, splashGradient, sizeof(splashGradient));
    }
    else
    {
        memset(fb, 0, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    }

    // Logo
    //
    if (splashLogo.pixels)
    {
        const SSplashImage *logo = &splashLogo;
        int left = (SCREEN_WIDTH - logo->width) / 2;
        int top = LOGO_CENTER_Y - logo->height / 2;

        int y0 = top < 0 ? -top : 0;
        int y1 = top + logo->height > SCREEN_HEIGHT ? SCREEN_HEIGHT - top : logo->height;

        for (int lx = 0; lx < logo->width; lx++)
        {
            int x = left + lx;
            if (x < 0 || x >= SCREEN_WIDTH)
                continue;

            const u32 *src = logo->pixels + lx * logo->height;
            u32 *dst = fb + x * SCREEN_HEIGHT;
            for (int ly = y0; ly < y1; ly++)
            {
                u32 s = src[logo->height - 1 - ly];
                u32 a = s & 0xff;
                if (a == 0)
                    continue;

                u32 *d = &dst[SCREEN_HEIGHT - 1 - (top + ly)];
                if (a == 255)
                {
                    *d = s | 0xff;
                    continue;
                }

                a += a >> 7;        // 0..256, so that opaque is exact
                u32 dv = *d;
                int sr = s >> 24, sg = (s >> 16) & 0xff, sb = (s >> 8) & 0xff;
                int dr = dv >> 24, dg = (dv >> 16) & 0xff, db = (dv >> 8) & 0xff;
                u32 r = dr + (((sr - dr) * (int)a) >> 8);
                u32 g = dg + (((sg - dg) * (int)a) >> 8);
                u32 b = db + (((sb - db) * (int)a) >> 8);
                *d = (r << 24) | (g << 16) | (b << 8) | 0xff;
            }
        }
    }
}


static void splashPresentFrame()
{
    u32 *fb = (u32 *) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    splashDrawFrame(fb, splashStep);
    GSPGPU_FlushDataCache(fb, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
}


void splash3dsBegin()
{
    splashActive = true;
    splashStep = 0;
    splashSkipped = 0;
    splashShowGradient = false;

    bool haveLogo = splashLoad(&splashLogo, splash_logo_png, splash_logo_png_end, 0, true, true);
    bool haveBackground = splashLoad(&splashBackground, splash_background_png, splash_background_png_end,
        BACKGROUND_DIM, false, false);

    if (haveLogo && !haveBackground)
    {
        splashBuildGradient();
        splashShowGradient = true;
    }

    // Draw straight into the displayed framebuffer: the menu
    // swaps both screens' buffers whenever it redraws the
    // bottom screen.
    gfxSetDoubleBuffering(GFX_TOP, false);
    splashPresentFrame();
}


void splash3dsTick()
{
    if (!splashActive)
        return;

    // Nothing to animate if neither image could be decoded.
    if (!splashLogo.pixels && !splashBackground.pixels)
        return;

    if (++splashSkipped < SPLASH_FRAME_INTERVAL)
        return;
    splashSkipped = 0;

    splashStep++;
    splashPresentFrame();
}


void splash3dsEnd()
{
    splashActive = false;
    splashFreeImage(&splashLogo);
    splashFreeImage(&splashBackground);
    gfxSetDoubleBuffering(GFX_TOP, true);
}
