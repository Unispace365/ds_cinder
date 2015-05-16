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
	float			getLatency() const;

	/*!
	 * \name setTimerFrequency
	 * \brief How frequent server should update its clients' timers? in
	 * terms of frame numbers. (Unit: Hz)
	 */
	void			setTimerFrequency(int frames);

	/*!
	 * \name setDrawTime
	 * \brief DEBUG ONLY! draws a little string on top left of the app
	 * showing the current time.
	 */
	void			setDrawTime(bool on);

private:
	using inherited	= Sprite;

	float			mClientLatency	{ 0.0f };
    bool            mDebugDrawTime  { false };
    int             mSyncFrequency  { 5 }; // in frames
	UpdateParams	mSyncUpdateParams;

private:
	void			writeAttributesTo(DataBuffer&) override;
	void			readAttributeFrom(const char, DataBuffer&) override;
	void			updateServer(const UpdateParams&) override;
	void			drawLocalClient() override;
};

}} //!ds::ui

#endif //!DS_UI_TIMER_SPRITE_H_