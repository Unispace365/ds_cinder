#pragma once
#ifndef DS_UPDATE_PARAMS_H
#define DS_UPDATE_PARAMS_H

namespace ds
{

class UpdateParams
{
    public:
        UpdateParams();
        void  setDeltaTime(float dt);
        float getDeltaTime() const;
        void  setElapsedTime(float elapsed);
        float getElapsedTime() const;
    private:
        float mDeltaTime;
        float mElapsedTime;
};

}

#endif//DS_UPDATE_PARAMS_H
