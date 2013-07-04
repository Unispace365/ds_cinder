#pragma once
#ifndef DS_PARAMS_ENGINEINITPARAMS_H_
#define DS_PARAMS_ENGINEINITPARAMS_H_

namespace ds
{
class EventNotifier;

/**
 * \class ds::EngineInitParams
 * \brief A small convenience class used to get parameters
 * into the engine. This only exists to make it easier to
 * change the engine constructor args, which get passed
 * around between multiple classes.
 */
class EngineInitParams
{
public:
	EngineInitParams(EventNotifier&);

	EventNotifier&			mNotifier;

private:
	EngineInitParams(const EngineInitParams&);
	EngineInitParams&		operator=(const EngineInitParams&);
};

} // namespace ds

#endif // DS_PARAMS_ENGINEINITPARAMS_H_
