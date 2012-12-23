///////////////////////////////////////////////////////////////////////////////////////////////////
//  
//  ___        __       ____                  ___      ___
// /\_ \    __/\ \     /\  _`\              /'___\ __ /\_ \
// \//\ \  /\_\ \ \____\ \ \L\ \_ __   ___ /\ \__//\_\\//\ \      __   _ __
//   \ \ \ \/\ \ \ '__`\\ \ ,__/\`'__\/ __`\ \ ,__\/\ \ \ \ \   /'__`\/\`'__\
//    \_\ \_\ \ \ \ \L\ \\ \ \/\ \ \//\ \L\ \ \ \_/\ \ \ \_\ \_/\  __/\ \ \/
//    /\____\\ \_\ \_,__/ \ \_\ \ \_\\ \____/\ \_\  \ \_\/\____\ \____\\ \_\
//    \/____/ \/_/\/___/   \/_/  \/_/ \/___/  \/_/   \/_/\/____/\/____/ \/_/
//
//
//  Copyright (C) 2007-2013 Cedric Guillemet
//
//  Contact  : cedric.guillemet@gmail.com
//  Twitter  : http://twitter.com/skaven_
//  Web home : http://skaven.fr
//
//  Made with the great help of Christophe Giraud and Maxime Houlier.
//
//    libProfiler is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    libProfiler is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with libProfiler.  If not, see <http://www.gnu.org/licenses/>
//
//
// Changelog:
// 23/12/12 : Initial release
//
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// libProfiler is a tool to measure time taken by a code portion. It can help you improve code performance.
// It can be easily integrated in your toolchain, continuous integration,...
// It's implemented as only one C++ header file!
// As you define the granularity, it may be more usefull than great tools like Verysleepy. And it
// works well with debug info turned off and full optmisation turned on.
// Sadly, it uses STL (nobody's perfect)
//
// How to use it:
//
// Include this header. For one of .cpp where it's included add:
// #define LIB_PROFILER_IMPLEMENTATION
//
// In your project preprocessor, define USE_PROFILER. If USE_PROFILER is not defined, every
// libProfiler macro is defined empty. So depending on your project target, you can enable/disable
// profiling.
//
// Let's see an example code :
//
//
// #include <iostream>
// #include <math.h>
//
// void myPrintf( const char *szText )
// {
//     printf("Profiler:%s", szText);
// }
//
//
// #define USE_PROFILER 1
// #define LIB_PROFILER_IMPLEMENTATION
// #define LIB_PROFILER_PRINTF myPrintf
// #include "libProfiler.h"
//
//
// void myFunction()
// {
//     PROFILER_START(myFunction);
//     float v = 0;
//     for(int i = 0;i<1000000;i++)
//         v += cosf(static_cast<float>(rand()));
//
//     printf("v = %5.4f\n", v);
//     PROFILER_END();
// }
//
// int main(int argc, const char * argv[])
// {
//     PROFILER_ENABLE;
//
//     PROFILER_START(Main);
//
//     std::cout << "Hello, World!\n";
//     myFunction();
//     myFunction();
//
//     PROFILER_END();
//
//     LogProfiler();
//
//     PROFILER_DISABLE;
//
//     return 0;
// }
//
//
//
// Enable/disable profiling with USE_PROFILER.
// In one of your .cpp file, define LIB_PROFILER_IMPLEMENTATION or you'll have troubles linking.
// You can overide the default printf output redefining preprocessor LIB_PROFILER_PRINTF.
// The sample will output:
//
// Hello, World!
// v = -1530.3564
// v = -190.7513
// Profiler:CALLSTACK of Thread 0
// Profiler:_______________________________________________________________________________________
// Profiler:| Total time   | Avg Time     |  Min time    |  Max time    | Calls  | Section
// Profiler:_______________________________________________________________________________________
// Profiler:|      79.0000 |      79.0000 |      79.0000 |      79.0000 |     1  | Main
// Profiler:|      79.0000 |      39.5000 |      38.0000 |      41.0000 |     2  |   myFunction
// Profiler:_______________________________________________________________________________________
//
// Profiler:
//
// Profiler:DUMP of Thread 0
// Profiler:_______________________________________________________________________________________
// Profiler:| Total time   | Avg Time     |  Min time    |  Max time    | Calls  | Section
// Profiler:_______________________________________________________________________________________
// Profiler:|      79.0000 |      79.0000 |      79.0000 |      79.0000 |      1 | Main
// Profiler:|      79.0000 |      39.5000 |      38.0000 |      41.0000 |      2 | myFunction
// Profiler:_______________________________________________________________________________________
//
// The first list correspond to the callstack ( with left spaced function name). You might see a
// a profiled block multiple time depending on where it was called.
// The second list is a flat one. Profiled block only appear once.
// Time unit is ms
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LIBPROFILER_H__
#define LIBPROFILER_H__

///////////////////////////////////////////////////////////////////////////////////////////////////
// includes

#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <map>
#include <string>

///////////////////////////////////////////////////////////////////////////////////////////////////
// PROFILE/LOG

#ifndef ProfilerPrintf
#define ProfilerPrintf printf
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// OS definition

#define PLATFORM_OS_WINDOWS    1
#define PLATFORM_OS_LINUX      2
#define PLATFORM_OS_MACOSX     3

//NOTE: all predefine C/C++ compiler macros: http://sourceforge.net/apps/mediawiki/predef/

#if defined( __WIN32__ ) || defined( _WIN32 ) || defined( __WIN64__ ) || defined( _WIN64 ) || defined( WIN32 )
#   define IS_OS_WINDOWS    1
#   define IS_OS_LINUX      0
#   define IS_OS_MACOSX     0
#   define PLATFORM_OS      PLATFORM_OS_WINDOWS
#   pragma message("Platform OS is Windows.")
#elif defined(__linux__) || defined( LINUX )
#   define IS_OS_WINDOWS    0
#   define IS_OS_LINUX      1
#   define IS_OS_MACOSX     0
#   define PLATFORM_OS      PLATFORM_OS_LINUX
#   pragma message("Platform OS is Linux.")
#elif ( defined(__APPLE__) && defined(__MACH__) )  || defined( MACOSX )
#   define IS_OS_WINDOWS    0
#   define IS_OS_LINUX      0
#   define IS_OS_MACOSX     1
#   define PLATFORM_OS      PLATFORM_OS_MACOSX
#   pragma message("Platform OS is MacOSX.")
#else
#   error "This platform is not supported."
#endif


#define PLATFORM_COMPILER_MSVC  1
#define PLATFORM_COMPILER_GCC   2

#if defined( _MSC_VER )
#   define PLATFORM_COMPILER            PLATFORM_COMPILER_MSVC
#   define PLATFORM_COMPILER_VERSION    _MSC_VER
#   define IS_COMPILER_MSVC     1
#   define IS_COMPILER_GCC      0
#   pragma message("Platform Compiler is Microsoft Visual C++.")
#elif defined( __GNUC__ )
#   define PLATFORM_COMPILER            PLATFORM_COMPILER_GCC
#   define PLATFORM_COMPILER_VERSION    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100)
#   define IS_COMPILER_MSVC     0
#   define IS_COMPILER_GCC      1
#   pragma message("Platform Compiler is GCC.")
#else
#   error "This compiler is not supported."
#endif


#define PLATFORM_MEMORY_ADDRESS_SPACE_32BIT  1
#define PLATFORM_MEMORY_ADDRESS_SPACE_64BIT  2

#if defined(__x86_64__) || defined(_M_X64) || defined(__powerpc64__)
#   define IS_PLATFORM_64BIT                1
#   define IS_PLATFORM_32BIT                0
#   define PLATFORM_MEMORY_ADDRESS_SPACE    PLATFORM_MEMORY_ADDRESS_SPACE_64BIT
#   pragma message("Using 64bit memory address space.")
#else
#   define IS_PLATFORM_64BIT                0
#   define IS_PLATFORM_32BIT                1
#   define PLATFORM_MEMORY_ADDRESS_SPACE    PLATFORM_MEMORY_ADDRESS_SPACE_32BIT
#   pragma message("Using 32bit memory address space.")
#endif

#if IS_OS_WINDOWS
#if !defined(snprintf)
#define snprintf sprintf_s
#endif
#endif

#if IS_OS_WINDOWS
#if !defined(vnsprintf)
#define vnsprintf vsprintf_s
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#define	_NAME_SEPARATOR_			"|"
#define	_THREADID_NAME_SEPARATOR_	"@"

#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)


inline void LOG(const char *format,...)
{
    va_list ptr_arg;
    va_start( ptr_arg, format );

    static char tmps[1024];
    vsprintf( tmps, format, ptr_arg );

    LIB_PROFILER_PRINTF( tmps );
    
    va_end(ptr_arg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// critical section

#if USE_PROFILER

// Critical Section
#if IS_OS_WINDOWS
typedef CRITICAL_SECTION ZCriticalSection_t;
inline char* ZGetCurrentDirectory(int bufLength, char *pszDest)
{
    return (char*)GetCurrentDirectoryA(bufLength, pszDest);
}

#elif IS_OS_LINUX
#include <pthread.h>
typedef pthread_mutex_t ZCriticalSection_t;
inline char* ZGetCurrentDirectory(int bufLength, char *pszDest)
{
    return getcwd(pszDest, bufLength);
}

#elif IS_OS_MACOSX
#import <CoreServices/CoreServices.h>
typedef MPCriticalRegionID ZCriticalSection_t;
inline char* ZGetCurrentDirectory(int bufLength, char *pszDest)
{
    return getcwd(pszDest, bufLength);
}
#endif


__inline ZCriticalSection_t* NewCriticalSection()
{
#if IS_OS_LINUX
	ZCriticalSection_t *cs = new pthread_mutex_t;
	//(*cs) = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init (cs, NULL);
	return cs;
#elif IS_OS_MACOSX
	MPCriticalRegionID* criticalRegion = new MPCriticalRegionID;
	OSStatus err = MPCreateCriticalRegion (criticalRegion);
	if (err != 0)
	{
		delete criticalRegion;
		criticalRegion = NULL;
	}
	
	return criticalRegion;
#elif IS_OS_WINDOWS
	CRITICAL_SECTION *cs = new CRITICAL_SECTION;
	InitializeCriticalSection(cs);
	return cs;
#endif
}

__inline void DestroyCriticalSection(ZCriticalSection_t *cs)
{
#if IS_OS_LINUX
	delete cs;
#elif IS_OS_MACOSX
	MPDeleteCriticalRegion(*cs);
#elif IS_OS_WINDOWS
	DeleteCriticalSection(cs);
	delete cs;
#endif
}

__inline void LockCriticalSection(ZCriticalSection_t *cs)
{
#if IS_OS_LINUX
	pthread_mutex_lock( cs );
#elif IS_OS_MACOSX
	MPEnterCriticalRegion(*cs, kDurationForever);
#elif IS_OS_WINDOWS
	EnterCriticalSection(cs);
#endif
}

__inline void UnLockCriticalSection(ZCriticalSection_t *cs)
{
#if IS_OS_LINUX
	pthread_mutex_unlock( cs );
#elif IS_OS_MACOSX
	MPExitCriticalRegion(*cs);
#elif IS_OS_WINDOWS
	LeaveCriticalSection(cs);
#endif 
}


bool Zprofiler_enable();
void Zprofiler_disable();
void Zprofiler_start( const char *profile_name );
void Zprofiler_end( );
void LogProfiler();

//defines

#define PROFILER_ENABLE Zprofiler_enable()
#define PROFILER_DISABLE Zprofiler_disable()
#define PROFILER_START(x) Zprofiler_start(QUOTE(x))
#define PROFILER_END() Zprofiler_end()

#else

#define LogProfiler()

#define PROFILER_ENABLE
#define PROFILER_DISABLE
#define PROFILER_START(x)
#define PROFILER_END()
#endif

#if USE_PROFILER
#ifdef LIB_PROFILER_IMPLEMENTATION

using namespace std;

void TimerInit();
//#if defined(WIN32)
double	startHighResolutionTimer( void );
/*
 #elif defined(_MAC_)
 void	startHighResolutionTimer(unsigned long time[2]);
 #else
 void	startHighResolutionTimer(unsigned long time[2]);
 #endif
 */
//unsigned long	endHighResolutionTimer(unsigned long time[2]);


#if IS_OS_WINDOWS
// Create A Structure For The Timer Information
struct
{
    __int64			frequency;									// Timer Frequency
    float			resolution;									// Timer Resolution
    unsigned long	mm_timer_start;								// Multimedia Timer Start Value
    unsigned long	mm_timer_elapsed;							// Multimedia Timer Elapsed Time
    bool			performance_timer;							// Using The Performance Timer?
    __int64			performance_timer_start;					// Performance Timer Start Value
    __int64			performance_timer_elapsed;					// Performance Timer Elapsed Time
} timer;														// Structure Is Named timer

#endif  //  IS_OS_WINDOWS


typedef struct stGenProfilerData
{
    double			totalTime;
    double			averageTime;
    double			minTime;
    double			maxTime;
    double			lastTime;				// Time of the previous passage
    double			elapsedTime;			// Elapsed Time
    unsigned long	nbCalls;				// Numbers of calls
    char			szBunchCodeName[1024];	// temporary.
} tdstGenProfilerData;

//  Hold the call stack
typedef std::vector<tdstGenProfilerData> tdCallStackType;

//	Hold the sequence of profiler_starts
std::map<std::string, tdstGenProfilerData> mapProfilerGraph;

// Hold profiler data vector in function of the thread
map<unsigned long, tdCallStackType> mapCallsByThread;

// Critical section
ZCriticalSection_t	*gProfilerCriticalSection;


// Critical section functions
/*
 void NewCriticalSection( void );
 void DestroyCriticalSection( void );
 void LockCriticalSection( void );
 void UnLockCriticalSection( void );
 */

//
// Activate the profiler
//
bool Zprofiler_enable()
{
    // Initialize the timer
    TimerInit();
    
    // Create the mutex
    gProfilerCriticalSection = NewCriticalSection();
    
    // Clear maps
    /*
     mapCallsByThread.clear();
     mapProfilerGraph.clear();
     mapCallsByThread.clear();
     */
    
    
    
    return true;
}

//
// Deactivate the profiler
//
void Zprofiler_disable()
{
    // Dump to file
    //Zprofiler_dumpToFile( DUMP_FILENAME );
    
    // Clear maps
    mapCallsByThread.clear();
    mapProfilerGraph.clear();
    mapCallsByThread.clear();
    
    // Delete the mutex
    DestroyCriticalSection(gProfilerCriticalSection);
}
#if IS_OS_MACOSX
unsigned long GetCurrentThreadId() { return 0; }
#elif IS_OS_LINUX
unsigned long GetCurrentThreadId() { return 0; }
#endif

//
// Start the profiling of a bunch of code
//
void Zprofiler_start( const char *profile_name )
{
    LockCriticalSection(gProfilerCriticalSection);
    
    unsigned long ulThreadId = GetCurrentThreadId();
    
    // Add the profile name in the callstack vector
    tdstGenProfilerData	GenProfilerData;
    memset(&GenProfilerData, 0, sizeof(GenProfilerData));
    GenProfilerData.lastTime	= startHighResolutionTimer();
    GenProfilerData.minTime		= 0xFFFFFFFF;
    
    // Find or add callstack
    tdCallStackType TmpCallStack;
    map<unsigned long, tdCallStackType>::iterator IterCallsByThreadMap = mapCallsByThread.find(ulThreadId);
    if( IterCallsByThreadMap == mapCallsByThread.end() )
    {
        // Not found. So insert the new pair
        mapCallsByThread.insert(std::make_pair(ulThreadId, TmpCallStack));
        IterCallsByThreadMap = mapCallsByThread.find(ulThreadId);
    }
    
    // It's the first element of the vector
    if ((*IterCallsByThreadMap).second.empty())
    {
        GenProfilerData.nbCalls	= 1;
        sprintf(GenProfilerData.szBunchCodeName, "%s%d%s%s", _NAME_SEPARATOR_, (int)ulThreadId, _THREADID_NAME_SEPARATOR_, profile_name);
        (*IterCallsByThreadMap).second.push_back( GenProfilerData );
    }
    // It's not the first element of the vector
    else
    {
        // Update the number of calls
        GenProfilerData.nbCalls++;
        
        // We need to construct the string with the previous value of the
        // profile_start
        char *previousString = (*IterCallsByThreadMap).second[(*IterCallsByThreadMap).second.size()-1].szBunchCodeName;
        
        // Add the current profile start string
        strcpy(GenProfilerData.szBunchCodeName, previousString);
        strcat(GenProfilerData.szBunchCodeName, _NAME_SEPARATOR_);
        strcat(GenProfilerData.szBunchCodeName, profile_name);
        
        // Push it
        (*IterCallsByThreadMap).second.push_back( GenProfilerData );
    }
    
    UnLockCriticalSection(gProfilerCriticalSection);
}

//
// Stop the profiling of a bunch of code
//
void Zprofiler_end( )
{
    unsigned long ulThreadId = GetCurrentThreadId();
    
    // Retrieve the right entry in function of the threadId
    map<unsigned long, tdCallStackType>::iterator IterCallsByThreadMap = mapCallsByThread.find(ulThreadId);
    
    // Check if vector is empty
    if( (*IterCallsByThreadMap).second.empty() )
    {
        LOG( "Il y a une erreur dans le vecteur CallStack !!!\n\n");
        return;
    }
    
    LockCriticalSection(gProfilerCriticalSection);
    
    // Retrieve the last element from the callstack vector
    tdstGenProfilerData	GenProfilerData;
    GenProfilerData	= (*IterCallsByThreadMap).second[(*IterCallsByThreadMap).second.size()-1];
    
    // Compute elapsed time
    GenProfilerData.elapsedTime += startHighResolutionTimer()-GenProfilerData.lastTime;
    GenProfilerData.totalTime	+= GenProfilerData.elapsedTime;
    
    // Find if this entry exists in the map
    std::map<std::string, tdstGenProfilerData>::iterator	IterMap;
    IterMap	= mapProfilerGraph.find( GenProfilerData.szBunchCodeName );
    if( IterMap!=mapProfilerGraph.end() )
    {
        (*IterMap).second.nbCalls++;
        
        // Retrieve time information to compute min and max time
        double minTime		= (*IterMap).second.minTime;
        double maxTime		= (*IterMap).second.maxTime;
        //double totalTime	= (*IterMap).second.totalTime;
        
        if( GenProfilerData.elapsedTime<minTime )
        {
            (*IterMap).second.minTime	= GenProfilerData.elapsedTime;
        }
        
        if( GenProfilerData.elapsedTime>maxTime )
        {
            (*IterMap).second.maxTime	= GenProfilerData.elapsedTime;
        }
        
        // Compute Total Time
        (*IterMap).second.totalTime	   += GenProfilerData.elapsedTime;
        // Compute Average Time
        (*IterMap).second.averageTime	= (*IterMap).second.totalTime/(*IterMap).second.nbCalls;
        
    }
    else
    {
        GenProfilerData.minTime	= GenProfilerData.elapsedTime;
        GenProfilerData.maxTime	= GenProfilerData.elapsedTime;
        
        // Compute Average Time
        GenProfilerData.averageTime	= GenProfilerData.totalTime/GenProfilerData.nbCalls;
        
        // Insert this part of the call stack into the Progiler Graph
        mapProfilerGraph.insert( std::make_pair(GenProfilerData.szBunchCodeName, GenProfilerData) );
    }
    
    // Now, pop back the GenProfilerData from the vector callstack
    (*IterCallsByThreadMap).second.pop_back();
    
    UnLockCriticalSection(gProfilerCriticalSection);
}

//
// Dump all data in a file
//

bool MyDataSortPredicate(const tdstGenProfilerData un, const tdstGenProfilerData deux)
{
    return un.szBunchCodeName > deux.szBunchCodeName;
}

void LogProfiler()
{
    
    // Thread Id String
    char szThreadId[16];
    char textLine[1024];
    char *tmpString;
    
    long i;
    //long nbTreadIds	= 0;
    long size		= 0;
    
    // Map for calls
    std::map<std::string, tdstGenProfilerData> mapCalls;
    std::map<std::string, tdstGenProfilerData>::iterator IterMapCalls;
    mapCalls.clear();
    
    // Map for Thread Ids
    std::map<std::string, int> ThreadIdsCount;
    std::map<std::string, int>::iterator IterThreadIdsCount;
    ThreadIdsCount.clear();
    
    // Vector for callstack
    vector<tdstGenProfilerData> tmpCallStack;
    vector<tdstGenProfilerData>::iterator IterTmpCallStack;
    tmpCallStack.clear();
    
    // Copy map data into a vector in the aim to sort it
    std::map<std::string, tdstGenProfilerData>::iterator	IterMap;
    for(IterMap=mapProfilerGraph.begin(); IterMap!=mapProfilerGraph.end(); ++IterMap)
    {
        strcpy((*IterMap).second.szBunchCodeName, (*IterMap).first.c_str());
        tmpCallStack.push_back( (*IterMap).second );
    }
    
    // Sort the vector
    std::sort(tmpCallStack.begin(), tmpCallStack.end(), MyDataSortPredicate);
    
    // Create a map with thread Ids
    for(IterTmpCallStack=tmpCallStack.begin(); IterTmpCallStack!=tmpCallStack.end(); ++IterTmpCallStack)
    {
        //// DEBUG
        //fprintf(DumpFile, "%s\n", (*IterTmpCallStack).szBunchCodeName );
        //// DEBUG
        
        tmpString	= strstr((*IterTmpCallStack).szBunchCodeName, _THREADID_NAME_SEPARATOR_);
        size		= (long)(tmpString-(*IterTmpCallStack).szBunchCodeName);
        strncpy(szThreadId, (*IterTmpCallStack).szBunchCodeName+1, size-1); szThreadId[size-1] = 0;
        ThreadIdsCount[szThreadId]++;
    }
    
    // Retrieve the number of thread ids
    //unsigned long nbThreadIds	= mapProfilerGraph.size();
    
    IterThreadIdsCount = ThreadIdsCount.begin();
    for(unsigned long nbThread=0;nbThread<ThreadIdsCount.size();nbThread++)
    {
        sprintf(szThreadId, "%s", IterThreadIdsCount->first.c_str() );
        
        LOG("CALLSTACK of Thread %s\n", szThreadId);
        LOG("_______________________________________________________________________________________\n");
        LOG("| Total time   | Avg Time     |  Min time    |  Max time    | Calls  | Section\n");
        LOG("_______________________________________________________________________________________\n");
        
        long nbSeparator = 0;
        for(IterTmpCallStack=tmpCallStack.begin(); IterTmpCallStack!=tmpCallStack.end(); ++IterTmpCallStack)
        {
            tmpString	= (*IterTmpCallStack).szBunchCodeName+1;
            
            if( strstr(tmpString, szThreadId) )
            {
                // Count the number of separator in the string
                nbSeparator	= 0;
                while( *tmpString )
                {
                    if( *tmpString++== '|' )
                    {
                        nbSeparator++;
                    }
                }
                
                // Get times and fill in the dislpay string
                sprintf(textLine, "| %12.4f | %12.4f | %12.4f | %12.4f |%6d  | ",
                        (*IterTmpCallStack).totalTime,
                        (*IterTmpCallStack).averageTime,
                        (*IterTmpCallStack).minTime,
                        (*IterTmpCallStack).maxTime,
                        (int)(*IterTmpCallStack).nbCalls);
                
                // Get the last start_profile_name in the string
                tmpString = strrchr((*IterTmpCallStack).szBunchCodeName, '|')+1;
                
                IterMapCalls	= mapCalls.find( tmpString );
                if( IterMapCalls!=mapCalls.end() )
                {
                    double minTime			= (*IterMapCalls).second.minTime;
                    double maxTime			= (*IterMapCalls).second.maxTime;
                    //double totalTime		= (*IterMapCalls).second.totalTime;
                    //double averageTime		= (*IterMapCalls).second.averageTime;
                    //unsigned long nbCalls	= (*IterMapCalls).second.nbCalls;
                    
                    if( (*IterTmpCallStack).minTime<minTime )
                    {
                        (*IterMapCalls).second.minTime	= (*IterTmpCallStack).minTime;
                    }
                    if( (*IterTmpCallStack).maxTime>maxTime )
                    {
                        (*IterMapCalls).second.maxTime	= (*IterTmpCallStack).maxTime;
                    }
                    (*IterMapCalls).second.totalTime		+= (*IterTmpCallStack).totalTime;
                    (*IterMapCalls).second.nbCalls			+= (*IterTmpCallStack).nbCalls;
                    (*IterMapCalls).second.averageTime		= (*IterMapCalls).second.totalTime/(*IterMapCalls).second.nbCalls;
                }
                
                else
                {
                    tdstGenProfilerData tgt;// = (*IterMap).second;
                    if( strstr(tmpString, szThreadId) )
                    {
                        strcpy( tgt.szBunchCodeName, tmpString );
                    }
                    else
                    {
                        sprintf(tgt.szBunchCodeName, "%s%s%s",
                                szThreadId,
                                _THREADID_NAME_SEPARATOR_,
                                tmpString);
                    }
                    
                    tgt.minTime			= (*IterTmpCallStack).minTime;
                    tgt.maxTime			= (*IterTmpCallStack).maxTime;
                    tgt.totalTime			= (*IterTmpCallStack).totalTime;
                    tgt.averageTime		= (*IterTmpCallStack).averageTime;
                    tgt.elapsedTime		= (*IterTmpCallStack).elapsedTime;
                    tgt.lastTime			= (*IterTmpCallStack).lastTime;
                    tgt.nbCalls			= (*IterTmpCallStack).nbCalls;
                    
                    mapCalls.insert( std::make_pair(tmpString, tgt) );
                }
                
                
                // Copy white space in the string to format the display
                // in function of the hierarchy
                for(i=0;i<nbSeparator;i++) strcat(textLine, "  ");
                
                // Remove the thred if from the string
                if( strstr(tmpString, _THREADID_NAME_SEPARATOR_) )
                {
                    tmpString += strlen(szThreadId)+1;
                }
                
                // Display the name of the bunch code profiled
                LOG("%s%s\n", textLine, tmpString );
            }
        }
        LOG("_______________________________________________________________________________________\n\n");
        ++IterThreadIdsCount;
    }
    LOG( "\n\n");
    
    //
    //	DUMP CALLS
    //
    IterThreadIdsCount = ThreadIdsCount.begin();
    for(unsigned long nbThread=0;nbThread<ThreadIdsCount.size();nbThread++)
    {
        sprintf(szThreadId, "%s", IterThreadIdsCount->first.c_str() );
        
        LOG( "DUMP of Thread %s\n", szThreadId);
        LOG( "_______________________________________________________________________________________\n");
        LOG( "| Total time   | Avg Time     |  Min time    |  Max time    | Calls  | Section\n");
        LOG( "_______________________________________________________________________________________\n");
        
        for(IterMapCalls=mapCalls.begin(); IterMapCalls!=mapCalls.end(); ++IterMapCalls)
        {
            tmpString = (*IterMapCalls).second.szBunchCodeName;
            if( strstr(tmpString, szThreadId) )
            {
                LOG( "| %12.4f | %12.4f | %12.4f | %12.4f | %6d | %s\n",
                    (*IterMapCalls).second.totalTime,
                    (*IterMapCalls).second.averageTime,
                    (*IterMapCalls).second.minTime,
                    (*IterMapCalls).second.maxTime,
                    (int)(*IterMapCalls).second.nbCalls,
                    (*IterMapCalls).second.szBunchCodeName+strlen(szThreadId)+1);
            }
        }
        LOG( "_______________________________________________________________________________________\n\n");
        ++IterThreadIdsCount;
    }
    
}

////
////	Gestion des timers
////
#if IS_OS_WINDOWS

// Initialize Our Timer (Get It Ready)
void TimerInit()
{
    memset(&timer, 0, sizeof(timer));
    
    // Check to see if a performance counter is available
    // If one is available the timer frequency will be updated
    if( !QueryPerformanceFrequency((LARGE_INTEGER *) &timer.frequency) )
    {
        // No performace counter available
        timer.performance_timer	= false;					// Set performance timer to false
        timer.mm_timer_start	= timeGetTime();			// Use timeGetTime() to get current time
        timer.resolution		= 1.0f/1000.0f;				// Set our timer resolution to .001f
        timer.frequency			= 1000;						// Set our timer frequency to 1000
        timer.mm_timer_elapsed	= timer.mm_timer_start;		// Set the elapsed time to the current time
    }
    else
    {
        // Performance counter is available, use it instead of the multimedia timer
        // Get the current time and store it in performance_timer_start
        QueryPerformanceCounter((LARGE_INTEGER *) &timer.performance_timer_start);
        timer.performance_timer			= true;				// Set performance timer to true
        
        // Calculate the timer resolution using the timer frequency
        timer.resolution				= (float) (((double)1.0f)/((double)timer.frequency));
        
        // Set the elapsed time to the current time
        timer.performance_timer_elapsed	= timer.performance_timer_start;
    }
}

// platform specific get hires times...
double startHighResolutionTimer()
{
    __int64 time;
    
    // Are we using the performance timer?
    if( timer.performance_timer )
    {
        // Grab the current performance time
        QueryPerformanceCounter((LARGE_INTEGER *) &time);
        
        // Return the current time minus the start time multiplied
        // by the resolution and 1000 (To Get MS)
        return ( (double) ( time - timer.performance_timer_start) * timer.resolution)*1000.0f;
    }
    else
    {
        // Return the current time minus the start time multiplied
        // by the resolution and 1000 (To Get MS)
        return( (double) ( timeGetTime() - timer.mm_timer_start) * timer.resolution)*1000.0f;
    }
}
/*
 unsigned long endHighResolutionTimer(unsigned long time[2])
 {
 unsigned long ticks=0;
 //__asm__ __volatile__(
 //   "rdtsc\n"
 //   "sub  0x4(%%ecx),  %%edx\n"
 //   "sbb  (%%ecx),  %%eax\n"
 //   : "=a" (ticks) : "c" (time)
 //   );
 return ticks;
 }
 */
#elif IS_OS_MACOSX

// Initialize Our Timer (Get It Ready)
void TimerInit()
{
}

double startHighResolutionTimer()
{
    UnsignedWide t;
    Microseconds(&t);
    /*time[0] = t.lo;
     time[1] = t.hi;
     */
    double ms = double(t.hi*1000LU);
    ms += double(t.lo/1000LU);//*0.001;
    
    
    return ms;
}
/*
 unsigned long endHighResolutionTimer(unsigned long time[2])
 {
 UnsignedWide t;
 Microseconds(&t);
 return t.lo - time[0];
 // given that we're returning a 32 bit integer, and this is unsigned subtraction...
 // it will just wrap around, we don't need the upper word of the time.
 // NOTE: the code assumes that more than 3 hrs will not go by between calls to startHighResolutionTimer() and endHighResolutionTimer().
 // I mean... that damn well better not happen anyway.
 }
 */
#else

// Initialize Our Timer (Get It Ready)
void TimerInit()
{
}

double startHighResolutionTimer()
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts); // Works on Linux
    
    
    double ms = double(ts.tv_sec*1000LU);
    ms += double(ts.tv_nsec/1000LU)*0.001;
    
    
    return ms;
}
#endif

#endif  // LIB_PROFILER_IMPLEMENTATION

#endif  // USE_PROFILER


#endif  // LIBPROFILER_H__
