#pragma once

#ifdef DEBUG
    #define D(...) printf(__VA_ARGS__)
#else
    #define D(...)
#endif