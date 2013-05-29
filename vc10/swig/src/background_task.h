
#include <ds/thread/serial_runnable.h>
#include "background_task_runnable.h"

namespace ds {
//class BackgroundTaskPimp;

namespace ui {
class SpriteEngine;
}


class BackgroundTask {
    public:
        BackgroundTask( ds::ui::SpriteEngine& );
        virtual ~BackgroundTask();
        virtual PyObject *onRun();
        virtual void onComplete( PyObject *results );
        void start( bool synchronous=false );

    private:
        ds::SerialRunnable< BackgroundTaskRunnable > mSerialRunnable;
        BackgroundTaskRunnable *mBackgroundTaskRunnable;
        //BackgroundTaskPimp *mPimp;
};


} // namespace ds

