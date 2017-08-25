#include "Threading.h"

#if defined ( FO_WINDOWS ) || defined ( FO_LINUX ) || defined ( FO_MAC ) || defined ( FO_ANDROID )
void Thread_Sleep( uint ms )
{
    # if defined ( FO_WINDOWS )
    Sleep( ms );
    # else
    struct timespec req;
    req.tv_sec = ms / 1000;
    req.tv_nsec = ( ms % 1000 ) * 1000000;
    while( nanosleep( &req, &req ) == -1 && errno == EINTR )
        continue;
    # endif
}
#endif

#ifndef NO_THREADING

# ifndef FO_WINDOWS
// Mutex static stuff
bool                Mutex::attrInitialized = false;
pthread_mutexattr_t Mutex::mutexAttr;
# endif

static void* ThreadBeginExecution( Thread::ThreadFunc func, char* name, void* arg )
{
    Thread::SetCurrentName( name );
    delete[] name;
    func( arg );
    return nullptr;
}

Thread::~Thread()
{
    if( thread.joinable() )
        thread.detach();
}

void Thread::Start( ThreadFunc func, const string& name, void* arg /* = nullptr */ )
{
    std::thread t( ThreadBeginExecution, func, Str::Duplicate( name ), arg );
    thread.swap( t );
}

void Thread::Wait()
{
    if( thread.joinable() )
        thread.join();
}

THREAD char Thread::threadName[ 64 ] = { 0 };
SizeTStrMap Thread::threadNames;
Mutex       Thread::threadNamesLocker;

size_t Thread::GetCurrentId()
{
    # ifdef FO_WINDOWS
    return (size_t) GetCurrentThreadId();
    # else
    return (size_t) pthread_self();
    # endif
}

void Thread::SetCurrentName( const char* name )
{
    if( threadName[ 0 ] )
        return;

    Str::Copy( threadName, name );
    SCOPE_LOCK( threadNamesLocker );
    threadNames.insert( std::make_pair( GetCurrentId(), threadName ) );
}

const char* Thread::GetCurrentName()
{
    return threadName;
}

const char* Thread::FindName( size_t thread_id )
{
    SCOPE_LOCK( threadNamesLocker );
    auto it = threadNames.find( thread_id );
    return it != threadNames.end() ? it->second.c_str() : nullptr;
}

#endif
