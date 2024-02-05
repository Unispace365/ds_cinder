#include "stdafx.h"

#include <ds/ui/grid/grid.h>

using namespace ds::css;

namespace {

using namespace ds::ui;

// GridItem functions.

float getWidthMin(const Sprite* item) {
	return item->getWidthMin();
}

float getWidthMax(const Sprite* item) {
	return item->getWidthMax();
}

float getHeightMin(const Sprite* item) {
	return item->getHeightMin();
}

float getHeightMax(const Sprite* item) {
	return item->getHeightMax();
}

const Range<size_t>& getColumnSpan(const Sprite* item) {
	return item->getColumnSpan();
}

const Range<size_t>& getRowSpan(const Sprite* item) {
	return item->getRowSpan();
}

// Accumulator functions.

float& getTrackUsedBreadthRef(Grid::Track& track) {
	return track.usedBreadth;
}

float& getTrackMaxBreadthRef(Grid::Track& track) {
	return track.maxBreadth;
}

// Breadth functions.

float getTrackBase(const Grid::Track& track) {
	return track.usedBreadth;
}

float getTrackMaxBreadth(const Grid::Track& track) {
	return track.maxBreadth;
}

SizingFn getSizingMin(const Grid::Track& track) {
	return track.min;
}

SizingFn getSizingMax(const Grid::Track& track) {
	return track.max;
}

// TracksForGrowthBeyondConstraint functions.

// Identity function.
std::vector<Grid::Track*> getTracks(const std::vector<Grid::Track*>& tracks) {
	return tracks;
}

std::vector<Grid::Track*> getTracksMinIsMax(const std::vector<Grid::Track*>& tracks) {
	std::vector<Grid::Track*> result;

	for (const auto& track : tracks) {
		if (track->min.isMaxContent()) result.push_back(track);
	}

	return result;
}

std::vector<Grid::Track*> getTracksMinIsMinOrMax(const std::vector<Grid::Track*>& tracks) {
	std::vector<Grid::Track*> result;

	for (const auto& track : tracks) {
		if (track->min.isMinContent() || track->min.isMaxContent()) result.push_back(track);
	}

	return result;
}

std::vector<Grid::Track*> getTracksMaxIsMinOrMax(const std::vector<Grid::Track*>& tracks) {
	std::vector<Grid::Track*> result;

	for (const auto& track : tracks) {
		if (track->max.isMinContent() || track->max.isMaxContent()) result.push_back(track);
	}

	return result.empty() ? tracks : result;
}

std::vector<Grid::Track*> getTracksMaxIsMax(const std::vector<Grid::Track*>& tracks) {
	std::vector<Grid::Track*> result;

	for (const auto& track : tracks) {
		if (track->max.isMaxContent()) result.push_back(track);
	}

	return result.empty() ? tracks : result;
}

// Subset functions.

// Converts a set of tracks into a set of track pointers for further processing.
std::vector<Grid::Track*> getAllTracks(const std::vector<Grid::Track>& tracks) {
	std::vector<Grid::Track*> result;

	for (const auto& track : tracks)
		result.push_back(const_cast<Grid::Track*>(&track));

	return result;
}

// Given a set of \a tracks, it returns a set of pointers to all tracks covered by \a item.
std::vector<Grid::Track*> getSpannedTracks(const std::vector<Grid::Track>& tracks, const Sprite* item,
										   const SpanFn& spanFn) {
	std::vector<Grid::Track*> result;

	const auto& span = spanFn(item);
	for (size_t i = span.min; i < span.max; ++i) {
		if (i < tracks.size()) result.push_back(const_cast<Grid::Track*>(tracks.data() + i));
	}

	return result;
}

// Returns the set of grid tracks whose MaxTrackSizingFunction is a flexible length.
std::vector<Grid::Track*> getFlexTracks(const std::vector<Grid::Track*>& tracks) {
	std::vector<Grid::Track*> result;

	for (auto track : tracks)
		if (track->max.isFlex()) result.push_back(track);

	return result;
}

// Returns the set of grid items whose span count equals \a spanCount.
std::vector<Sprite*> getItemsWithSpanCount(const std::vector<Sprite*>& items, const SpanFn& spanFn, size_t spanCount) {
	std::vector<Sprite*> result;

	for (auto item : items) {
		const auto& span = spanFn(item);
		if (span.count() == spanCount) result.push_back(item);
	}

	return result;
}

// AdditionalSpace functions.

float calcAdditionSpaceBase(const std::vector<Grid::Track>& tracks, const Sprite* item, const SizeFn& sizeFn,
							const SpanFn& spanFn) {
	float result = sizeFn(item);

	for (const auto track : getSpannedTracks(tracks, item, spanFn)) {
		result -= track->usedBreadth;
	}

	return glm::max(0.f, result);
}

float calcAdditionSpaceLimit(const std::vector<Grid::Track>& tracks, const Sprite* item, const SizeFn& sizeFn,
							 const SpanFn& spanFn) {
	float result = sizeFn(item);

	for (const auto track : getSpannedTracks(tracks, item, spanFn)) {
		if (std::isfinite(track->maxBreadth))
			result -= track->maxBreadth;
		else
			result -= track->usedBreadth;
	}

	return glm::max(0.f, result);
}

} // namespace

namespace ds::ui {

Grid::Grid(SpriteEngine& engine)
  : Sprite(engine) {
	setTransparent(false); // For debugging.
}

void Grid::setColumns(const std::string& def) {
	mColumns.clear();
	try {
		parse(mColumns, def);
	} catch (const std::exception& exc) {
		DS_LOG_ERROR(exc.what() << " in " << def)
	}
}

void Grid::setRows(const std::string& def) {
	mRows.clear();
	try {
		parse(mRows, def);
	} catch (const std::exception& exc) {
		DS_LOG_ERROR(exc.what() << " in " << def)
	}
}

void Grid::setColumnGap(const std::string& def) {
	mColumnGap = Value(def);
}

void Grid::setRowGap(const std::string& def) {
	mRowGap = Value(def);
}

void Grid::setGap(const std::string& def) {
	mColumnGap = mRowGap = Value(def);
}

ci::Rectf Grid::calcArea(const Range<size_t>& column, const Range<size_t>& row) const {
	const auto x1 = calcColumnPos(column.min);
	const auto y1 = calcRowPos(row.min);
	auto	   x2 = calcColumnPos(column.max);
	auto	   y2 = calcRowPos(row.max);
	if (glm::max(0.0f, x2 - x1) > 0) x2 -= mColumnGap.asUser(getWidth());
	if (glm::max(0.0f, y2 - y1) > 0) y2 -= mRowGap.asUser(getWidth());
	return {x1, y1, x2, y2};
}

float Grid::calcWidth() const {
	return calcColumnPos(mColumns.size());
}

float Grid::calcHeight() const {
	return calcRowPos(mRows.size());
}

void Grid::drawLocalClient() {
	if (mNeedsLayout) runLayout();

	const auto engine = dynamic_cast<Engine*>(&mEngine);
	if (engine && engine->isShowingSettingsEditor()) {
		ci::gl::ScopedColor		   sc(1, 1, 1);
		ci::gl::ScopedBlendPremult sb;

		ci::gl::color(ci::ColorA8u(255, 204, 0, 255));
		for (size_t row = 0; row < mRows.size(); ++row) {
			for (size_t col = 0; col < mColumns.size(); ++col) {
				ci::gl::drawStrokedRect(calcArea({col, col + 1}, {row, row + 1}));
			}
		}

		float width	 = calcWidth();
		float height = calcHeight();

		ci::gl::color(ci::ColorA8u(255, 204, 0, 255));
		ci::gl::drawStrokedRect({0, 0, width, height}, 15);

		ci::gl::color(ci::ColorA8u(128, 51, 0, 128));
		for (const auto item : allItems()) {
			ci::gl::drawStrokedRect(calcArea(item), 15);
		}
	}
}

void Grid::addChild(Sprite& newChild) {
	mNeedsLayout = true;
	Sprite::addChild(newChild);
}

bool Grid::areaOverlapsItem(const ci::Rectf& area, const Sprite* item) const {
	const auto& col		 = item->getColumnSpan();
	const auto& row		 = item->getRowSpan();
	const auto	itemArea = calcArea(col, row);
	return itemArea.intersects(area);
}

bool Grid::areaOverlapsItems(const ci::Rectf& area, const std::vector<Sprite*>& items) const {
	for (const auto item : items) {
		if (areaOverlapsItem(area, item)) return true;
	}
	return false;
}

void Grid::runLayout() {
	if (!mNeedsLayout) return;

	ci::Timer t{true};

	// Initialize item spans, taken from the sprites. These are then updated during layout.
	// TODO sort items by specified order, see: https://drafts.csswg.org/css-flexbox-1/#order-modified-document-order
	auto items = allItems();
	for (auto item : items) {
		if (item->isColumnSpanAuto()) item->setColumnSpan({0, 0});
		if (item->isRowSpanAuto()) item->setRowSpan({0, 0});
	}

	try {
		// 1. Call ComputedUsedBreadthOfGridTracks for grid columns to resolve their logical width.
		computeUsedBreadthOfGridTracks(mColumns, getWidth(), mColumnGap.asUser(getWidth()), ::getColumnSpan,
									   ::getWidthMin, ::getWidthMax);

		// 2. Call ComputedUsedBreadthOfGridTracks for grid rows to resolve their logical height.
		// TODO The logical width of grid Columns from the prior step is used in the formatting of grid items in
		// content-sized grid rows to determine their required height.
		computeUsedBreadthOfGridTracks(mRows, getHeight(), mRowGap.asUser(getHeight()), ::getRowSpan, ::getHeightMin,
									   ::getHeightMax);

		// 3. TODO If the minimum content size of any grid item has changed based on available height for the grid item
		// as computed in step 2, adjust the min content size of the grid item and restart the grid track sizing
		// algorithm (once only).
	} catch (const std::exception& exc) {
		DS_LOG_ERROR(exc.what())
	}

	t.stop();
	std::stringstream ss;
	ss << std::string("Running layout on ds::ui::Grid took ");
	ss << t.getSeconds() << " seconds.";
	DS_LOG_VERBOSE(0, ss.str());

	// Position anything that's not auto-positioned.
	size_t colCursor{0};
	size_t rowCursor{0};

	for (auto item : items) {
		auto col = item->getColumnSpan();
		auto row = item->getRowSpan();

		// TEMP while we don't have auto grid item placement
		if (item->isRowSpanAuto()) {
			if (rowCursor >= mRows.size()) continue;
			row = {rowCursor, rowCursor + 1};
		}
		if (item->isColumnSpanAuto()) {
			col = {colCursor, colCursor + 1};
			if (++colCursor >= mColumns.size()) {
				colCursor = 0;
				++rowCursor;
			}
		}

		const auto area = calcArea(col, row);
		item->fitInsideArea(area);
	}

#if 0 // Automatic Grid Item Placement Algorithm.

	// Process the items locked to a given row.
	for (auto item : items) {
		const auto& col = item->getColumnSpan();
		const auto& row = item->getRowSpan();
		if (col.count() && row.count()) continue;
		if (row.count()) {
			for (size_t i = 0; i < mColumns.size(); ++i) {
				const auto area = calcArea({i, i + 1}, row);
				if (!areaOverlapsItems(area, items)) {
					item->setColumnSpan({i, i + 1});

					item->setPosition(area.x1, area.y1);
					item->setSize(area.getWidth(), area.getHeight());

					auto text = dynamic_cast<Text*>(item);
					if (text) text->setResizeLimit(area.getWidth(), area.getHeight());
					break;
				}
			}
		}
	}

	// Determine the number of columns in the implicit grid.
	size_t numColumns = mColumns.size();
	for (auto item : items) {
		const auto& col = item->getColumnSpan();
		if (col.count()) {
			numColumns = glm::max(numColumns, col.max);
		}
	}

	// Position the remaining grid items.
	Range<size_t> cursor{0, 1};
	for (auto item : items) {
		const auto& col = item->getColumnSpan();
		const auto& row = item->getRowSpan();
		if (col.count() && row.count()) continue;
		if (col.count()) {
			// Set the column position of the cursor to be equal to the inline-start index of the grid item.
			cursor = {0, 1};

			// Increment the auto-placement cursor’s row position until a value is found where the grid item does
			// not overlap any occupied grid cells (creating new rows in the implicit grid as necessary). Position
			// the item’s block-start edge to the auto-placement cursor’s row position.
			auto area = calcArea(col, cursor);
			while (cursor.min < mRows.size() && areaOverlapsItems(area, items)) {
				++cursor.min;
				++cursor.max;

				// TODO create new row
				if (cursor.max > mRows.size()) break;

				area = calcArea(col, cursor);
			}

			item->setRowSpan(cursor);

			item->setPosition(area.x1, area.y1);
			item->setSize(area.getWidth(), area.getHeight());

			auto text = dynamic_cast<Text*>(item);
			if (text) text->setResizeLimit(area.getWidth(), area.getHeight());
		} else {
			item->setRowSpan({0,1});

			cursor = {0,1};

			// Increment the column position of the auto-placement cursor until this item’s grid area does not overlap
			// any occupied grid cells or overflow the number of columns determined in the previous step.
			auto area = calcArea(cursor, row);
			while (areaOverlapsItems(area, items)) {
				++cursor.min;
				++cursor.max;
				if(cursor.max > numColumns) break;

				area = calcArea(cursor, row);
			}

			// If a non-overlapping position was found in the previous step, position the item’s inline-start and
			// block-start edges to the auto-placement cursor’s column and row position, respectively Otherwise,
			// increment the auto-placement cursor’s row position (creating new rows in the implicit grid as necessary),
			// set its column position to 1, and return to the previous step.
			
			item->setColumnSpan(cursor);

			item->setPosition(area.x1, area.y1);
			item->setSize(area.getWidth(), area.getHeight());

			auto text = dynamic_cast<Text*>(item);
			if (text) text->setResizeLimit(area.getWidth(), area.getHeight());
		}
	}
#endif
	mInitialized = true;
	mNeedsLayout = false;

	onLayoutUpdate();
}

float Grid::calcPos(size_t index, const std::vector<Track>& tracks, float gap) {
	if (index > tracks.size()) throw std::runtime_error("Index out of range");
	float allocatedSpace = 0;
	for (size_t i = 0; i < index; ++i) {
		if (allocatedSpace > 0 && tracks.at(i).usedBreadth > 0) allocatedSpace += gap;
		allocatedSpace += tracks.at(i).usedBreadth;
	}
	return allocatedSpace;
}

void Grid::computeUsedBreadthOfGridTracks(std::vector<Track>& tracks, float percentOf, float gap, const SpanFn& spanFn,
										  const SizeFn& minFn, const SizeFn& maxFn) {
	// Initialize per grid track variables.
	for (auto& track : tracks)
		track.initialize(percentOf);

	// Resolve content-based TrackSizingFunctions
	resolveContentBasedTrackSizingFunctions(tracks, allItems(), spanFn, minFn, maxFn);

	// Grow all grid tracks from their UsedBreadth up to their MaxBreadth value until RemainingSpace is exhausted.

	// If RemainingSpace is defined
	float remainingSpace = calculateRemainingSpace(tracks, percentOf, gap);
	if (!approxZero(remainingSpace)) {
		// Iterate over all grid tracks and assign UsedBreadth to UpdatedTrackBreadth.
		for (auto& track : tracks)
			track.updatedTrackBreadth = track.usedBreadth;

		// Call DistributeSpaceToTracks
		distributeSpaceToTracks(tracks, remainingSpace, getTrackMaxBreadth, getAllTracks(tracks), {}, getTrackBase);

		// Iterate over all grid tracks and assign UpdatedTrackBreadth to UsedBreadth
		for (auto& track : tracks)
			track.usedBreadth = track.updatedTrackBreadth;
	} else {
		for (auto& track : tracks)
			track.usedBreadth = track.maxBreadth;
	}

	// Grow all grid tracks having a flexible length as the MaxTrackSizingFunction.
	float normalizedFlexBreadth = 0;

	// If RemainingSpace is defined
	remainingSpace = calculateRemainingSpace(tracks, percentOf, gap);
	if (!approxZero(remainingSpace)) {
		normalizedFlexBreadth = calculateNormalizedFlexBreadth(getAllTracks(tracks), percentOf, gap);
	} else {
		// i
		for (const auto& track : tracks) {
			if (track.max.isFlex()) {
				normalizedFlexBreadth = glm::max(normalizedFlexBreadth, track.usedBreadth / track.flexValue());
			}
		}
		//  ii
		for (const auto item : allItems()) {
			const auto spanned					 = getSpannedTracks(tracks, item, spanFn);
			const auto itemNormalizedFlexBreadth = calculateNormalizedFlexBreadth(spanned, maxFn(item), gap);
			normalizedFlexBreadth				 = glm::max(normalizedFlexBreadth, itemNormalizedFlexBreadth);
		}
	}

	for (auto& track : tracks)
		track.usedBreadth = glm::max(track.usedBreadth, normalizedFlexBreadth * track.flexValue());
}

void Grid::resolveContentBasedTrackSizingFunctions(std::vector<Track>& tracks, const std::vector<Sprite*>& items,
												   const SpanFn& spanFn, const SizeFn& minFn, const SizeFn& maxFn) {
	// Filter all grid items into a set, such that each grid item has either a SpanCount of 1 or does not cross a
	// flex-sized grid track.
	std::vector<Sprite*> filtered;

	for (auto item : items) {
		bool isFlex = false;

		const auto& span = spanFn(item);
		if (span.count() > 1) {
			for (size_t i = span.min; i < span.max; ++i) {
				if (i < tracks.size() && tracks.at(i).max.isFlex()) {
					isFlex = true;
					break;
				}
			}
		}

		if (!isFlex) filtered.push_back(item);
	}

	if (filtered.empty()) return;

	// Group all grid items in the filtered set by their SpanCount ascending.
	std::sort(filtered.begin(), filtered.end(), [spanFn](const Sprite* a, const Sprite* b) { //
		return spanFn(a).count() < spanFn(b).count();
	});

	const auto maxSpanCount = spanFn(filtered.back()).count();
	for (size_t spanCount = 1; spanCount <= maxSpanCount; ++spanCount) {
		const auto group = getItemsWithSpanCount(filtered, spanFn, spanCount);
		if (group.empty()) continue;

		// Resolve content-based MinTrackSizingFunctions.
		resolveContentBasedTrackSizingFunctionsForItems(
			tracks, //
			group,	// All grid items in the current group.
			[minFn,
			 spanFn](const std::vector<Track>& t,
					 const Sprite* i) { // A function which given a grid item returns the min-content size of that
										// grid item less the summed UsedBreadth of all grid tracks it covers.
				return calcAdditionSpaceBase(t, i, minFn, spanFn);
			},
			getTrackMaxBreadth, // A function which given a grid track returns its MaxBreadth.
			[spanFn](const std::vector<Track>& t,
					 const Sprite* i) { // A function which given a grid item returns the set of grid tracks covered by
										// that grid item that have a min-content or max-content MinTrackSizingFunction.
				const auto spanned = getSpannedTracks(t, i, spanFn);
				return getTracksMinIsMinOrMax(spanned);
			},
			getTracksMaxIsMinOrMax, // A function which given a set of grid tracks returns the subset of grid tracks
									// having a min-content or max-content MaxTrackSizingFunction. If that set is the
									// empty set, return the input set instead.
			getTrackUsedBreadthRef	// A function which given a grid track returns a reference to its UsedBreadth
									// variable.
		);

		resolveContentBasedTrackSizingFunctionsForItems(
			tracks, //
			group,	// All grid items in the current group.
			[maxFn,
			 spanFn](const std::vector<Track>& t,
					 const Sprite* i) { // A function which given a grid item returns the max-content size of that
										// grid item less the summed UsedBreadth of all Grid tracks it covers.
				return calcAdditionSpaceBase(t, i, maxFn, spanFn);
			},
			getTrackMaxBreadth, // A function which given a grid track returns its MaxBreadth.
			[spanFn](const std::vector<Track>& t,
					 const Sprite* i) { // A function which given a grid item returns the set of grid tracks covered
										// by that grid item that have a max-content MinTrackSizingFunction.
				const auto spanned = getSpannedTracks(t, i, spanFn);
				return getTracksMinIsMax(spanned);
			},
			getTracksMaxIsMax, // A function which given a set of grid tracks returns the subset of grid tracks having a
							   // max-content MaxTrackSizingFunction. If that set is the empty set, return the input set
							   // instead.
			getTrackUsedBreadthRef // A function which given a grid track returns a reference to its UsedBreadth
								   // variable.
		);

		// Resolve content-based MaxTrackSizingFunctions.
		resolveContentBasedTrackSizingFunctionsForItems(
			tracks, //
			group,	// All grid items in the current group.
			[minFn,
			 spanFn](const std::vector<Track>& t,
					 const Sprite* i) { // A function which given a grid item returns the min-content size of that
										// grid item less the summed MaxBreadth (unless the MaxBreadth is infinite, in
										// which case use the UsedBreadth) of all grid tracks it covers.
				return calcAdditionSpaceLimit(t, i, minFn, spanFn);
			},
			getTrackMaxBreadth, // A function which given a grid track returns its MaxBreadth.
			[spanFn](const std::vector<Track>& t,
					 const Sprite* i) { //  A function which given a grid item returns the set of grid tracks covered by
				//  that grid item that have a min-content or max-content MaxTrackSizingFunction.
				const auto spanned = getSpannedTracks(t, i, spanFn);
				return getTracksMaxIsMinOrMax(spanned);
			},
			getTracks,			  // The identity function.
			getTrackMaxBreadthRef // A function which given a grid track returns a reference to its MaxBreadth variable.
		);

		resolveContentBasedTrackSizingFunctionsForItems(
			tracks, //
			group,	// All grid items in the current group.
			[maxFn,
			 spanFn](const std::vector<Track>& t,
					 const Sprite* i) { // A function which given a grid item returns the max-content size of that
										// grid item less the summed MaxBreadth (unless the MaxBreadth is infinite, in
										// which case use the UsedBreadth) of all grid tracks it covers.
				return calcAdditionSpaceLimit(t, i, maxFn, spanFn);
			},
			[group](const Track& track) { // A function which given a grid track returns infinity if the grid
										  // track's SpanGroupInWhichMaxBreadthWasMadeFinite is equal to the
										  // current group; otherwise return the grid track's MaxBreadth.
				if (track.spanGroupInWhichMaxBreadthWasMadeFinite == group)
					return std::numeric_limits<float>::infinity();
				return track.maxBreadth;
			},
			[spanFn](const std::vector<Track>& t,
					 const Sprite* i) { // A function which given a grid item returns the set of grid tracks covered
										// by that grid item that have a max-content MaxTrackSizingFunction.
				const auto spanned = getSpannedTracks(t, i, spanFn);
				return getTracksMaxIsMax(spanned);
			},
			getTracks,			  // The identity function.
			getTrackMaxBreadthRef // A function which given a Grid track returns a reference to its MaxBreadth variable.
		);
	}

	// For each grid track from the set of all grid tracks:
	for (auto& track : tracks) {
		if (!std::isfinite(track.maxBreadth)) track.maxBreadth = track.usedBreadth;
	}
}

void Grid::resolveContentBasedTrackSizingFunctionsForItems(std::vector<Track>&						tracks, //
														   const std::vector<Sprite*>&				items,
														   const AdditionalSpaceFn&					spaceFn,
														   const TrackGrowthConstraintFn&			constraintFn,
														   const TracksForGrowthFn&					tracksFn,
														   const TracksForGrowthBeyondConstraintFn& tracksBeyondFn,
														   const AccumulatorFn&						accumulatorFn) {
	// A function which given a grid track returns the UsedBreadth of the grid track if Accumulator returns infinity;
	// otherwise the value of the Accumulator is returned.
	const auto currentBreadthFn = [accumulatorFn](const Track& t) {
		const auto acc = accumulatorFn(const_cast<Track&>(t));
		return std::isfinite(acc) ? acc : t.usedBreadth;
	};

	// Iterate over all grid tracks and assign UsedBreadth to UpdatedTrackBreadth.
	for (auto& track : tracks)
		track.updatedTrackBreadth = accumulatorFn(track);

	// DistributeSpaceToTracks.
	for (const auto item : items) {
		const auto spaceToDistribute = spaceFn(tracks, item);
		if (approxZero(spaceToDistribute)) continue;

		const auto tracksForGrowth = tracksFn(tracks, item);
		if (tracksForGrowth.empty()) continue;

		distributeSpaceToTracks(tracks, spaceToDistribute, constraintFn, tracksForGrowth,
								tracksBeyondFn(tracksForGrowth), currentBreadthFn);
	}

	// Iterate over all grid tracks and assign UpdatedTrackBreadth to UsedBreadth
	for (auto& track : tracks) {
		if (!std::isfinite(accumulatorFn(track)) && std::isfinite(track.updatedTrackBreadth))
			track.spanGroupInWhichMaxBreadthWasMadeFinite = items;
		accumulatorFn(track) = track.updatedTrackBreadth;
	}
}

void Grid::distributeSpaceToTracks(std::vector<Track>& tracks, float spaceToDistribute,
								   const TrackGrowthConstraintFn& constraintFn, std::vector<Track*> tracksFn,
								   const std::vector<Track*>& tracksBeyond, const BreadthFn& currentBreadthFn) {
	// 1. Sort TracksForGrowth by TrackGrowthConstraint( t ) - CurrentBreadth( t ) ascending.
	std::sort(tracksFn.begin(), tracksFn.end(), [constraintFn, currentBreadthFn](Track* a, Track* b) {
		const auto na = constraintFn(*a) - currentBreadthFn(*a);
		const auto nb = constraintFn(*b) - currentBreadthFn(*b);
		return na < nb;
	});

	// 2.
	for (size_t i = 0; i < tracksFn.size(); ++i) {
		const auto t = tracksFn.at(i);
		const auto share =
			glm::min(spaceToDistribute / float(tracksFn.size() - i), constraintFn(*t) - currentBreadthFn(*t));
		t->tempBreadth = currentBreadthFn(*t) + share;
		spaceToDistribute -= share;
	}

	// 3.
	if (spaceToDistribute > 0) {
		for (size_t i = 0; i < tracksBeyond.size(); ++i) {
			const auto t	 = tracksBeyond.at(i);
			const auto share = spaceToDistribute / float(tracksBeyond.size() - i);
			t->tempBreadth += share;
			spaceToDistribute -= share;
		}
	}

	// 4.
	for (const auto& track : tracksFn) {
		if (std::isfinite(track->updatedTrackBreadth))
			track->updatedTrackBreadth = glm::max(track->updatedTrackBreadth, track->tempBreadth);
		else
			track->updatedTrackBreadth = track->tempBreadth;
	}
}

float Grid::calculateNormalizedFlexBreadth(const std::vector<Track*>& tracks, float spaceToFill, float gap) {
	// 1.
	float allocatedSpace = 0;
	for (const auto& track : tracks) {
		if (allocatedSpace > 0 && track->usedBreadth > 0) allocatedSpace += gap;
		allocatedSpace += track->usedBreadth;
	}

	// 2.
	auto flexTracks = getFlexTracks(tracks);

	// 3.
	for (const auto& track : flexTracks)
		track->normalizedFlexValue = track->usedBreadth / track->flexValue();

	// 4.
	std::sort(flexTracks.begin(), flexTracks.end(),
			  [](const Track* a, const Track* b) { return a->normalizedFlexValue < b->normalizedFlexValue; });

	// 5 + 6.
	float spaceNeededFromFlexTracks	 = spaceToFill - allocatedSpace;
	float currentBandFractionBreadth = 0;
	float accumulatedFractions		 = 0;

	// 7.
	for (const auto track : flexTracks) {
		if (track->normalizedFlexValue > currentBandFractionBreadth) {
			if (track->normalizedFlexValue * accumulatedFractions > spaceNeededFromFlexTracks) break;
			currentBandFractionBreadth = track->normalizedFlexValue;
		}
		accumulatedFractions += track->flexValue();
		spaceNeededFromFlexTracks += track->usedBreadth;
	}

	// 8.
	return spaceNeededFromFlexTracks / accumulatedFractions;
}

float Grid::calculateRemainingSpace(const std::vector<Track>& tracks, float spaceToFill, float gap) {
	if (std::isinf(spaceToFill) || std::isnan(spaceToFill)) return std::numeric_limits<float>::signaling_NaN();

	float allocatedSpace = 0;
	for (const auto& track : tracks) {
		if (allocatedSpace > 0 && track.usedBreadth > 0) allocatedSpace += gap;
		allocatedSpace += track.usedBreadth;
	}

	return glm::max(0.0f, spaceToFill - allocatedSpace);
}

std::vector<Sprite*> Grid::allItems() {
	return getChildren();
}

std::vector<Sprite*> Grid::nonFlexibleItems(const std::vector<Track>& tracks, const SpanFn& spanFn) {
	std::vector<Sprite*> result = allItems();

	for (auto itr = result.begin(); itr != result.end();) {
		const size_t count = result.size();

		const auto& span = spanFn(*itr);
		for (size_t i = span.min; i < span.max; ++i) {
			if (i < tracks.size() && tracks.at(i).max.isFlex()) {
				itr = result.erase(itr);
				break;
			}
		}

		if (count == result.size()) ++itr;
	}

	std::sort(result.begin(), result.end(), [spanFn](const Sprite* a, const Sprite* b) { //
		return spanFn(a).count() < spanFn(b).count();
	});

	return result;
}

void Grid::parse(std::vector<Track>& tracks, const std::string& def) {
	auto sInOut = def.c_str();
	skipSpace(&sInOut);

	while (*sInOut) {
		if (strncmp(sInOut, "repeat", 6) == 0) {
			sInOut += 6;
			skipSpaceOrParenthesis(&sInOut);
			// TODO: add support for auto-fill and auto-fit
			const auto n = parseInt(&sInOut);
			if (n <= 0) throw std::runtime_error("Error parsing repeat(): expected counter > 0");
			skipSpaceOrComma(&sInOut);
			auto s = fetchUntil(&sInOut, ')');
			skipSpaceOrParenthesis(&sInOut);
			for (int i = 0; i < n; ++i) {
				parse(tracks, s);
			}
		} else if (*sInOut == '[') {
			sInOut += 1;
			auto s = fetchUntil(&sInOut, ']');
			DS_LOG_WARNING("Grid line names '" << s << "' are currently not supported.");
			sInOut += 1;
			skipSpace(&sInOut);
		} else {
			tracks.emplace_back(&sInOut); // Create track.
			skipSpace(&sInOut);
		}
	}
}

Range<size_t> Grid::parseSpan(const char** sInOut) {
	skipSpace(sInOut);
	const auto minimum = parseInt(sInOut) - 1;
	skipUntil(sInOut, '/');
	auto maximum = minimum + 1;
	if (**sInOut == '/') {
		(*sInOut)++;
		skipSpace(sInOut);
		if (isNumeric(**sInOut))
			maximum = parseInt(sInOut) - 1;
		else if (strncmp(*sInOut, "span", 4) == 0) {
			*sInOut += 4;
			skipSpace(sInOut);
			maximum = minimum + parseInt(sInOut);
		}
	}

	assert(minimum >= 0);
	assert(maximum >= 0);
	assert(minimum <= maximum);

	return {size_t(minimum), size_t(maximum)};
}

SizingFn::SizingFn(const std::string& str) {
	const char* sInOut = str.c_str();
	parse(&sInOut);
}

SizingFn::SizingFn(const char** sInOut) {
	parse(sInOut);
}

void SizingFn::parse(const char** sInOut) {
	skipSpace(sInOut);
	if (strncmp(*sInOut, "auto", 4) == 0) {
		*sInOut += 4;
		mUnit = UNDEFINED;
	} else if (strncmp(*sInOut, "min-content", 11) == 0) {
		*sInOut += 11;
		mUnit = MIN_CONTENT;
	} else if (strncmp(*sInOut, "max-content", 11) == 0) {
		*sInOut += 11;
		mUnit = MAX_CONTENT;
	} else {
		mValue = Value(sInOut);
		mUnit  = FIXED;
	}
}

Grid::Track::Track(const std::string& str) {
	const char* sInOut = str.c_str();
	parse(&sInOut);
}

Grid::Track::Track(const char** sInOut) {
	parse(sInOut);
}

void Grid::Track::parse(const char** sInOut) {
	skipSpace(sInOut);
	if (strncmp(*sInOut, "auto", 4) == 0) {
		*sInOut += 4;
		min = SizingFn("min-content");
		max = SizingFn("max-content");
	} else if (strncmp(*sInOut, "minmax", 6) == 0) {
		*sInOut += 6;
		skipSpaceOrParenthesis(sInOut);
		min = SizingFn(sInOut);
		skipSpaceOrComma(sInOut);
		max = SizingFn(sInOut);
		skipSpaceOrParenthesis(sInOut);
	} else if (strncmp(*sInOut, "fit-content", 11) ==
			   0) { // TODO not yet in the specification used for this version of the code.
		*sInOut += 11;
		skipSpaceOrParenthesis(sInOut);
		min = max = SizingFn(sInOut);
		if (!min.isIntrinsic()) throw std::runtime_error("Sizing function must be intrinsic");
		skipSpaceOrParenthesis(sInOut);
	} else { // includes flex values
		min = max = SizingFn(sInOut);
	}
}

void Grid::Track::initialize(float percentOf) {
	// Sizing functions should be properly initialized before running the algorithm.
	assert(min);
	assert(max);

	tempBreadth			= 0;
	updatedTrackBreadth = 0;
	updatedLimit		= 0;

	// SpanGroupInWhichMaxBreadthWasMadeFinite = null
	spanGroupInWhichMaxBreadthWasMadeFinite.clear();

	if (min.isFixed()) {
		// If MinTrackSizingFunction is a percentage or length, then UsedBreadth = resolved length
		usedBreadth = min.value().asUser(percentOf);
	} else {
		// If MinTrackSizingFunction is min-content, max-content, or a flexible length, then UsedBreadth = 0
		usedBreadth = 0;
	}

	if (max.isFixed()) {
		// If MaxTrackSizingFunction is percentage or length, then MaxBreadth = resolved length.
		// If the resolved length of the MaxTrackSizingFunction is less than the MinTrackSizingFunction, MaxBreadth =
		// UsedBreadth.
		maxBreadth = glm::max(usedBreadth, max.value().asUser(percentOf));
	} else if (max.isFlex()) {
		// If MaxTrackSizingFunction is a flexible length, then MaxBreadth = UsedBreadth
		maxBreadth = usedBreadth;
	} else {
		// If MaxTrackSizingFunction is min-content, or max-content, then MaxBreadth = Infinity
		maxBreadth = std::numeric_limits<float>::infinity();
	}
}

} // namespace ds::ui

namespace {

auto INIT = []() {
	ds::App::AddStartup([](ds::Engine& e) {
		// Register the properties for our grid.
		e.registerSpritePropertySetter<Grid>("grid-template-columns",
											 [](Grid& grid, const std::string& theValue,
												const std::string& fileReferrer) { grid.setColumns(theValue); });
		e.registerSpritePropertySetter<Grid>(
			"grid-template-rows",
			[](Grid& grid, const std::string& theValue, const std::string& fileReferrer) { grid.setRows(theValue); });
		e.registerSpritePropertySetter<Grid>(
			"grid-gap",
			[](Grid& grid, const std::string& theValue, const std::string& fileReferrer) { grid.setGap(theValue); });
		e.registerSpritePropertySetter<Grid>("grid-column-gap",
											 [](Grid& grid, const std::string& theValue,
												const std::string& fileReferrer) { grid.setColumnGap(theValue); });
		e.registerSpritePropertySetter<Grid>(
			"grid-row-gap",
			[](Grid& grid, const std::string& theValue, const std::string& fileReferrer) { grid.setRowGap(theValue); });

		// Register sprite properties used by the grid system.
		e.registerSpritePropertySetter<Sprite>(
			"grid-column", [](Sprite& item, const std::string& theValue, const std::string& fileReferrer) {
				const char* sInOut = theValue.c_str();
				item.setColumnSpan(Grid::parseSpan(&sInOut));
			});
		e.registerSpritePropertySetter<Sprite>(
			"grid-row", [](Sprite& item, const std::string& theValue, const std::string& fileReferrer) {
				const char* sInOut = theValue.c_str();
				item.setRowSpan(Grid::parseSpan(&sInOut));
			});
	});
	return true;
}();

}