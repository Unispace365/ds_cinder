#pragma once
#ifndef DS_UTIL_NOTIFIER_H
#define DS_UTIL_NOTIFIER_H

/* A much more useful version of the notifier.
 */

#include <map>
#include <functional>

namespace ds {

template <typename T>
class Notifier
{
  public:
    Notifier();

    void clear();

    /* 
     * addListener
     * /brief adds function func referenced by id. id is usually the this pointer of a class.
     */
    void addListener(void *id, const std::function<void(const T *)> &func);
    
    /* 
     * removeListener
     * /brief removes function referenced by id.
     */
    void removeListener(void *id);
    void removeAllListeners();

    
    /* 
     * notify
     * /brief passes a pointer of type T to all listeners.
     */
    void notify( const T *v = nullptr );
  private:
    std::map<void *, std::function<void(const T *)>> mFunctions;
};

template <typename T>
Notifier<T>::Notifier()
{

}

template <typename T>
void Notifier<T>::clear()
{
  mFunctions.clear();
}

template <typename T>
void Notifier<T>::addListener( void *id, const std::function<void(const T *)> &func )
{
  try {
    mFunctions[id] = func;
  } catch (std::exception &) {
  }
}


template <typename T>
void Notifier<T>::removeListener( void *id )
{
  auto found = mFunctions.find(id);
  if (found != mFunctions.end()) {
    mFunctions.erase(found);
  }
}

template <typename T>
void Notifier<T>::removeAllListeners()
{
  mFunction.clear();
}

template <typename T>
void Notifier<T>::notify( const T *v /*= nullptr */ )
{
  for (auto it = mFunctions.begin(), it2 = mFunctions.end(); it != it2; ++it) {
    if (it->second)
      (it->second)(v);
  }
}

} // namespace ds

#endif
