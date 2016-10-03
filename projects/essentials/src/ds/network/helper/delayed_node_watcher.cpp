#include "delayed_node_watcher.h"

#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>

namespace ds {

    /**
    * \class ds::DelayedNodeWatcher
    */
    DelayedNodeWatcher::DelayedNodeWatcher(ds::ui::SpriteEngine& eng, const std::string& host, const int port)
        : ds::AutoUpdate(eng)
        , mNodeWatcher(eng)
        , mNeedQuery(false)
        , mDelayWaitTime(1.0f)
        , mUniquifyDelayedMessages(false)
    {

        mLastQueryTime = Poco::Timestamp().epochMicroseconds();

        mNodeWatcher.add([this](const ds::NodeWatcher::Message& msg){
            mNeedQuery = true;

            if (mRegularNodeCallback){
                mRegularNodeCallback(msg);
            }
            else {
                mDelayedMessages.push_back(msg);
            }

            mLastQueryTime = Poco::Timestamp().epochMicroseconds();
        });
    }

    void DelayedNodeWatcher::addRegularNodeCallback(const std::function<void(const NodeWatcher::Message&)>& callback){
        mRegularNodeCallback = callback;
    }

    void DelayedNodeWatcher::setDelayedNodeCallback(const std::function<void()>& callback){
        mDelayedNodeCallback = callback;
    }

    void DelayedNodeWatcher::setDelayedMessageNodeCallback(const std::function<void(const NodeWatcher::Message&)>& callback){
        mDelayedMessageNodeCallback = callback;
    }

    void  DelayedNodeWatcher::setUniquifyDelayedMessage(bool uniquify){
        mUniquifyDelayedMessages = uniquify;
    }

    bool DelayedNodeWatcher::sort(NodeWatcher::Message& a, NodeWatcher::Message& b){
        //std::wstring upperA = a.();
        //std::transform(upperA.begin(), upperA.end(), upperA.begin(), ::toupper);
        //std::wstring upperB = b.getTitle();
        //std::transform(upperB.begin(), upperB.end(), upperB.begin(), ::toupper);

        //return upperA < upperB;
        return true;
    }

    bool DelayedNodeWatcher::uniquify(NodeWatcher::Message& a, NodeWatcher::Message& b){
        //if (a.getTitle() == b.getTitle()){
        //    return true;
        //}
        //else {
        //    return false;
        //}
        return true;

    }



    void DelayedNodeWatcher::update(const ds::UpdateParams & p){
        Poco::Timestamp::TimeVal nowwy = Poco::Timestamp().epochMicroseconds();
        float delty = (float)(nowwy - mLastQueryTime) / 1000000.0f;
        if (mNeedQuery && delty > mDelayWaitTime){
            mNeedQuery = false;
            mLastQueryTime = nowwy;

            if (mDelayedMessageNodeCallback){

                //lump all messages together, and uniquify.   
                //Note: this will probably affect the order, since the 
                //messages need to be sorted for the uniquify process to work.
                //If you need to preserve order of messages, use at your own risk..
                if (mUniquifyDelayedMessages){
                    NodeWatcher::Message uniqueMessages;
                    for (auto it = mDelayedMessages.begin(); it != mDelayedMessages.end(); ++it){
                        for (auto msg : it->mData) {
                            uniqueMessages.mData.push_back(msg);
                        }
                    }
                    std::sort(uniqueMessages.mData.begin(), uniqueMessages.mData.end());
                    auto uniqueified = std::unique(uniqueMessages.mData.begin(), uniqueMessages.mData.end());
                    uniqueMessages.mData.erase(uniqueified, uniqueMessages.mData.end());

                    mDelayedMessageNodeCallback(uniqueMessages);
                }
                else {
                    // send all the stored messages
                    for (auto it = mDelayedMessages.begin(); it != mDelayedMessages.end(); it++){
                        mDelayedMessageNodeCallback(*it);
                    }
                }
            }
            mDelayedMessages.clear();

            if (mDelayedNodeCallback){
                mDelayedNodeCallback();
            }
        }
    }

} // !namespace ds
