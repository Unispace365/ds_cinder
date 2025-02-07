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
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/util/float_util.h>

namespace ds::ui {

template <typename T>
using SpriteFn = std::function<T(const Sprite*)>;
using SizeFn   = SpriteFn<float>;
using SpanFn   = SpriteFn<const Range<size_t>&>;

class Grid : public Sprite, public ILayout {
  public:
	struct Track;

	using AdditionalSpaceFn		  = std::function<float(const std::vector<Track>&, const Sprite*)>;
	using TrackGrowthConstraintFn = std::function<float(Track&)>;
	using TracksForGrowthFn		  = std::function<std::vector<Track*>(const std::vector<Track>&, const Sprite*)>;
	using TracksForGrowthBeyondConstraintFn = std::function<std::vector<Track*>(const std::vector<Track*>&)>;
	using BreadthFn							= std::function<float(const Track&)>;
	using AccumulatorFn						= std::function<float&(Track&)>;

	Grid(SpriteEngine& engine);

	void onChildAdded(Sprite&) override;
	void onChildRemoved(Sprite&) override;

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

	// Calculates the area occupied by the given \a column and \a row ranges. Assumes ranges are adjusted for gaps.
	ci::Rectf calcArea(const Range<size_t>& column, const Range<size_t>& row) const;
	// Calculates the area occupied by the given \a item.
	ci::Rectf calcArea(const Sprite* item, bool hasColumnGaps, bool hasRowGaps) const {
		const auto col = adjustForGaps(item->getColumnSpan(), hasColumnGaps);
		const auto row = adjustForGaps(item->getRowSpan(), hasRowGaps);
		return calcArea(col, row);
	}

	// Returns the width of the grid based on the column tracks, or 0 if the grid is not initialized.
	float calcWidth() const;
	// Returns the height of the grid based on the row tracks, or 0 if the grid is not initialized.
	float calcHeight() const;

	void drawLocalClient() override;

	void drawPostLocalClient() override;

	void addChild(Sprite& newChild) override;

	void updateLayout() const { mNeedsLayout = true; }

	void runLayout() override { performGridLayout(); }

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

	void fitInsideArea(const ci::Rectf& area) override;
	
	// Parses a span definition into a track range, e.g. "1" or "1 / span 3".
	static Range<size_t> parseSpan(const char** sInOut);
	// Returns the track range adjusted for gaps.
	static Range<size_t> adjustForGaps(const Range<size_t>& span, bool hasGaps);

  private:
	// Returns whether the \a area overlaps the \a span.
	bool areaOverlapsItem(const ci::Rectf& area, const Sprite* item) const;
	// Returns whether the \a area overlaps any of the \a spans.
	bool areaOverlapsItems(const ci::Rectf& area, const std::vector<Sprite*>& items) const;

	// Calculates the position of the grid line with the specified \a index.
	static float calcPos(size_t index, const std::vector<Track>& tracks, bool excludeFlex = false);
	// Calculates the position of the grid line with the specified \a index.
	static float calcPos(size_t index, const std::vector<Track*>& tracks, bool excludeFlex = false);

	// Parses the grid track definition.
	static std::vector<Track> parseTracks(const std::string& def);
	// Parses the grid track definition including gaps.
	static std::vector<Track> parseTracks(const std::string& def, const std::string& gap);

	// Calculates the grid lines based on the grid tracks.
	static void calculateGridLines(const std::vector<Track>& tracks, std::vector<float>& gridLines);

	// Performs the layout algorithm.
	void performGridLayout();
	//! This is the core grid track sizing algorithm. It is run for grid columns and grid rows.
	void		computeUsedBreadthOfGridTracks(css::Value::Direction direction, std::vector<Track>& tracks,
											   const SpanFn& spanFn, const SizeFn& minFn, const SizeFn& maxFn, bool hasGaps);
	static void resolveContentBasedTrackSizingFunctions(std::vector<Track>& tracks, const std::vector<Sprite*>& items,
														const SpanFn& spanFn, const SizeFn& minFn, const SizeFn& maxFn,
														bool hasGaps);
	static void
	resolveContentBasedTrackSizingFunctionsForItems(std::vector<Track>& tracks, // Set of tracks that need to be sized.
													const std::vector<Sprite*>&				 items,			 //
													const AdditionalSpaceFn&				 spaceFn,		 //
													const TrackGrowthConstraintFn&			 constraintFn,	 //
													const TracksForGrowthFn&				 tracksFn,		 //
													const TracksForGrowthBeyondConstraintFn& tracksBeyondFn, //
													const AccumulatorFn&					 accumulatorFn);
	static void	 distributeSpaceToTracks(float spaceToDistribute, const TrackGrowthConstraintFn& constraintFn,
										 std::vector<Track*> tracks, const std::vector<Track*>& tracksBeyond,
										 const BreadthFn& currentBreadthFn);
	static float calculateNormalizedFlexBreadth(const std::vector<Track*>& tracks, float spaceToFill);

	static float calculateRemainingSpace(const std::vector<Track>& tracks, float spaceToFill);

	// Returns a list of all items.
	std::vector<Sprite*> allItems();
	// Returns a list of all items that do not span a track with a flexible sizing function, sorted by span count.
	// The \a spanFn is either `getColumnSpan` or `getRowSpan'.
	std::vector<Sprite*> nonFlexibleItems(const std::vector<Track>& tracks, const SpanFn& spanFn, bool hasGaps);

	static void parse(std::vector<Track>& tracks, const std::string& def);

	EventClient			  mEventClient;
	std::string			  mColumnsDef;
	std::string			  mRowsDef;
	std::string			  mColumnGapDef;
	std::string			  mRowGapDef;
	std::vector<float>	  mHorizontalGridLines;
	std::vector<float>	  mVerticalGridLines;
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