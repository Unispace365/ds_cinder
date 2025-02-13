#include "stdafx.h"

#include <ds/ui/grid/grid.h>

using namespace ds::css;

namespace ds::ui {

Grid::Grid(SpriteEngine& engine)
  : Sprite(engine)
  , mEventClient(engine) {
	setTransparent(false); // For debugging.
}

void Grid::onChildAdded(Sprite& sprite) {
	mNeedsLayout = true;
	sprite.setDimensionsChangedCallback([&](Sprite* s) { mNeedsLayout |= !s->animationRunning(); });
	sprite.onAddedToLayout(this);
}

void Grid::onChildRemoved(Sprite& sprite) {
	mNeedsLayout = true;
	sprite.setDimensionsChangedCallback(nullptr);
}

ci::Rectf Grid::calcArea(const Range<size_t>& column, const Range<size_t>& row) const {
	if (column.min == column.max || row.min == row.max) return {0, 0, getWidth(), getHeight()};
	if (column.min >= mHorizontalGridLines.size() || column.max > mHorizontalGridLines.size() ||
		row.min >= mVerticalGridLines.size() || row.max > mVerticalGridLines.size())
		return {0, 0, getWidth(), getHeight()};

	const auto x1 = mHorizontalGridLines.at(column.min);
	const auto y1 = mVerticalGridLines.at(row.min);
	const auto x2 = mHorizontalGridLines.at(column.max);
	const auto y2 = mVerticalGridLines.at(row.max);
	return {x1, y1, x2, y2};
}

void Grid::drawLocalClient() {
	if (mNeedsLayout) performGridLayout();
}

void Grid::drawPostLocalClient() {
	if (getDebugging()) {
		ci::gl::ScopedColor		 sc(1, 1, 1);
		ci::gl::ScopedBlendAlpha sb;
		ci::gl::ScopedGlslProg	 sp(getStockShader(ci::gl::ShaderDef().color()));

		float width	 = getWidth();
		float height = getHeight();

		ci::gl::begin(GL_LINES);
		ci::gl::color(ci::ColorA8u(255, 204, 0, 255));
		for (float x : mHorizontalGridLines) {
			ci::gl::vertex(x, 0);
			ci::gl::vertex(x, height);
		}
		for (float y : mVerticalGridLines) {
			ci::gl::vertex(0, y);
			ci::gl::vertex(width, y);
		}
		ci::gl::end();

		ci::gl::color(ci::ColorA8u(255, 204, 0, 255));
		ci::gl::drawStrokedRect({0, 0, width, height}, 5);

		if (!mChildren.empty()) {
			const bool hasColumnGaps =
				!mColumnGapDef.empty() && Value(mColumnGapDef).asUser(this, Value::Direction::HORIZONTAL) > 0;
			const bool hasRowGaps =
				!mRowGapDef.empty() && Value(mRowGapDef).asUser(this, Value::Direction::VERTICAL) > 0;

			for (const auto child : mChildren) {
				ci::gl::color(ci::ColorA8u(204, 51, 0, 128));
				ci::gl::drawStrokedRect(child->getBoundingBox(), 1);

				const auto col = adjustForGaps(child->getColumnSpan(), hasColumnGaps);
				const auto row = adjustForGaps(child->getRowSpan(), hasRowGaps);
				ci::gl::color(ci::ColorA8u(255, 102, 0, 128));
				ci::gl::drawStrokedRect(calcArea(col, row), 3);
			}
		}
	}
}

void Grid::addChild(Sprite& newChild) {
	mNeedsLayout = true;
	Sprite::addChild(newChild);
}

bool Grid::setAvailableSize(const ci::vec2& size, float& minWidth, float& minHeight, float& maxWidth, float& maxHeight,
							bool favorWidthOverHeight) {
	// Keep track of the size, so we can restore it later (size change should not be permanent).
	const auto originalSize = getSize();

	// Resize as requested and perform layout.
	const auto padding = ci::vec2(mLayoutLPad + mLayoutRPad, mLayoutTPad + mLayoutBPad);
	setSize(size - padding);

	if (mNeedsLayout) performGridLayout();

	// Calculate inner and outer bounds.
	const auto outer = ci::Rectf{0, 0, size.x - padding.x, size.y - padding.y};
	const auto inner = ci::Rectf{0, 0, getWidth(), getHeight()};

	// Calculate size constraints.
	bool changed =
		Sprite::setAvailableSize(size, inner, outer, minWidth, minHeight, maxWidth, maxHeight, favorWidthOverHeight);

	// Restore original size but keep the layout.
	setSize(originalSize);
	mNeedsLayout = false;

	// Return whether anything changed.
	return changed;
}

void Grid::fitInsideArea(const ci::Rectf& area) {
	const auto padding = ci::vec2(mLayoutLPad + mLayoutRPad, mLayoutTPad + mLayoutBPad);
	const auto size	   = area.getSize();
	setSize(size - padding);

	if (mNeedsLayout) performGridLayout();

	Sprite::fitInsideArea(area);
}

void Grid::onAddedToLayout(Sprite* layout) {
	// When placed inside a grid, we assume you want to allow this grid to resize non-uniformly.
	const auto grid = dynamic_cast<Grid*>(layout);
	if (grid) mFit = "none";
}

bool Grid::areaOverlapsItem(const ci::Rectf& area, const Item& item) const {
	const auto itemArea = calcArea(item);
	return itemArea.intersects(area);
}

bool Grid::areaOverlapsItems(const ci::Rectf& area, const std::vector<Item>& items) const {
	for (const auto& item : items) {
		if (areaOverlapsItem(area, item)) return true;
	}
	return false;
}

std::vector<Grid::Track> Grid::parseTracks(const std::string& def) {
	std::vector<Track> result;
	parse(result, def);
	return result;
}

std::vector<Grid::Track> Grid::parseTracks(const std::string& def, const std::string& gap) {
	std::vector<Track> result;
	parse(result, def);
	if (result.size() < 2) return result;

	size_t originalSize = result.size();
	result.resize(result.size() * 2 - 1);
	for (size_t i = originalSize - 1; i > 0; --i) {
		std::swap(result[i * 2], result[i]);
		result[i * 2 - 1] = Track{gap};
	}
	return result;
}

void Grid::calculateGridLines(const std::vector<Track>& tracks, std::vector<float>& gridLines) {
	gridLines.clear();
	gridLines.reserve(tracks.size() + 1);

	float x = 0;
	for (const auto& track : tracks) {
		gridLines.push_back(x);
		x += track.usedBreadth;
	}
	gridLines.push_back(x);
}

void Grid::performGridLayout() {
	// Make sure nested layouts are updated.
	for (auto child : mChildren) {
		auto layout = dynamic_cast<ILayout*>(child);
		if (layout) layout->runLayout();
	}

	// Initialize items. These are then updated during layout.
	// TODO sort items by specified order, see:
	// https://drafts.csswg.org/css-flexbox-1/#order-modified-document-order
	auto items = allItems();

	// Create tracks.
	const bool hasColumnGaps =
		!mColumnGapDef.empty() && Value(mColumnGapDef).asUser(this, Value::Direction::HORIZONTAL) > 0;
	const bool hasRowGaps = !mRowGapDef.empty() && Value(mRowGapDef).asUser(this, Value::Direction::VERTICAL) > 0;

	try {
		// Get specified size. May be 0 or inf.
		float width	 = getWidth();
		float height = getHeight();
		if (ds::approxZero(width)) width = std::numeric_limits<float>::infinity();
		if (ds::approxZero(height)) height = std::numeric_limits<float>::infinity();

		bool hasChanged = false;
		for (;;) {
			auto columns = hasColumnGaps ? parseTracks(mColumnsDef, mColumnGapDef) : parseTracks(mColumnsDef);
			auto rows	 = hasRowGaps ? parseTracks(mRowsDef, mRowGapDef) : parseTracks(mRowsDef);

			// 1. Call ComputedUsedBreadthOfGridTracks for grid columns to resolve their logical width.
			computeUsedBreadthOfGridTracks(width, columns, items, getColumnSpan, getWidthMin, getWidthMax);

			// 2. Call ComputedUsedBreadthOfGridTracks for grid rows to resolve their logical height.
			// TODO The logical width of grid Columns from the prior step is used in the formatting of grid items in
			// content-sized grid rows to determine their required height.
			computeUsedBreadthOfGridTracks(height, rows, items, getRowSpan, getHeightMin, getHeightMax);

			// Determine the grid lines.
			calculateGridLines(columns, mHorizontalGridLines);
			calculateGridLines(rows, mVerticalGridLines);

			// 3. If the minimum content size of any grid item has changed based on available height for the grid
			// item as computed in step 2, adjust the min content size of the grid item and restart the grid track
			// sizing algorithm (once only).
			if (hasChanged) break;

			for (auto& item : items) {
				const auto area = calcArea(item);

				hasChanged |= item.sprite->setAvailableSize(area.getSize(), item.minWidth, item.minHeight,
															item.maxWidth, item.maxHeight, canGrow(height));
			}

			if (!hasChanged) break;
		}
	} catch (const std::exception& exc) {
		DS_LOG_ERROR(exc.what())
	}

	// Position items.
	for (const auto& item : items) {
		const auto area = calcArea(item);
		item.sprite->fitInsideArea(area);
	}

	// Done.
	mInitialized = true;
	mNeedsLayout = false;

	onLayoutUpdate();
}

float Grid::calcPos(size_t index, const std::vector<Track>& tracks, bool excludeFlex) {
	float allocatedSpace = 0;
	for (size_t i = 0; i < index && i < tracks.size(); ++i) {
		if (!std::isfinite(tracks.at(i).usedBreadth)) continue;
		if (excludeFlex && tracks.at(i).isFlex()) continue;
		allocatedSpace += tracks.at(i).usedBreadth;
	}

	return allocatedSpace;
}

float Grid::calcPos(size_t index, const std::vector<Track*>& tracks, bool excludeFlex) {
	float allocatedSpace = 0;
	for (size_t i = 0; i < index && i < tracks.size(); ++i) {
		if (!std::isfinite(tracks.at(i)->usedBreadth)) continue;
		if (excludeFlex && tracks.at(i)->isFlex()) continue;
		allocatedSpace += tracks.at(i)->usedBreadth;
	}

	return allocatedSpace;
}

Range<size_t> Grid::adjustForGaps(const Range<size_t>& span, bool hasGaps) {
	return hasGaps ? Range<size_t>{span.min * 2, span.max * 2 - 1} : span;
}

void Grid::computeUsedBreadthOfGridTracks(float spaceToFill, std::vector<Track>& tracks, const std::vector<Item>& items,
										  const SpanFn& spanFn, const SizeFn& minFn, const SizeFn& maxFn) const {
	const auto viewportSize = glm::vec2{mEngine.getWorldWidth(), mEngine.getWorldHeight()};

	// Initialize per grid track variables.
	for (auto& track : tracks)
		track.initialize({spaceToFill, viewportSize});

	// Resolve content-based TrackSizingFunctions
	resolveContentBasedTrackSizingFunctions(tracks, items, spanFn, minFn, maxFn);

	// Grow all grid tracks from their UsedBreadth up to their MaxBreadth value until RemainingSpace is exhausted.

	// If RemainingSpace is defined
	float remainingSpace = calculateRemainingSpace(tracks, spaceToFill);
	if (!approxZero(remainingSpace)) {
		// Iterate over all grid tracks and assign UsedBreadth to UpdatedTrackBreadth.
		for (auto& track : tracks)
			track.updatedTrackBreadth = track.usedBreadth;

		// Call DistributeSpaceToTracks
		distributeSpaceToTracks(remainingSpace, getTrackMaxBreadth, getAllTracks(tracks), {}, getTrackBase);

		// Iterate over all grid tracks and assign UpdatedTrackBreadth to UsedBreadth
		for (auto& track : tracks)
			track.usedBreadth = track.updatedTrackBreadth;
	} else if (canGrow(spaceToFill)) {
		// Note: only if grid can grow in size.
		for (auto& track : tracks)
			track.usedBreadth = track.maxBreadth;
	}

	// Grow all grid tracks having a flexible length as the MaxTrackSizingFunction.
	float normalizedFlexBreadth = 0;

	remainingSpace = calculateRemainingSpace(tracks, spaceToFill);
	if (!approxZero(remainingSpace)) {
		// If RemainingSpace is defined
		normalizedFlexBreadth = calculateNormalizedFlexBreadth(getAllTracks(tracks), spaceToFill);
	} else {
		// i
		for (const auto& track : tracks) {
			if (track.max.isFlex()) {
				normalizedFlexBreadth = glm::max(normalizedFlexBreadth, track.usedBreadth / track.flexValue());
			}
		}
		//  ii
		for (const auto& item : items) {
			const auto spanned =
				/*canGrow(spaceToFill) ? getSpannedTracks(tracks, item, spanFn) :*/ getAllTracks(tracks);
			const auto itemNormalizedFlexBreadth = calculateNormalizedFlexBreadth(spanned, maxFn(item));
			normalizedFlexBreadth				 = glm::max(normalizedFlexBreadth, itemNormalizedFlexBreadth);
		}
	}

	if (!approxZero(normalizedFlexBreadth)) {
		for (auto& track : tracks)
			track.usedBreadth = glm::max(track.usedBreadth, normalizedFlexBreadth * track.flexValue());
	}
}

void Grid::resolveContentBasedTrackSizingFunctions(std::vector<Track>& tracks, const std::vector<Item>& items,
												   const SpanFn& spanFn, const SizeFn& minFn, const SizeFn& maxFn) {
	// Filter all grid items into a set, such that each grid item has either a SpanCount of 1 or does not cross a
	// flex-sized grid track.
	std::vector<Item> filtered;

	for (const auto& item : items) {
		bool isValid = true;

		const auto span = spanFn(item);
		if (span.count() > 1) {
			for (size_t i = span.min; i < span.max; ++i) {
				if (i < tracks.size() && tracks.at(i).max.isFlex()) {
					isValid = false;
					break;
				}
			}
		}

		if (isValid) filtered.push_back(item);
	}

	if (filtered.empty()) return;

	// Group all grid items in the filtered set by their SpanCount ascending.
	std::sort(filtered.begin(), filtered.end(), [spanFn](const Item& a, const Item& b) { //
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
			[minFn, spanFn](const std::vector<Track>& t,
							const Item& i) { // A function which given a grid item returns the min-content size of that
											 // grid item less the summed UsedBreadth of all grid tracks it covers.
				return calcAdditionSpaceBase(t, i, minFn, spanFn);
			},
			getTrackMaxBreadth, // A function which given a grid track returns its MaxBreadth.
			[spanFn](const std::vector<Track>& t,
					 const Item& i) { // A function which given a grid item returns the set of grid tracks covered by
				// that grid item that have a min-content or max-content MinTrackSizingFunction.
				const auto spanned = getSpannedTracks(t, i, spanFn);
				return getTracksMinIsMinOrMax(spanned);
			},
			getTracksMaxIsMinOrMax, // A function which given a set of grid tracks returns the subset of grid tracks
			// having a min-content or max-content MaxTrackSizingFunction. If that set is the
			// empty set, return the input set instead.
			getTrackUsedBreadthRef // A function which given a grid track returns a reference to its UsedBreadth
								   // variable.
		);

		resolveContentBasedTrackSizingFunctionsForItems(
			tracks, //
			group,	// All grid items in the current group.
			[maxFn, spanFn](const std::vector<Track>& t,
							const Item& i) { // A function which given a grid item returns the max-content size of that
											 // grid item less the summed UsedBreadth of all Grid tracks it covers.
				return calcAdditionSpaceBase(t, i, maxFn, spanFn);
			},
			getTrackMaxBreadth, // A function which given a grid track returns its MaxBreadth.
			[spanFn](const std::vector<Track>& t,
					 const Item& i) { // A function which given a grid item returns the set of grid tracks covered
									  // by that grid item that have a max-content MinTrackSizingFunction.
				const auto spanned = getSpannedTracks(t, i, spanFn);
				return getTracksMinIsMax(spanned);
			},
			getTracksMaxIsMax, // A function which given a set of grid tracks returns the subset of grid tracks
							   // having a
			// max-content MaxTrackSizingFunction. If that set is the empty set, return the input set
			// instead.
			getTrackUsedBreadthRef // A function which given a grid track returns a reference to its UsedBreadth
								   // variable.
		);

		// Resolve content-based MaxTrackSizingFunctions.
		resolveContentBasedTrackSizingFunctionsForItems(
			tracks, //
			group,	// All grid items in the current group.
			[minFn, spanFn](const std::vector<Track>& t,
							const Item& i) { // A function which given a grid item returns the min-content size of that
											 // grid item less the summed MaxBreadth (unless the MaxBreadth is infinite,
											 // in which case use the UsedBreadth) of all grid tracks it covers.
				return calcAdditionSpaceLimit(t, i, minFn, spanFn);
			},
			getTrackMaxBreadth, // A function which given a grid track returns its MaxBreadth.
			[spanFn](const std::vector<Track>& t,
					 const Item& i) { //  A function which given a grid item returns the set of grid tracks covered by
				//  that grid item that have a min-content or max-content MaxTrackSizingFunction.
				const auto spanned = getSpannedTracks(t, i, spanFn);
				return getTracksMaxIsMinOrMax(spanned);
			},
			getTracks,			  // The identity function.
			getTrackMaxBreadthRef // A function which given a grid track returns a reference to its MaxBreadth
								  // variable.
		);

		resolveContentBasedTrackSizingFunctionsForItems(
			tracks, //
			group,	// All grid items in the current group.
			[maxFn, spanFn](const std::vector<Track>& t,
							const Item& i) { // A function which given a grid item returns the max-content size of that
											 // grid item less the summed MaxBreadth (unless the MaxBreadth is infinite,
											 // in which case use the UsedBreadth) of all grid tracks it covers.
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
					 const Item& i) { // A function which given a grid item returns the set of grid tracks covered
									  // by that grid item that have a max-content MaxTrackSizingFunction.
				const auto spanned = getSpannedTracks(t, i, spanFn);
				return getTracksMaxIsMax(spanned);
			},
			getTracks,			  // The identity function.
			getTrackMaxBreadthRef // A function which given a Grid track returns a reference to its MaxBreadth
								  // variable.
		);
	}

	// For each grid track from the set of all grid tracks:
	for (auto& track : tracks) {
		if (!std::isfinite(track.maxBreadth)) track.maxBreadth = track.usedBreadth;
	}
}

void Grid::resolveContentBasedTrackSizingFunctionsForItems(std::vector<Track>&						tracks, //
														   const std::vector<Item>&					items,
														   const AdditionalSpaceFn&					spaceFn,
														   const TrackGrowthConstraintFn&			constraintFn,
														   const TracksForGrowthFn&					tracksFn,
														   const TracksForGrowthBeyondConstraintFn& tracksBeyondFn,
														   const AccumulatorFn&						accumulatorFn) {
	// A function which given a grid track returns the UsedBreadth of the grid track if Accumulator returns
	// infinity; otherwise the value of the Accumulator is returned.
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

		distributeSpaceToTracks(spaceToDistribute, constraintFn, tracksForGrowth, tracksBeyondFn(tracksForGrowth),
								currentBreadthFn);
	}

	// Iterate over all grid tracks and assign UpdatedTrackBreadth to UsedBreadth
	for (auto& track : tracks) {
		if (!std::isfinite(accumulatorFn(track)) && std::isfinite(track.updatedTrackBreadth))
			track.spanGroupInWhichMaxBreadthWasMadeFinite = items;
		accumulatorFn(track) = track.updatedTrackBreadth;
	}
}

void Grid::distributeSpaceToTracks(float spaceToDistribute, const TrackGrowthConstraintFn& constraintFn,
								   std::vector<Track*> tracks, const std::vector<Track*>& tracksBeyond,
								   const BreadthFn& currentBreadthFn) {
	// 1. Sort TracksForGrowth by TrackGrowthConstraint( t ) - CurrentBreadth( t ) ascending.
	std::sort(tracks.begin(), tracks.end(), [constraintFn, currentBreadthFn](Track* a, Track* b) {
		const auto na = constraintFn(*a) - currentBreadthFn(*a);
		const auto nb = constraintFn(*b) - currentBreadthFn(*b);
		return na < nb;
	});

	// 2.
	for (size_t i = 0; i < tracks.size(); ++i) {
		const auto t = tracks.at(i);
		const auto share =
			glm::min(spaceToDistribute / float(tracks.size() - i), constraintFn(*t) - currentBreadthFn(*t));
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
	for (const auto track : tracks) {
		if (std::isfinite(track->updatedTrackBreadth))
			track->updatedTrackBreadth = glm::max(track->updatedTrackBreadth, track->tempBreadth);
		else
			track->updatedTrackBreadth = track->tempBreadth;
	}
}

float Grid::calculateNormalizedFlexBreadth(const std::vector<Track*>& tracks, float spaceToFill) {
	// 1.
	const float allocatedSpace = calcPos(tracks.size(), tracks);

	// 2.
	auto flexTracks = getFlexTracks(tracks);

	// 3.
	for (const auto& track : flexTracks)
		track->normalizedFlexValue = track->usedBreadth / track->flexValue();

	// 4.
	std::sort(flexTracks.begin(), flexTracks.end(),
			  [](const Track* a, const Track* b) { return a->normalizedFlexValue < b->normalizedFlexValue; });

	// 5 + 6.
	float spaceNeededFromFlexTracks	 = glm::max(0.f, spaceToFill - allocatedSpace);
	float currentBandFractionBreadth = 0;
	float accumulatedFractions		 = 0;

	if (flexTracks.empty()) return 0;

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

float Grid::calculateRemainingSpace(const std::vector<Track>& tracks, float spaceToFill) {
	if (std::isinf(spaceToFill) || std::isnan(spaceToFill)) return 0;
	const float allocatedSpace = calcPos(tracks.size(), tracks);
	return glm::max(0.0f, spaceToFill - allocatedSpace);
}

std::vector<Grid::Item> Grid::allItems() const {
	std::vector<Item> result;

	if (!mChildren.empty()) {
		const bool hasColumnGaps =
			!mColumnGapDef.empty() && Value(mColumnGapDef).asUser(this, Value::Direction::HORIZONTAL) > 0;
		const bool hasRowGaps = !mRowGapDef.empty() && Value(mRowGapDef).asUser(this, Value::Direction::VERTICAL) > 0;
		const auto gridSize	  = getSize();

		for (auto child : mChildren) {
			const auto colSpan = adjustForGaps(child->getColumnSpan(), hasColumnGaps);
			const auto rowSpan = adjustForGaps(child->getRowSpan(), hasRowGaps);
			result.emplace_back(child, colSpan, rowSpan, gridSize);
		}
	}

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

Grid::SizingFn::SizingFn(const std::string& str) {
	const char* sInOut = str.c_str();
	parse(&sInOut);
}

Grid::SizingFn::SizingFn(const char** sInOut) {
	parse(sInOut);
}

void Grid::SizingFn::parse(const char** sInOut) {
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

void Grid::Track::initialize(const Value::Dimensions& dimensions) {
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
		usedBreadth = min.value().asUser(dimensions);
	} else {
		// If MinTrackSizingFunction is min-content, max-content, or a flexible length, then UsedBreadth = 0
		usedBreadth = 0;
	}

	if (max.isFixed()) {
		// If MaxTrackSizingFunction is percentage or length, then MaxBreadth = resolved length.
		// If the resolved length of the MaxTrackSizingFunction is less than the MinTrackSizingFunction, MaxBreadth
		// = UsedBreadth.
		maxBreadth = glm::max(usedBreadth, max.value().asUser(dimensions));
	} else if (max.isFlex()) {
		// If MaxTrackSizingFunction is a flexible length, then MaxBreadth = UsedBreadth
		maxBreadth = usedBreadth;
	} else {
		// If MaxTrackSizingFunction is min-content, or max-content, then MaxBreadth = Infinity
		maxBreadth = std::numeric_limits<float>::infinity();
	}
}

} // namespace ds::ui