#pragma once
#ifndef UI_TABLE_UTIL_SPRITECACHE_H_
#define UI_TABLE_UTIL_SPRITECACHE_H_

#include <functional>
#include <vector>
#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui{

/** \class SpriteCache
	Utility to manage a collection of sprites.
	When you're done with a sprite, this can keep it around for the next time you need it
	So you don't have to release() sprites and recreate them.
	*/
template <class T>
class SpriteCache
{
public:
	SpriteCache(ds::ui::Sprite& parent,
				const std::function<T*(void)>& allocFn = []()->T*{return new T; });

	/** Answer an existing hidden view, or create one */
	T*						activateNext();

	/** Put a sprite away for reuse later */
	void					deactivate(T&);

	/** Put all sprites away for reuse later */
	void					deactivateAll();

	/** How many active sprites are there? */
	size_t					countActives() const;
	T*						findActive(const std::function<T*(T&)>&);
	void					forEachActive(const std::function<void(T&)>&);
	void					forEachActive(const std::function<void(const T&)>&) const;

private:
	ds::ui::Sprite&			mParent;
	std::vector<T*>			mCache;
	std::function<T*(void)>	mAllocFn;
};

/*  SpriteCache implementation */
template <class T>
SpriteCache<T>::SpriteCache(ds::ui::Sprite& parent,
							const std::function<T*(void)>& allocFn)
							: mParent(parent)
							, mAllocFn(allocFn) {
	mCache.reserve(16);
}

template <class T>
T* SpriteCache<T>::activateNext() {
	try {
		for(auto it = mCache.begin(), end = mCache.end(); it != end; ++it) {
			T*		v = (*it);
			if(v && !v->visible()) {
				v->show();
				return v;
			}
		}
		T*			v = mAllocFn();
		if(!v) return nullptr;
		mParent.addChild(*v);
		mCache.push_back(v);
		return v;
	} catch(std::exception const&) {
	}
	return nullptr;
}

template <class T>
void SpriteCache<T>::deactivate(T& t)
{
	t.hide();
}

template <class T>
void SpriteCache<T>::deactivateAll()
{
	forEachActive([this](T& t){this->deactivate(t); });
}

template <class T>
size_t SpriteCache<T>::countActives() const
{
	size_t      ans = 0;
	for(auto it = mCache.begin(), end = mCache.end(); it != end; ++it) {
		const T*    v = (*it);
		if(v && v->visible()) ++ans;
	}
	return ans;
}

template <class T>
T* SpriteCache<T>::findActive(const std::function<T*(T&)>& fn)
{
	for(auto it = mCache.begin(), end = mCache.end(); it != end; ++it) {
		T*    v = (*it);
		if(v && v->visible()) {
			v = fn(*v);
			if(v) return v;
		}
	}
	return nullptr;
}

template <class T>
void SpriteCache<T>::forEachActive(const std::function<void(T&)>& fn)
{
	for(auto it = mCache.begin(), end = mCache.end(); it != end; ++it) {
		T*    v = (*it);
		if(v && v->visible()) {
			fn(*v);
		}
	}
}

template <class T>
void SpriteCache<T>::forEachActive(const std::function<void(const T&)>& fn) const
{
	for(auto it = mCache.begin(), end = mCache.end(); it != end; ++it) {
		const T*    v = (*it);
		if(v && v->visible()) {
			fn(*v);
		}
	}
}

} // namespace ui
} // namespace ds

#endif // UI_TABLE_UTIL_SPRITECACHE_H_