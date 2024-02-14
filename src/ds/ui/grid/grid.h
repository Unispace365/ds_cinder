#pragma once

/*
 * Implementation of the CSS Grid Layout algorithm as described in:
 * https://www.w3.org/TR/2013/WD-css3-grid-layout-20130402/#layout-algorithm
 *
 * Author: Paul Houx, Downstream Amsterdam ( paul.houx@downstream.com )
 *
 * There are several amendments to the used specification, which we will gradually implement to
 * make the code compatible with the latest browsers.
 */

#include <ds/ui/grid/css.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/util/float_util.h>

namespace ds::ui {

template <typename T>
using SpriteFn = std::function<T(const Sprite*)>;
using SizeFn   = SpriteFn<float>;
using SpanFn   = SpriteFn<const Range<size_t>&>;

class Grid : public Sprite {
  public:
	struct Track;

	using AdditionalSpaceFn		  = std::function<float(const std::vector<Track>&, const Sprite*)>;
	using TrackGrowthConstraintFn = std::function<float(Track&)>;
	using TracksForGrowthFn		  = std::function<std::vector<Track*>(const std::vector<Track>&, const Sprite*)>;
	using TracksForGrowthBeyondConstraintFn = std::function<std::vector<Track*>(const std::vector<Track*>&)>;
	using BreadthFn							= std::function<float(const Track&)>;
	using AccumulatorFn						= std::function<float&(Track&)>;

	Grid(SpriteEngine& engine);

	// Accepts CSS-style definition, e.g. "100px 1fr 20%".
	void setColumns(const std::string& def);
	// Accepts CSS-style definition, e.g. "100px 1fr 20%".
	void setRows(const std::string& def);

	// Accepts CSS-style definition, e.g. "10px".
	void setColumnGap(const std::string& def);
	// Accepts CSS-style definition, e.g. "10px".
	void setRowGap(const std::string& def);
	// Accepts CSS-style definition, e.g. "10px".
	void setGap(const std::string& def);

	//
	ci::Rectf calcArea(const Range<size_t>& column, const Range<size_t>& row) const;
	//
	ci::Rectf calcArea(const Sprite* item) const {
		const auto& col = item->getColumnSpan();
		const auto& row = item->getRowSpan();
		return calcArea(col, row);
	}

	//
	float calcWidth(bool excludeFlex = false) const;
	//
	float calcHeight(bool excludeFlex = false) const;
	//
	float calcColumnPos(size_t index, bool excludeFlex = false) const {
		return calcPos(index, mColumns, mColumnGap.asUser(this, css::Value::HORIZONTAL), excludeFlex);
	}
	//
	float calcRowPos(size_t index, bool excludeFlex = false) const {
		return calcPos(index, mRows, mRowGap.asUser(this, css::Value::VERTICAL), excludeFlex);
	}

	static Range<size_t> parseSpan(const char** sInOut);

	void drawLocalClient() override;

	void drawPostLocalClient() override;

	void addChild(Sprite& newChild) override;

	void setLayoutUpdatedFunction(const std::function<void()>& layoutUpdatedFunction) {
		mLayoutUpdatedFunction = layoutUpdatedFunction;
	}
	void onLayoutUpdate() const {
		if (mLayoutUpdatedFunction) {
			mLayoutUpdatedFunction();
		}
	}

	void setSizeAll(float width, float height, float depth) override {
		mNeedsLayout |= !mChildren.empty();
		Sprite::setSizeAll(width, height, depth);
	}

	bool setAvailableSize(const ci::vec2& size) override;

  private:
	// Returns whether the \a area overlaps the \a span.
	bool areaOverlapsItem(const ci::Rectf& area, const Sprite* item) const;
	// Returns whether the \a area overlaps any of the \a spans.
	bool areaOverlapsItems(const ci::Rectf& area, const std::vector<Sprite*>& items) const;
	// Calculates the position of the grid line with the specified \a index.
	static float calcPos(size_t index, const std::vector<Track>& tracks, float gap, bool excludeFlex = false);
	// Calculates the position of the grid line with the specified \a index.
	static float calcPos(size_t index, const std::vector<Track*>& tracks, float gap, bool excludeFlex = false);
	// Returns the number of gaps.
	static int countGaps(size_t index, const std::vector<Track>& tracks);
	// Returns the number of gaps.
	static int countGaps(size_t index, const std::vector<Track*>& tracks);
	// Performs the layout algorithm.
	void runLayout();
	//! This is the core grid track sizing algorithm. It is run for grid columns and grid rows.
	void		computeUsedBreadthOfGridTracks(css::Value::Direction direction, std::vector<Track>& tracks,
											   const SpanFn& spanFn, const SizeFn& minFn, const SizeFn& maxFn);
	static void resolveContentBasedTrackSizingFunctions(std::vector<Track>& tracks, const std::vector<Sprite*>& items,
														const SpanFn& spanFn, const SizeFn& minFn, const SizeFn& maxFn);
	static void
	resolveContentBasedTrackSizingFunctionsForItems(std::vector<Track>& tracks, // Set of tracks that need to be sized.
													const std::vector<Sprite*>&				 items,			 //
													const AdditionalSpaceFn&				 spaceFn,		 //
													const TrackGrowthConstraintFn&			 constraintFn,	 //
													const TracksForGrowthFn&				 tracksFn,		 //
													const TracksForGrowthBeyondConstraintFn& tracksBeyondFn, //
													const AccumulatorFn&					 accumulatorFn);
	static void	 distributeSpaceToTracks(std::vector<Track>& tracks, float spaceToDistribute,
										 const TrackGrowthConstraintFn& constraintFn, std::vector<Track*> tracksFn,
										 const std::vector<Track*>& tracksBeyond, const BreadthFn& currentBreadthFn);
	static float calculateNormalizedFlexBreadth(const std::vector<Track*>& tracks, float spaceToFill, float gap);

	static float calculateRemainingSpace(const std::vector<Track>& tracks, float spaceToFill, float gap);

	// Returns a list of all items.
	std::vector<Sprite*> allItems();
	// Returns a list of all items that do not span a track with a flexible sizing function, sorted by span count.
	// The \a spanFn is either `getColumnSpan` or `getRowSpan'.
	std::vector<Sprite*> nonFlexibleItems(const std::vector<Track>& tracks, const SpanFn& spanFn);

	static void parse(std::vector<Track>& tracks, const std::string& def);

	std::vector<Track>	  mColumns;
	std::vector<Track>	  mRows;
	css::Value			  mColumnGap{0, css::Value::PIXELS};
	css::Value			  mRowGap{0, css::Value::PIXELS};
	std::function<void()> mLayoutUpdatedFunction;
	mutable bool		  mInitialized{false};
	mutable bool		  mNeedsLayout{true};
};

class SizingFn {
  public:
	enum Unit { UNDEFINED, FIXED, MIN_CONTENT, MAX_CONTENT };

	SizingFn() = default;

	explicit SizingFn(const std::string& str);
	explicit SizingFn(const char** sInOut);

	const css::Value& value() const { return mValue; }
	Unit			  unit() const { return mUnit; }

	bool isFixed() const { return mUnit == FIXED && mValue.isFixed(); }
	bool isIntrinsic() const { return !isFixed(); }
	bool isMinContent() const { return mUnit == MIN_CONTENT; }
	bool isMaxContent() const { return mUnit == MAX_CONTENT; }
	bool isFlex() const { return mUnit == FIXED && mValue.isFlex(); }

	//! Returns whether the sizing function is defined.
	operator bool() const { return mUnit != UNDEFINED; }

  private:
	void parse(const char** sInOut);

	Unit	   mUnit{UNDEFINED};
	css::Value mValue;
};

struct Grid::Track {
	enum Type { BREADTH, MIN_MAX, FIT_CONTENT };

	Track() = default;
	explicit Track(const std::string& str);
	explicit Track(const char** sInOut);

	/// Returns the flex factor (the value in front of 'fr'), or 0 if track is not flexible.
	float flexValue() const { return isFlex() ? max.value().value() : 0; }

	//! Returns whether this track uses the minmax sizing function.
	bool isMinMax() const { return type == MIN_MAX; }
	//! Returns whether this track is set to fit content.
	bool isFitContent() const { return type == FIT_CONTENT; }
	//! Returns whether this track is flexible.
	bool isFlex() const { return min.isFlex(); }

	//! Returns whether this track is not flexible but can still grow larger.
	bool canGrow() const { return !isFlex() && !approxEqual(usedBreadth, maxBreadth); }

	void parse(const char** sInOut);

	// See: https://www.w3.org/TR/css-grid-1/#algo-init
	void initialize(const css::Value::Dimensions& dimensions);

	Type	 type{BREADTH}; //
	SizingFn min;			//
	SizingFn max;			//

	float				 usedBreadth{0};									 // Used by the track sizing algorithm.
	float				 maxBreadth{std::numeric_limits<float>::infinity()}; // Used by the track sizing algorithm.
	float				 updatedTrackBreadth{0};							 // Used by the track sizing algorithm.
	float				 updatedLimit{0};									 // Used by the track sizing algorithm.
	float				 tempBreadth{0};									 // Used by the track sizing algorithm.
	float				 normalizedFlexValue{0};							 // Used by the track sizing algorithm.
	std::vector<Sprite*> spanGroupInWhichMaxBreadthWasMadeFinite;			 // Used by the track sizing algorithm.
};

} // namespace ds::ui