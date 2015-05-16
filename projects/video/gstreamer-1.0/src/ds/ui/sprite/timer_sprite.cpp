#include "timer_sprite.h"

#include <chrono>

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine.h>
#include <ds/app/blob_registry.h>
#include <ds/data/data_buffer.h>
#include <ds/app/blob_reader.h>
#include <ds/cfg/settings.h>
#include <ds/app/app.h>

namespace
{
// Will be filled after registration
char						BLOB_TYPE	= 0;
// Marks internal changes that needs to be synced
const ds::ui::DirtyState&	TIME_DIRTY	= ds::ui::newUniqueDirtyState();
// Arbitrary ID's
const char					TIME_ATT	= 99;

struct TimePacket
{
	long long	mSendTime	{ 0L };
	float		mServerTime	{ 0.0f };

	void		write_to_ds_buffer(ds::DataBuffer& buf) const;
	void		read_from_ds_buffer(ds::DataBuffer& buf);

} _packet;

void TimePacket::write_to_ds_buffer(ds::DataBuffer& buf) const
{
	buf.add(mSendTime);
	buf.add(mServerTime);
}

void TimePacket::read_from_ds_buffer(ds::DataBuffer& buf)
{
	// retain the order of write_to_buffer.
	mSendTime = buf.read<long long>();
	mServerTime = buf.read<float>();
}

static inline long long now()
{ // PHP's now() impl
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

static struct Initialize
{
    Initialize() {
        ds::App::AddStartup([](ds::Engine& engine){
            ds::ui::TimerSprite::installSprite(engine);
        });
    }

} INIT;

void noop() {}

} //! anonymous namespace

namespace ds {
namespace ui {

/*!
 * \brief: Constructor, nothing special.
 */
TimerSprite::TimerSprite(ds::ui::SpriteEngine& eng)
	: inherited(eng)
    , mCallbackFn(noop)
{
	mBlobType = BLOB_TYPE;
	
	if (mEngine.getSettings("engine").getIntSize("timer:update_rate") > 0
		&& mEngine.getSettings("engine").getInt("timer:update_rate") > 0)
	{
		mSyncFrequency = mEngine.getSettings("engine").getInt("timer:update_rate");
	}
}

/*!
 * \brief: Installs and registers class' sprite blob
 * with engine during its initialization, as server.
 */
void TimerSprite::installAsServer(ds::BlobRegistry& registry)
{
	BLOB_TYPE = registry.add([](ds::BlobReader& r)
	{
		inherited::handleBlobFromClient(r);
	});
}

/*!
 * \brief: Installs and registers class' sprite blob
 * with engine during its initialization, as client.
 */
void TimerSprite::installAsClient(ds::BlobRegistry& registry)
{
	BLOB_TYPE = registry.add([](ds::BlobReader& r)
	{
		inherited::handleBlobFromServer<TimerSprite>(r);
	});
}

// !just a convenience to install the sprite
void TimerSprite::installSprite(Engine& engine)
{
    engine.installSprite(installAsServer, installAsClient);
}

/*!
 * \brief: Server update loop.
*/
void TimerSprite::updateServer(const ds::UpdateParams& p)
{
	inherited::updateServer(p);
	
	mSyncUpdateParams = p;

	if (ci::app::getElapsedFrames() % mSyncFrequency == 0)
	{
        markAsDirty(TIME_DIRTY);
        mCallbackFn();
    }
}

/*!
 * \brief: client draw loop.
*/
void TimerSprite::drawLocalClient()
{
	inherited::drawLocalClient();
	
#if _DEBUG
	if (mDebugDrawTime)
		ci::gl::drawString(std::to_string(mSyncUpdateParams.getElapsedTime()), ci::Vec2f::zero());
#endif
}

/*!
 * \brief: Server's packet send virtual.
*/
void TimerSprite::writeAttributesTo(ds::DataBuffer& buf)
{
	// TIME SEND IN SERVER MODE
	inherited::writeAttributesTo(buf);

	if (mDirty.has(TIME_DIRTY))
	{
		buf.add(TIME_ATT);
		_packet.mServerTime = mSyncUpdateParams.getElapsedTime();
		_packet.mSendTime = now();
		_packet.write_to_ds_buffer(buf);
	}
}

/*!
 * \brief: Client's packet receive virtual.
*/
void TimerSprite::readAttributeFrom(const char attributeId, ds::DataBuffer& buf)
{
	// TIME RECEIVE IN CLIENT MODE
	if (attributeId == TIME_ATT)
	{
		_packet.read_from_ds_buffer(buf);
		
		/*
		 * ------------- t = t0
		 * client    server
		 *    |        |
		 *    | frame1 |
		 * ---+--------+--- server sets dirty
		 *    |        |
		 *    | frame2 |
		 * ---+--------+--- server sends time
		 *    |        |
		 *    | frame3 |
		 * ---+--------+--- client receives / stores time
		 *    |        |
		 *    | frame4 |
		 * ---+--------+--- time is available to client
		 *
		 * You can see in the diagram above, the current time diff
		 * calculated in readAttributeFrom on client side is roughly
		 * twice the actual frame latency. Therefore it needs to be
		 * divided by two.
		 */
		
		mClientLatency = static_cast<float>(now() - _packet.mSendTime) * 0.0005f; //0.0005f is 0.001 / 2
		auto _oldServerTime = mSyncUpdateParams.getElapsedTime();
		mSyncUpdateParams.setElapsedTime(_packet.mServerTime + mClientLatency);
		mSyncUpdateParams.setDeltaTime(mSyncUpdateParams.getElapsedTime() - _oldServerTime);
	}
	else
	{
		inherited::readAttributeFrom(attributeId, buf);
	}
}

float TimerSprite::getLatency() const
{
	return mClientLatency;
}

const UpdateParams& TimerSprite::getUpdateParams()
{
	return mSyncUpdateParams;
}

void TimerSprite::setDrawTime(bool on)
{
	if (on == mDebugDrawTime) return;

#if _DEBUG

	if (on)
	{
		setTransparent(false);
		setSize(150.0f, 50.0f);
		setColor(ci::Color::black());
		setUseShaderTextuer(true);
	}

#endif

	mDebugDrawTime = on;
}

void TimerSprite::setTimerFrequency(int freq)
{
	if (freq == mSyncFrequency || freq <= 0) return;
	mSyncFrequency = freq;
}

void TimerSprite::setTimerCallback(const std::function<void()>& fn)
{
    mCallbackFn = fn;
}

int TimerSprite::getTimerFrequency() const
{
    return mSyncFrequency;
}

}
} //!ds::ui