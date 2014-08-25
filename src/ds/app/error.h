#pragma once
#ifndef DS_APP_ERROR_H_
#define DS_APP_ERROR_H_

#include <stdint.h>
#include <vector>
#include <ds/app/engine/engine_service.h>
#include <ds/app/event.h>
#include <ds/app/event_client.h>

namespace ds {
class Engine;
extern const std::string&	ERROR_CHANNEL;

/********************************************
 * FRAMEWORK ERROR MECHANISM.
 * The system stores a global list of all
 * current errors. To use, clients will want
 * to either add an error, remove one, or
 * hear about changes in the list.
 *
 * To add, on SpriteEngine, do this:
 *		engine.getChannel(ds::ERROR_CHANNEL).notify(AddErrorEvent(ds::ErrorRef()));
 * To remove, on SpriteEngine, do this:
 *		engine.getChannel(ds::ERROR_CHANNEL).notify(RemoveErrorEvent(error_id)));
 * To listen for changes, create an EventClient and pass it the notifier.
 * Note that the first time you register, you immediately receive an
 * ErrorsChangedEvent, letting you do any initialization.
 *		engine.getChannel(ds::ERROR_CHANNEL)
 ********************************************/

/********************************************
 * MODEL
 * Basic error reporting model.
 ********************************************/
/**
 * \class ds::ErrorRef
 * \brief A single error. Errors are identified by
 * a unique ID, only one of which can be in the master
 * error list at a time.
 */
class ErrorRef {
public:
	// Errors are uniquely identified
	static int32_t			getNextId();
	ErrorRef();
	ErrorRef(const int32_t);
	ErrorRef(const int32_t, const std::wstring &name, const std::wstring &descr);

	bool					empty() const;
	void					clear();

	int32_t					getId() const;
	const std::wstring&		getName() const;
	const std::wstring&		getDescription() const;

private:
	ErrorRef&				setId(const int32_t);
	ErrorRef&				setName(const std::wstring&);
	ErrorRef&				setDescription(const std::wstring&);

	class Data;
	std::shared_ptr<Data>	mData;
};

/**
 * \class ds::ErrorList
 */
class ErrorList {
public:
	ErrorList();

	void					operator=(const ErrorList&);

	bool					empty() const;
	void					clear();
	void					swap(ErrorList&);

	size_t					size() const;
	const ErrorRef&			getAt(const size_t) const;
	const ErrorRef&			findById(const int32_t) const;

	void					push_back(const ErrorRef&);
	void					removeById(const int32_t);

private:
	std::vector<ErrorRef>	mAll;
};

/********************************************
 * EVENTS
 * Handle communication about error changes.
 * All events sent and received over the
 * engine.getChannel(ERROR_CHANNEL) notifier.
 ********************************************/
/**
 * \class ds::AddErrorEvent
 * \brief Add a new error to the global error list.
 */
class AddErrorEvent : public ds::Event {
public:
	static int				WHAT();
	AddErrorEvent(const ErrorRef&);
	const ErrorRef&			mError;
};

/**
 * \class ds::RemoveErrorEvent
 * \brief Remove the error with the supplied ID.
 */
class RemoveErrorEvent : public ds::Event {
public:
	static int				WHAT();
	RemoveErrorEvent(const int32_t);
	const int32_t			mErrorId;
};

/**
 * \class ds::ErrorsChangedEvent
 * \brief The global error list has changed.
 */
class ErrorsChangedEvent : public ds::Event {
public:
	static int				WHAT();
	ErrorsChangedEvent(const ErrorList&);
	const ErrorList&		mErrors;
};

/**
 * \class ds::ToggleTestErrorEvent
 * \brief For debugging, turn a test error on or off.
 */
class ToggleTestErrorEvent : public ds::Event {
public:
	static int				WHAT();
	ToggleTestErrorEvent();
};

/********************************************
 * SERVICE
 * Manage the global error list.
 ********************************************/
class ErrorService : public ds::EngineService {
public:
	ErrorService(Engine&);

	virtual void		stop();

private:
	void				onEvent(const ds::Event&);

	ds::EventClient*	mEventClient;
	ErrorList			mErrors;
	ErrorsChangedEvent	mOnAddEvent;
};

} // namespace ds

#endif