#pragma once
#ifndef DS_APP_EVENTNOTIFIER_H
#define DS_APP_EVENTNOTIFIER_H

#include <ds/app/event.h>
#include <ds/util/notifier.h>

namespace ds {

/**
 * \class ds::EventNotifier
 * \brief Holder for an event notifier.
 */
class EventNotifier {
public:
	EventNotifier();
	virtual ~EventNotifier();

	void						addListener(void *id, const std::function<void(const ds::Event*)>&);
	void						addRequestListener(void *id, const std::function<void(ds::Event&)>&);
	void						removeListener(void *id);
	void						removeRequestListener(void *id);

	// Send an event to the system, for clients that don't need
	// an EventClient (i.e. don't need to receive events)
	void						notify(const ds::Event&);

	/**
	* Request information from the system.
	* \param requestEvent The event to be sent as a request to the event system
	*/
	void						request(ds::Event& requestEvent);

	/** \brief Set an event that gets fired when a new listener is added.
	 * DANGEROUS: The caller needs to guarantee the T* it's returning is valid
	 * outside the scope of the fn.
	 * \param onAddListenerFunction The function to be called when a new listener has been added 
	 */
	void						setOnAddListenerFn(const std::function<ds::Event*(void)> &onAddListenerFunction);

protected:
	friend class EventClient;

	ds::Notifier<ds::Event>    mEventNotifier;
};

} // namespace ds

#endif // DS_APP_EVENTNOTIFIER_H
