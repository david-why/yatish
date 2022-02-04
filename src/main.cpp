#ifndef __INT24_TYPE__
#define __INT24_TYPE__ int
#define __INT24_MAX__ 0x7fffff
#define __INT24_MIN__ (~__INT24_MAX__)
typedef __INT24_TYPE__ int24_t;
#define __UINT24_TYPE__ unsigned int
#define __UINT24_MAX__ 0xffffff
typedef __UINT24_TYPE__ uint24_t;
#endif

#include <tice.h>
#include <graphx.h>

#include "gfxutils.h"
#include "gfx/gfx.h"

#include "yatish.hpp"

int main()
{
    if (yatish_Init())
        yatish_Run();
    yatish_Deinit();

    return 0;
}
