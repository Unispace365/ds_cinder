#include <Python.h>

#include "background_task.h"
#include <ds/thread/serial_runnable.h>
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/util/memory_ds.h"

namespace ds {

typedef std::function< PyObject* (void) > PythonRunFunc;
typedef std::function< void (PyObject*) > PythonCompleteFunc;


BackgroundTaskRunnable::BackgroundTaskRunnable()
    : mParent( NULL )
    , mResults( NULL )
{}
      
BackgroundTaskRunnable::BackgroundTaskRunnable( BackgroundTask *parent )
    : mParent(parent)
    , mResults( NULL )
{}

BackgroundTaskRunnable::~BackgroundTaskRunnable()
{
    Py_XDECREF( mResults );
}

void BackgroundTaskRunnable::run()
{
    // Declare this thread and acquire Python Global Interpreter Locky
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    // Do Python stuff here...
    
    // Free PyObject that was previously being referenced...
    Py_XDECREF( mResults );

    // I own this reference... I'm responsible for deleting it...
    mResults = mParent->onRun();


    // Release the thread. No Python API allowed beyond this point.
    PyGILState_Release(gstate);

    //this->setReplyHandler( [this](BackgroundTaskRunnable&) { this->onComplete(); } );
}


/*
class BackgroundTaskPimp {
public:
    BackgroundTaskPimp( ds::ui::SpriteEngine &engine, BackgroundTask *parent ) 
        : mSerialRunnable( engine )
        , mParent( parent )
    {
        mRunnable->setPythonRunFunc( [mParent]() -> PyObject * { return mParent->run(); } );
        mRunnable->setPythonCompleteFunc( [mParent](PyObject * results) { mParent->onComplete( results ); } );
    }

    static BackgroundTaskRunnable *alloc_runnable() {
        BackgroundTaskRunnable *ret = new BackgroundTaskRunnable();
        ret ->
    }

    ds::SerialRunnable< BackgroundTaskRunnable > mSerialRunnable;
    BackgroundTaskRunnable mRunnable;
    BackgroundTask *mParent;
    //ds::SerialRunnable<BackgroundTaskRunnable> mRunnable;
 
};
*/

//class BackgroundTaskRunnable : public Poco::Runnable, ds::SerialRunnable<BackgroundTaskRunnable> {
/*
class BackgroundTaskRunnable : public Poco::Runnable {
    public:
        BackgroundTaskRunnable();
        void setPythonRunFunc( const PythonRunFunc &func );
        void setPythonCompleteFunc( const PythonCompleteFunc &func );

    private:
        virtual void run();
        void onComplete();

        PythonRunFunc mPythonRunFunc;
        PythonCompleteFunc mPythonCompleteFunc;
        PyObject *mResults;
};
*/




BackgroundTask::BackgroundTask( ds::ui::SpriteEngine& engine )
    //: mRunnable( new BackgroundTaskRunnable( engine ) )
    //: mPimp( new BackgroundTaskPimp( engine )
    : mSerialRunnable( engine, [this](void) -> BackgroundTaskRunnable * { this->mBackgroundTaskRunnable = new BackgroundTaskRunnable( this ); return this->mBackgroundTaskRunnable; }  )
{
    mSerialRunnable.setReplyHandler( [this](BackgroundTaskRunnable) { this->onComplete( this->mBackgroundTaskRunnable->mResults ); } );
}

BackgroundTask::~BackgroundTask()
{
}


PyObject *
BackgroundTask::onRun()
{
    return NULL;
}

void 
BackgroundTask::onComplete( PyObject *results )
{
}

void 
BackgroundTask::start( bool synchronous )
{
    mSerialRunnable.start( nullptr, synchronous );
}


// ---------------------------------------------
/*
BackgroundTaskRunnable::BackgroundTaskRunnable( ds::ui::SpriteEngine& engine )
    ,m
    : mResults( NULL )
{
    // Declare this thread and acquire Python Global Interpreter Locky
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    // Do Python stuff here...
    mResults = Py_None;

    // Release the thread. No Python API allowed beyond this point.
    PyGILState_Release(gstate);

    this->setReplyHandler( [this](BackgroundTaskRunnable&) { this->onComplete(); } );
}

void BackgroundTaskRunnable::setPythonRunFunc( const PythonRunFunc &func )
{
    mPythonRunFunc = func;
}

void BackgroundTaskRunnable::setPythonCompleteFunc( const PythonCompleteFunc &func )
{
    mPythonCompleteFunc = func;
}


void BackgroundTaskRunnable::run()
{
    // Declare this thread and acquire Python Global Interpreter Locky
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    // Do Python stuff here...
    if ( mPythonRunFunc ) {
        mResults = mPythonRunFunc();
    }

    // Release the thread. No Python API allowed beyond this point.
    PyGILState_Release(gstate);
}

void BackgroundTaskRunnable::onComplete()
{
    if ( mPythonCompleteFunc ) {
        mPythonCompleteFunc( mResults );
    }
}

*/

} // namespace ds


