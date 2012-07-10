#pragma once
#ifndef DS_UI_VIEW_VIEW_H_
#define DS_UI_VIEW_VIEW_H_

namespace ds {
class Engine;

/**
 * \class ds::View
 */
class View {
public:
	View(Engine&);
	virtual ~View();

protected:
	Engine&			mEngine;
};

} // namespace ds

#endif // DS_UI_VIEW_VIEW_H_