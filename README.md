libProfiler
===========


libProfiler is a tool to measure time taken by a code portion. It can help you improve code performance.
It can be easily integrated in your toolchain, continuous integration,...
It's implemented as only one multiplatform and threadsafe C++ header file! It works on Windows, Linux and MacOSX.
As you define the granularity, it may be more usefull than great tools like Verysleepy. And it
works well with debug info turned off and full optmisation turned on.
Sadly, it uses STL (nobody's perfect)

How to use it:

Include this header. For one of .cpp where it's included add:

    #define LIB_PROFILER_IMPLEMENTATION

In your project preprocessor, define USE_PROFILER. If USE_PROFILER is not defined, every
libProfiler macro is defined empty. So depending on your project target, you can enable/disable
profiling.

Let's see an example code :


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


Enable/disable profiling with USE_PROFILER.
In one of your .cpp file, define LIB_PROFILER_IMPLEMENTATION or you'll have troubles linking.
You can overide the default printf output redefining preprocessor LIB_PROFILER_PRINTF.
The sample will output:
    Hello, World!
    v = -1530.3564
    v = -190.7513
    Profiler:CALLSTACK of Thread 0
    Profiler:_______________________________________________________________________________________
    Profiler:| Total time   | Avg Time     |  Min time    |  Max time    | Calls  | Section
    Profiler:_______________________________________________________________________________________
    Profiler:|      79.0000 |      79.0000 |      79.0000 |      79.0000 |     1  | Main
    Profiler:|      79.0000 |      39.5000 |      38.0000 |      41.0000 |     2  |   myFunction
    Profiler:_______________________________________________________________________________________
    
    Profiler:
    
    Profiler:DUMP of Thread 0
    Profiler:_______________________________________________________________________________________
    Profiler:| Total time   | Avg Time     |  Min time    |  Max time    | Calls  | Section
    Profiler:_______________________________________________________________________________________
    Profiler:|      79.0000 |      79.0000 |      79.0000 |      79.0000 |      1 | Main
    Profiler:|      79.0000 |      39.5000 |      38.0000 |      41.0000 |      2 | myFunction
    Profiler:_______________________________________________________________________________________

The first list correspond to the callstack ( with left spaced function name). You might see a
a profiled block multiple time depending on where it was called.
The second list is a flat one. Profiled block only appear once.
Time unit is ms.
    
This text is also present in libProfiler.h
