#pragma once
#ifndef NU_UTIL_NOTIFIER_H
#define NU_UTIL_NOTIFIER_H

/* A much more useful version of the notifier.
 */

#include <map>
#include <functional>

namespace ds2
{
/* \class Notifier
 * \brief A generic message passing system.
 * There are two types of messages: Notifications, where the client
 * will get no response (fire and forget) and requests, where the
 * point is to get a response.
 */
template <typename T>
class Notifier
{
  public:
    Notifier();

    void clear();
    // ? This is identical to clear, probably shouldn't have it
    void removeAllListeners();

    /* Notification mechanism where no response is expected
     */
    void addListener(void *id, const std::function<void(const T *)> &func);
    void removeListener(void *id);

    void notify( const T *v = nullptr );

    /* Request mechanism for requesting data.
     */
    void addRequestListener(void *id, const std::function<void(T&)> &func);
    void removeRequestListener(void *id);

    void request(T&);

  private:
    std::map<void *, std::function<void(const T *)>> mFunctions;
    std::map<void *, std::function<void(T&)>> mRequestFn;
};

template <typename T>
Notifier<T>::Notifier()
{
}

template <typename T>
void Notifier<T>::clear()
{
	mFunctions.clear();
	mRequestFn.clear();
}

template <typename T>
void Notifier<T>::removeAllListeners()
{
  clear();
}

template <typename T>
void Notifier<T>::addListener( void *id, const std::function<void(const T *)> &func )
{
    try
    {
        mFunctions[id] = func;
    }
    catch (std::exception &)
    {
    }
}


template <typename T>
void Notifier<T>::removeListener( void *id )
{
  // This is required, otherwise the app can hit an exception during shutdown
  // if the map is empty but clients still exist.
  if (mFunctions.empty()) return;
  auto found = mFunctions.find(id);
  if ( found != mFunctions.end() )
  {
      mFunctions.erase(found);
  }
}

template <typename T>
void Notifier<T>::notify( const T *v /*= nullptr */ )
{
    for ( auto it = mFunctions.begin(), it2 = mFunctions.end(); it != it2; ++it )
    {
        if ( it->second )
            (it->second)(v);
    }
}


template <typename T>
void Notifier<T>::addRequestListener( void *id, const std::function<void(T&)> &func )
{
    try
    {
        mRequestFn[id] = func;
    }
    catch (std::exception &)
    {
    }
}


template <typename T>
void Notifier<T>::removeRequestListener( void *id )
{
  // This is required, otherwise the app can hit an exception during shutdown
  // if the map is empty but clients still exist.
  if (mRequestFn.empty()) return;
  auto found = mRequestFn.find(id);
  if ( found != mRequestFn.end() )
  {
      mRequestFn.erase(found);
  }
}

template <typename T>
void Notifier<T>::request( T& v )
{
    for ( auto it = mRequestFn.begin(), it2 = mRequestFn.end(); it != it2; ++it )
    {
        if ( it->second )
            (it->second)(v);
    }
}

} // namespace {template_namespace}2

#endif
