#include "app_defs.h"

#include "ds/app/engine/engine.h"

namespace ds {

/**
 * \class ds::RootList
 */
RootList::RootList(const std::vector<int>* roots) {
	if (roots) {
		for (auto it=roots->begin(), end=roots->end(); it!=end; ++it) {
			mRoots.push_back(Root());
			if ((*it) == Engine::CAMERA_PERSP) mRoots.back().mType = Root::kPerspective;
		}
	}
}

bool RootList::empty() const {
	return mRoots.empty();
}

RootList& RootList::ortho() {
	mRoots.push_back(Root());
	mRoots.back().mType = Root::kOrtho;
	return *this;
}

RootList& RootList::persp() {
	mRoots.push_back(Root());
	mRoots.back().mType = Root::kPerspective;
	return *this;
}

RootList& RootList::pickSelect() {
	if (!mRoots.empty()) mRoots.back().mPick = Root::kSelect;
	return *this;
}

RootList& RootList::pickColor() {
	if (!mRoots.empty()) mRoots.back().mPick = Root::kColor;
	return *this;
}

RootList& RootList::perspFov(const float v) {
	if (!mRoots.empty()) mRoots.back().mPersp.mFov = v;
	return *this;
}

RootList& RootList::perspPosition(const ci::Vec3f& pt) {
	if (!mRoots.empty()) mRoots.back().mPersp.mPosition = pt;
	return *this;
}

RootList& RootList::perspTarget(const ci::Vec3f& pt) {
	if (!mRoots.empty()) mRoots.back().mPersp.mTarget = pt;
	return *this;
}

RootList& RootList::perspNear(const float v) {
	if (!mRoots.empty()) mRoots.back().mPersp.mNearPlane = v;
	return *this;
}

RootList& RootList::perspFar(const float v) {
	if (!mRoots.empty()) mRoots.back().mPersp.mFarPlane = v;
	return *this;
}

/**
 * \class ds::RootList::Root
 */
RootList::Root::Root()
		: mType(kOrtho)
		, mPick(kDefault) {
}


} // namespace ds
