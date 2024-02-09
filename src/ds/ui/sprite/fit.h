#pragma once
#ifndef DS_UI_SPRITE_FIT_H_
#define DS_UI_SPRITE_FIT_H_

namespace ds::ui {

//! A class to help calculate the transformation matrix to fit one rectangle into another.
class Fit {
  public:
	enum class Align {
		NONE,
		X_MIN_Y_MIN,
		X_MID_Y_MIN,
		X_MAX_Y_MIN,
		X_MIN_Y_MID,
		X_MID_Y_MID,
		X_MAX_Y_MID,
		X_MIN_Y_MAX,
		X_MID_Y_MAX,
		X_MAX_Y_MAX
	};
	enum class MeetOrSlice { NONE, MEET, SLICE };

	Fit() = default;
	Fit(Align align, MeetOrSlice meetOrSlice = MeetOrSlice::NONE)
	  : mAlign(align)
	  , mMeetOrSlice(meetOrSlice) {}

	explicit Fit(const std::string& css) {
		const char* sInOut = css.c_str();
		parse(&sInOut);
	}
	explicit Fit(const char** sInOut) { parse(sInOut); }

	//! Returns whether the fit is set to none. If it is, no transformations will be necessary.
	[[nodiscard]] bool isNone() const { return mAlign == Align::NONE; }

	//! Calculate the transformation matrix to fit the inner rectangle into the outer rectangle. Optionally normalizes
	//! the result to the range 0-1.
	[[nodiscard]] glm::mat3x2 calcTransform(const ci::Rectf& outer, const ci::Rectf& inner,
											bool normalized = false) const;
	//! Calculate the transformation matrix to fit the inner rectangle into the outer rectangle. Optionally normalizes
	//! the result to the range 0-1.
	[[nodiscard]] glm::mat3 calcTransform3x3(const ci::Rectf& outer, const ci::Rectf& inner,
											 bool normalized = false) const;
	//! Calculate the transformation matrix to fit the inner rectangle into the outer rectangle. Optionally normalizes
	//! the result to the range 0-1.
	[[nodiscard]] glm::mat4 calcTransform4x4(const ci::Rectf& outer, const ci::Rectf& inner,
											 bool normalized = false) const;

	[[nodiscard]] Align		  align() const { return mAlign; }
	[[nodiscard]] MeetOrSlice meetOrSlice() const { return mMeetOrSlice; }

  private:
	void parse(const char** sInOut);

	Align		mAlign{Align::X_MIN_Y_MIN};
	MeetOrSlice mMeetOrSlice{MeetOrSlice::NONE};
};

} // namespace ds::ui

#endif // DS_UI_SPRITE_FIT_H_