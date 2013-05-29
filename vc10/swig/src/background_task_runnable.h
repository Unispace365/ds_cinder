#include <Poco/Runnable.h>

namespace ds {
class BackgroundTask;

class BackgroundTaskRunnable : public Poco::Runnable {
    public:
        BackgroundTaskRunnable();
        BackgroundTaskRunnable( BackgroundTask *parent );
        ~BackgroundTaskRunnable();
		PyObject *mResults;

        //void setPythonRunFunc( const PythonRunFunc &func );
        //void setPythonCompleteFunc( const PythonCompleteFunc &func );

        virtual void run();

    private:
		BackgroundTask *mParent;
        //void onComplete();

        //PythonRunFunc mPythonRunFunc;
        //PythonCompleteFunc mPythonCompleteFunc;
       
};

} // namespace ds
