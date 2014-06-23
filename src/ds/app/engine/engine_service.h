#pragma once
#ifndef DS_APP_ENGINE_ENGINESERVICE_H_
#define DS_APP_ENGINE_ENGINESERVICE_H_

namespace ds {

/**
 * \class ds::EngineService
 * Abstract superclass for generic classes that live in and are scoped to the engine.
 */
class EngineService {
public:
	virtual ~EngineService()				{ }
	virtual void				start()		{ }

protected:
	EngineService()							{ }
};

} // namespace ds

#endif