#include "stdafx.h"

#include "ds/debug/logger.h"
#include "ds/ui/panel/panel_layouts.h"
#include "ds/util/pixel_packer/pak/algoMaxRects.h"
#include "ds/util/pixel_packer/util/myVector2.h"

#include <algorithm>
#include <map>

namespace ds { namespace ui {

	struct PanelPackage {
		int			panelIndex;
		float		aspectRatio;
		ci::vec2	size;	  // for row packing
		ci::vec2	position; // for row packing
		t_myVector2 packSize; // for the bin packing algo
		ci::Rectf	outputRect;

		static bool sortByBigness(const PanelPackage& a, const PanelPackage& b) {
			return a.packSize.x * a.packSize.y > b.packSize.x * b.packSize.y;
		}

		static float adjustSizes(std::vector<PanelPackage>& packages, float fractionalAmount, float padding) {
			float piecemealArea = 0.0f;
			for (auto it = packages.begin(); it < packages.end(); ++it) {
				float tw = float((*it).packSize.x) * fractionalAmount - padding;
				float th = tw / (*it).aspectRatio;
				tw += padding;
				th += padding;

				piecemealArea += tw * th;
				it->packSize.x = int(tw);
				it->packSize.y = int(th);
			}

			return piecemealArea;
		}
	};

	bool PanelLayouts::binPack(std::vector<ds::ui::BasePanel*> panels, const ci::Rectf& totalAreaRect, float padding,
							   float animationDuration) {
		if (panels.empty()) return false;

		std::vector<PanelPackage> thePackages;
		int						  ind = 0;

		ci::vec2 totalArea = ci::vec2(totalAreaRect.getWidth(), totalAreaRect.getHeight());

		float totalAreaAmount = totalArea.x * totalArea.y;
		float piecemealArea	  = 0.0f;
		for (auto it = panels.begin(); it < panels.end(); ++it) {
			float tw = (*it)->getScaleWidth();
			float th = (*it)->getScaleHeight();
			if (tw < 1.0f || th < 1.0f) continue;

			PanelPackage pp;
			pp.aspectRatio = (*it)->getContentAspectRatio();

			if (tw > totalArea.x) {
				tw = totalArea.x;
				th = tw / pp.aspectRatio;
			}

			if (th > totalArea.y) {
				th = totalArea.y;
				tw = th * pp.aspectRatio;
			}

			// do the width again in case the height calculation made it too wide
			if (tw > totalArea.x) {
				tw = totalArea.x;
				th = tw / pp.aspectRatio;
			}
			piecemealArea += tw * th;
			pp.packSize	  = t_myVector2(int(tw), int(th));
			pp.panelIndex = ind;
			thePackages.push_back(pp);
			ind++;
		}

		if (thePackages.empty()) {
			DS_LOG_WARNING("Bin pack failed with no packages!");
			return false;
		}

		// be sure padding gets added
		PanelPackage::adjustSizes(thePackages, 1.0f, padding);

		while (piecemealArea < totalAreaAmount * 0.75) {
			piecemealArea = PanelPackage::adjustSizes(thePackages, 1.1f, padding);
		}

		while (piecemealArea > totalAreaAmount * 0.9f) {
			piecemealArea = PanelPackage::adjustSizes(thePackages, 0.95f, padding);
		}


		std::vector<PanelPackage> outputPackages;
		bool					  isFine = false;
		t_algoMaxRects			  packs;

		for (int i = 0; i < 100; i++) {
			isFine = false;
			std::vector<t_myVector2> sizes;
			for (auto it = thePackages.begin(); it < thePackages.end(); ++it) {
				sizes.push_back((*it).packSize);
			}

			auto returny = packs.pack(sizes, t_myVector2(int(totalArea.x), int(totalArea.y)), isFine);
			if (isFine) {

				for (auto it = returny.begin(); it != returny.end(); ++it) {
					int xx	   = (*it).second.x;
					int yy	   = (*it).second.y;
					int wid	   = (*it).first.x;
					int hei	   = (*it).first.y;
					int right  = xx + wid;
					int bottom = yy + hei;

					for (auto it = thePackages.begin(); it < thePackages.end(); ++it) {
						if ((*it).packSize.x == wid && (*it).packSize.y == hei) {
							(*it).outputRect = ci::Rectf(float(xx), float(yy), float(right), float(bottom));
							outputPackages.push_back((*it));
							thePackages.erase(it);
							break;
						}
					}
				}
				break;
			} else {
				piecemealArea = PanelPackage::adjustSizes(thePackages, 0.95f, padding);
			}
		}

		if (outputPackages.empty()) {
			DS_LOG_WARNING("Panel Layouts couldn't determine a layout!");
			return false;
		}

		float farthestRight	 = 0.0f;
		float farthestBottom = 0.0f;
		for (auto it = outputPackages.begin(); it < outputPackages.end(); ++it) {
			ci::vec2 br	   = (*it).outputRect.getLowerRight();
			farthestRight  = std::max(br.x, farthestRight);
			farthestBottom = std::max(br.y, farthestBottom);
		}

		float		offsetX	   = (totalArea.x - farthestRight) / 2.0f;
		float		offsetY	   = (totalArea.y - farthestBottom) / 2.0f;
		float		delayey	   = 0.0f;
		const float deltaDelay = animationDuration / float(outputPackages.size());
		for (auto it = outputPackages.begin(); it < outputPackages.end(); ++it) {
			ci::Rectf recty		  = (*it).outputRect;
			auto	  tmv		  = panels[(*it).panelIndex];
			ci::vec3  destination = ci::vec3(recty.getUpperLeft().x + offsetX + totalAreaRect.x1,
											 recty.getUpperLeft().y + offsetY + totalAreaRect.y1, 0.0f);
			float	  destWidth	  = recty.getWidth() - padding;
			if (animationDuration > 0.0f) {
				tmv->tweenStarted();
				tmv->tweenPosition(destination, animationDuration, delayey, ci::EaseInOutQuad(),
								   [tmv] { tmv->tweenEnded(); });
				tmv->animateWidthTo(destWidth / tmv->getScale().x);
			} else {
				tmv->setPosition(destination);
				tmv->setViewerWidth(destWidth / tmv->getScale().x);
			}
		}

		return true;
	}

	bool PanelLayouts::rowPack(std::vector<ds::ui::BasePanel*> panels, const ci::Rectf& totalAreaRect, float padding,
							   float animationDuration, int numRows) {
		std::vector<PanelPackage> thePackages;
		int						  ind = 0;

		if (totalAreaRect.getWidth() < 1.0f) {
			DS_LOG_WARNING("PanelLayouts:rowPack: too small of an area to fit stuff into!");
			return false;
		}

		if (panels.empty()) {
			DS_LOG_VERBOSE(3, "RowPack: no panels set to arrange");
		}

		if (panels.size() == 1) numRows = 1;
		numRows = std::max(1, numRows);

		ci::vec2 totalArea = ci::vec2(totalAreaRect.getWidth(), totalAreaRect.getHeight());

		for (auto it : panels) {
			PanelPackage pp;
			// a = w / h
			// ah = w
			// h = w / a
			pp.aspectRatio = it->getContentAspectRatio();
			if (pp.aspectRatio == 0.0f) pp.aspectRatio = 1.0f;
			pp.size		  = ci::vec2(100.0f, 100.0f / pp.aspectRatio);
			pp.panelIndex = ind;
			thePackages.push_back(pp);
			ind++;
		}

		if (thePackages.empty()) {
			DS_LOG_WARNING("Didn't find any valid panels to rowPack");
			return false;
		}

		float rowHeight = totalAreaRect.getHeight() / float(numRows);
		if (numRows > 1) {
			rowHeight -= float(numRows - 1) * padding;
		}

		// a = w / h
		// w = ah
		// h = w / a

		float xp		 = 0.0f;
		float yp		 = 0.0f;
		float totalWidth = 0.0f;

		/// first, find out the total width if we were to put everything in one big row
		for (const auto& it : thePackages) {
			totalWidth += it.aspectRatio * rowHeight + padding;
		}

		/// then divide that width by the number of rows
		float widthPerRow = totalWidth / float(numRows);

		/// then group the panels into the number of rows
		std::vector<float>						 rowWidths;
		std::map<int, std::vector<PanelPackage>> theRows;
		int										 curRow = 0;
		for (const auto& it : thePackages) {
			float thisW = it.aspectRatio * rowHeight;
			if (xp > widthPerRow && theRows.size() < numRows && it.panelIndex != thePackages.back().panelIndex) {
				rowWidths.emplace_back(xp);
				xp = 0.0f;
				curRow++;
			}
			theRows[curRow].emplace_back(it);
			xp += thisW + padding;
		}

		if (rowWidths.size() < theRows.size()) {
			rowWidths.emplace_back(xp);
		} else {
			rowWidths.back() += xp;
		}

		if (rowWidths.size() != theRows.size()) {
			DS_LOG_WARNING("Houston we have a big problemo");
			return false;
		}

		float theBot = totalAreaRect.y1;

		/// then determine where each panel will go
		for (int i = 0; i < theRows.size(); i++) {
			xp					= 0.0f;
			float thisRowHeight = rowHeight;
			if (rowWidths[i] > totalArea.x) {
				float thisAsp	= (rowWidths[i] - float(theRows[i].size()) * padding) / rowHeight;
				float destWidth = totalArea.x - float(theRows[i].size()) * padding;
				thisRowHeight	= destWidth / thisAsp;
			}

			for (auto& it : theRows[i]) {
				ci::vec2 destination = ci::vec2(xp + totalAreaRect.x1, yp + totalAreaRect.y1);

				theBot = std::max(destination.y + thisRowHeight, theBot);

				float destWidth = thisRowHeight * it.aspectRatio;
				it.size			= ci::vec2(destWidth, thisRowHeight);
				it.position		= destination;

				xp += destWidth + padding;
			}

			yp += thisRowHeight + padding;
		}

		// center vertically
		float deltaDif = (totalAreaRect.getY2() - theBot) / 2.0f;


		for (const auto& row : theRows) {
			for (const auto& it : row.second) {
				auto	 tmv		 = panels[it.panelIndex];
				ci::vec3 destination = ci::vec3(it.position.x, it.position.y + deltaDif, 0.0f);
				float	 destWidth	 = it.size.x;
				if (animationDuration > 0.0f) {
					tmv->tweenStarted();
					tmv->tweenPosition(destination, animationDuration, 0.0f, ci::EaseInOutQuad(),
									   [tmv] { tmv->tweenEnded(); });
					tmv->animateWidthTo(destWidth / tmv->getScale().x);
				} else {
					tmv->setPosition(destination);
					tmv->setViewerWidth(destWidth / tmv->getScale().x);
				}
			}
		}

		return true;
	}

}} // namespace ds::ui