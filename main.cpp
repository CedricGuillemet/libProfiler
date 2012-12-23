//
//  main.cpp
//  libProfiler
//
//  Created by Cedric Guillemet on 12/23/12.
//  Copyright (c) 2012 Cedric Guillemet. All rights reserved.
//

#include <iostream>
#include <math.h>

void myPrintf( const char *szText )
{
    printf("Profiler:%s", szText);
}


#define USE_PROFILER 1
#define LIB_PROFILER_IMPLEMENTATION
#define LIB_PROFILER_PRINTF myPrintf
#include "libProfiler.h"


void myFunction()
{
    PROFILER_START(myFunction);
    float v = 0;
    for(int i = 0;i<1000000;i++)
        v += cosf(static_cast<float>(rand()));

    printf("v = %5.4f\n", v);
    PROFILER_END();
}

int main(int argc, const char * argv[])
{
    PROFILER_ENABLE;
    
    PROFILER_START(Main);

    std::cout << "Hello, World!\n";
    myFunction();
    myFunction();    

    PROFILER_END();

    LogProfiler();
    
    PROFILER_DISABLE;
    
    return 0;
}

