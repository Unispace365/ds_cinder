#include "update_params.h"

namespace ds
{

UpdateParams::UpdateParams()
    : mDeltaTime(0.0f)
    , mElapsedTime(0.0f)
{

}

void UpdateParams::setDeltaTime( float dt )
{
    mDeltaTime = dt;
}

float UpdateParams::getDeltaTime() const
{
    return mDeltaTime;
}

void UpdateParams::setElapsedTime( float elapsed )
{
    mElapsedTime = elapsed;
}

float UpdateParams::getElapsedTime() const
{
    return mElapsedTime;
}

}
