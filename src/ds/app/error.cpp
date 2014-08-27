#include "error.h"

#include <ds/app/engine/engine.h>
#include <ds/debug/logger.h>

#pragma warning(disable: 4355)

namespace ds {

namespace {
const std::wstring			EMPTY_WSZ;
const ErrorRef				EMPTY_ERROR;
const std::string			_ERROR_CHANNEL("ds/error");
}

const std::string&			ERROR_CHANNEL(_ERROR_CHANNEL);

/********************************************
 * MODEL
 * Basic error reporting model.
 ********************************************/

/**
 * \class ds::ErrorRef::Data
 */
class ErrorRef::Data {
public:
	Data()						: mId(0) { }

	int32_t						mId;
	std::wstring				mName,
								mDescription;
};

/**
 * \class ds::ErrorRef
 */
int32_t ErrorRef::getNextId() {
	static int32_t	ID = 0;
	return ++ID;
}

ErrorRef::ErrorRef() {
}

bool ErrorRef::empty() const {
	return !mData;
}

void ErrorRef::clear() {
	mData.reset();
}

int32_t ErrorRef::getId() const {
	if (!mData) return 0;
	return mData->mId;
}

const std::wstring& ErrorRef::getName() const {
	if (!mData) return EMPTY_WSZ;
	return mData->mName;
}

const std::wstring& ErrorRef::getDescription() const {
	if (!mData) return EMPTY_WSZ;
	return mData->mDescription;
}

ErrorRef::ErrorRef(const int32_t id) {
	setId(id);
}

ErrorRef::ErrorRef(const int32_t id, const std::wstring &name, const std::wstring &descr) {
	setId(id);
	setName(name);
	setDescription(descr);
}

ErrorRef& ErrorRef::setId(const int32_t id) {
	if (!mData) mData.reset(new Data());
	if (mData) mData->mId = id;
	return *this;
}

ErrorRef& ErrorRef::setName(const std::wstring& s) {
	if (!mData) mData.reset(new Data());
	if (mData) mData->mName = s;
	return *this;
}

ErrorRef& ErrorRef::setDescription(const std::wstring& s) {
	if (!mData) mData.reset(new Data());
	if (mData) mData->mDescription = s;
	return *this;
}

/**
 * \class gm::ErrorList
 */
ErrorList::ErrorList() {
}

void ErrorList::operator=(const ErrorList& o) {
	mAll = o.mAll;
}

bool ErrorList::empty() const {
	return mAll.empty();
}

void ErrorList::clear() {
	mAll.clear();
}

void ErrorList::swap(ErrorList& o) {
	if (this != &o) {
		mAll.swap(o.mAll);
	}
}

size_t ErrorList::size() const {
	return mAll.size();
}

const ErrorRef& ErrorList::getAt(const size_t index) const {
	if (index >= mAll.size()) return EMPTY_ERROR;
	return mAll[index];
}

const ErrorRef& ErrorList::findById(const int32_t id) const {
	for (auto it=mAll.begin(), end=mAll.end(); it!=end; ++it) {
		if (it->getId() == id) return *it;
	}
	return EMPTY_ERROR;
}

void ErrorList::push_back(const ErrorRef &e) {
	if (!e.empty()) mAll.push_back(e);
}

void ErrorList::removeById(const int32_t id) {
	// XXX should remove all with this ID
	for (auto it=mAll.begin(), end=mAll.end(); it!=end; ++it) {
		if (it->getId() == id) {
			mAll.erase(it);
			return;
		}
	}
}

/********************************************
 * EVENTS
 ********************************************/
 /**
 * \class ds::AddErrorEvent
 */
static ds::EventRegistry	ADDE("AddErrorEvent");
int AddErrorEvent::WHAT()	{ return ADDE.mWhat; }

AddErrorEvent::AddErrorEvent(const ErrorRef &e)
		: ds::Event(ADDE.mWhat)
		, mError(e) {
}

/**
 * \class ds::RemoveErrorEvent
 */
static ds::EventRegistry		REME("RemoveErrorEvent");
int RemoveErrorEvent::WHAT()	{ return REME.mWhat; }

RemoveErrorEvent::RemoveErrorEvent(const int32_t id)
		: ds::Event(REME.mWhat)
		, mErrorId(id) {
}

/**
 * \class ds::ErrorsChangedEvent
 */
static ds::EventRegistry		EC("ErrorsChangedEvent");
int ErrorsChangedEvent::WHAT()	{ return EC.mWhat; }

ErrorsChangedEvent::ErrorsChangedEvent(const ErrorList &e)
		: ds::Event(EC.mWhat)
		, mErrors(e) {
}

/**
 * \class ds::ToggleTestErrorEvent
 */
static ds::EventRegistry			TTE("ToggleTestErrorEvent");
int ToggleTestErrorEvent::WHAT()	{ return TTE.mWhat; }

ToggleTestErrorEvent::ToggleTestErrorEvent()
		: ds::Event(TTE.mWhat) {
}

/********************************************
 * SERVICE
 ********************************************/
ErrorService::ErrorService(Engine &e)
		: mEventClient(new EventClient(e.getChannel(ERROR_CHANNEL),
						[this](const ds::Event *e){if (e) onEvent(*e); }))
		, mOnAddEvent(mErrors) {
	e.getChannel(ERROR_CHANNEL).setOnAddListenerFn([this]()->ds::Event*{return &mOnAddEvent;});
}

void ErrorService::stop() {
	delete mEventClient;
	mEventClient = nullptr;
}

void ErrorService::onEvent(const ds::Event &_e) {
	if (!mEventClient) return;

	if (AddErrorEvent::WHAT() == _e.mWhat) {
		const AddErrorEvent&		e((const AddErrorEvent&)_e);
		if (mErrors.findById(e.mError.getId()).empty()) {
			mErrors.push_back(e.mError);
			mEventClient->notify(ErrorsChangedEvent(mErrors));
			DS_LOGW_ERROR("AddErrorEvent id=" << e.mError.getId() << " name=" << e.mError.getName());
		}
	} else if (RemoveErrorEvent::WHAT() == _e.mWhat) {
		const RemoveErrorEvent&		e((const RemoveErrorEvent&)_e);
		mErrors.removeById(e.mErrorId);
		mEventClient->notify(ErrorsChangedEvent(mErrors));
	} else if (ToggleTestErrorEvent::WHAT() == _e.mWhat) {
		static const int32_t		TEST_ID = ErrorRef::getNextId();
		if (mErrors.findById(TEST_ID).empty()) {
			ErrorRef		er(1, L"Something's wrong!", L"And yet I can't figure out what it is.");
			onEvent(AddErrorEvent(er));
		} else {
			onEvent(RemoveErrorEvent(TEST_ID));
		}
	}
}


} // namespace ds
