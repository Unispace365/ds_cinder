#ifndef DS_UI_TIMER_SPRITE_H_
#define DS_UI_TIMER_SPRITE_H_

#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {

// forward declaration
class SpriteEngine;

/*!
 * \class TimerSprite
 * \namespace ds::ui
 * \brief A leaf Sprite that can act as a synchronized timer among
 * multiple instances of the engine.
 */
class TimerSprite final : public Sprite
{
public:
	TimerSprite(SpriteEngine&);

	static void		installAsServer(BlobRegistry&);
	static void		installAsClient(BlobRegistry&);
	static void		installSprite(Engine&);

public:
	/*!
	 * \name getUpdateParams
	 * \brief in server mode, this returns the updateParams of the last
	 * frame of the engine. in client mode this represents server's -
	 * concurrent updateParams, latency calculated inside it.
	 * \see readAttributeFrom impl.
	 */
	const UpdateParams&
					getUpdateParams();
	/*!
	 * \name getLatency
	 * \brief returns the approximate latency between client and server.
	 * \note returns 0 in server mode. (Unit: seconds)
	 */
	double			getLatency() const;

	/*!
	 * \name TimerFrequency
	 * \brief How frequent server should update its clients' timers? in
	 * terms of frame numbers. (Unit: Hz)
	 */
	void			setTimerFrequency(int frames);
    int             getTimerFrequency() const;

    /*!
     * \name setTimerCallback
     * \brief a callback to be called every time server sends out packet
     */
    void            setTimerCallback(const std::function<void()>&);

	/*!
	 * \name setDrawTime
	 * \brief DEBUG ONLY! draws a little string on top left of the app
	 * showing the current time.
	 */
	void			setDrawTime(bool on);

    /*!
     * \name now
     * \brief similar to PHP's now() function. returns time since epoch.
     */
    double          now() const;

    /*!
	 * \name getServerSendTime
	 * \brief The time that server sent out the time packet.
	 */
    double          getServerSendTime() const;

private:
    double			mClientLatency;
    double          mSendTime;
    bool            mDebugDrawTime;
    int             mSyncFrequency;
	UpdateParams	mSyncUpdateParams;
    std::function < void() >
                    mCallbackFn;

private:
	void			writeAttributesTo(DataBuffer&) override;
	void			readAttributeFrom(const char, DataBuffer&) override;
	void			updateServer(const UpdateParams&) override;
	void			drawLocalClient() override;
};

}} //!ds::ui

#endif //!DS_UI_TIMER_SPRITE_H_