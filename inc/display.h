#pragma once


#if (USE_COLOR == COLOR_GREEN)
    #define COLOR_OCT_1 255
    #define COLOR_OCT_2 0 
    #define COLOR_OCT_3 255
#elif (USE_COLOR == COLOR_BLUE )
    #define COLOR_OCT_1 0
    #define COLOR_OCT_2 255
    #define COLOR_OCT_3 255
#elif (USE_COLOR == COLOR_RED)
    #define COLOR_OCT_1 255
    #define COLOR_OCT_2 255
    #define COLOR_OCT_3 0
#else
    #define COLOR_OCT_1 0
    #define COLOR_OCT_2 0
    #define COLOR_OCT_3 0
#endif

void run_display_task(void *arg);
