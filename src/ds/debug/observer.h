#ifndef _DS_OBSERVER_H_
#define _DS_OBSERVER_H_

#include "AntTweakBar.h"
#include "ds\util\bit_mask.h"
#include "ds\util\string_util.h"
#include "cinder\Color.h"
#include "logger.h"

// A handy typedef for variables that need to be observed
// via setters/getters. Example of these kind of variables
// is position of Sprites. If you access them directly by
// mPosition and attempt to change it, Sprite's position
// won't change. You have to call Sprite::setPostion
typedef void (TW_CALL *ObserverSetter)(const void *, void *);
// Use this macro to retrieve a getter defined by OBSERVER_SETTER
#define OBSERVER_SETTER_NAME(variable) (observer_setter_##variable)
// Use this inside a class to define Observer setters
#define OBSERVER_SETTER(variable) \
	static void TW_CALL observer_setter_##variable(const void * value, void * clientData)

// A handy typedef for variables that need to be observed
// via setters/getters. Example of these kind of variables
// is position of Sprites.
typedef void (TW_CALL *ObserverGetter)(void *, void *);
// Use this macro to retrieve a getter defined by OBSERVER_GETTER
#define OBSERVER_GETTER_NAME(variable) (observer_getter_##variable)
// Use this inside a class to define Observer getters
#define OBSERVER_GETTER(variable) \
	static void TW_CALL observer_getter_##variable(void * value, void * clientData)

namespace ds {

	extern const ds::BitMask	OBSERVER_LOG;

	class Observer {
	public:
		Observer();
		~Observer();

		// Supplies observer with a unique hash as their name
		// Classes should override this to supply unique names
		// for their Observers. For example Sprites override
		// this and return their mId.
		virtual std::string	observerHashGenerator() const;

		// Initializes the Observer.
		// Sprites (or classes) can override this
		// to add more variables to the Tweaker GUI
		// via Observer::observe API
		virtual void		installObserver();

	private:
		// Pointer to an AntTweakBar Object
		TwBar*				mTweaker;

	public:
		// Retrieves the original Ant Tweak Bar pointer.
		// You can use this to work with AntTweakBar API inside your sprite
		// **WARNING** If you add variables through the pointer, they
		// won't be enabled in live mode. use Observer::observe API instead.
		TwBar*				getObserver();

	public:
		enum ACCESS_TYPE {
			READ_ONLY,
			READ_WRITE
		};
	
	public:
		// This can be extended to support more observe-able types.
		template<typename T>
		void				observe(std::string name, T* var, ACCESS_TYPE type = READ_WRITE, ObserverSetter setter = nullptr, ObserverGetter getter = nullptr, std::string AntTweakBarParamSyntaxDef = "") {
			DS_LOG_WARNING_M("The typename you are trying to observe does not have a predefined observer."
				<< std::endl
				<< "passed name is: " << name
				<< std::endl
				<< "C++ type id is: " << typeid(T).name()
				<< std::endl
				<< "You can use Sprite::getPointer() and AntTweakBar's API to register observers for this type."
				<< std::endl,
				ds::OBSERVER_LOG);
		};
		template<typename T>
		void				observe(std::string name, T* var, T min, T max, ACCESS_TYPE type = READ_WRITE, ObserverSetter setter = nullptr, ObserverGetter getter = nullptr) {
			DS_LOG_WARNING_M("The typename you are trying to observe does not have a predefined observer."
				<< std::endl
				<< "passed name is: " << name
				<< std::endl
				<< "C++ type id is: " << typeid(T).name()
				<< std::endl
				<< "You can use Sprite::getPointer() and AntTweakBar's API to register observers for this type."
				<< std::endl,
				ds::OBSERVER_LOG);
		};
		template<typename T>
		void				observe(std::string name, T* var, T step, T min, T max, ACCESS_TYPE type = READ_WRITE, ObserverSetter setter = nullptr, ObserverGetter getter = nullptr) {
			DS_LOG_WARNING_M("The typename you are trying to observe does not have a predefined observer."
				<< std::endl
				<< "passed name is: " << name
				<< std::endl
				<< "C++ type id is: " << typeid(T).name()
				<< std::endl
				<< "You can use Sprite::getPointer() and AntTweakBar's API to register observers for this type."
				<< std::endl,
				ds::OBSERVER_LOG);
		};
		template<typename T>
		void				observe(std::string name, T* var, T step, ACCESS_TYPE type = READ_WRITE, ObserverSetter setter = nullptr, ObserverGetter getter = nullptr) {
			DS_LOG_WARNING_M("The typename you are trying to observe does not have a predefined observer."
				<< std::endl
				<< "passed name is: " << name
				<< std::endl
				<< "C++ type id is: " << typeid(T).name()
				<< std::endl
				<< "You can use Sprite::getPointer() and AntTweakBar's API to register observers for this type."
				<< std::endl,
				ds::OBSERVER_LOG);
		};

		//Predefined observers
		// Int32
		template<>
		void 				observe<int>(std::string name, int* var, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			if (type == READ_WRITE)
			{
				TwAddVarRW(getObserver(), name.c_str(), TW_TYPE_INT32, var, AntTweakBarParamSyntaxDef.c_str());
			}
			else if (type == READ_ONLY)
			{
				TwAddVarRO(getObserver(), name.c_str(), TW_TYPE_INT32, var, AntTweakBarParamSyntaxDef.c_str());
			}
		}
		template<>
		void 				observe<int>(std::string name, int* var, int min, int max, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " min="+value_to_string(min)+" "+"max="+value_to_string(max)+" ";
			observe(name, var, type, setter, getter, def);
		}
		template<>
		void 				observe<int>(std::string name, int* var, int step, int min, int max, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " min="+value_to_string(min)+" "+"max="+value_to_string(max)+" step="+value_to_string(step)+" ";
			observe(name, var, type, setter, getter, def);
		}
		template<>
		void 				observe<int>(std::string name, int* var, int step, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " step="+value_to_string(step)+" ";
			observe(name, var, type, setter, getter, def);
		}

		// double
		template<>
		void 				observe<double>(std::string name, double* var, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			if (type == READ_WRITE)
			{
				TwAddVarRW(getObserver(), name.c_str(), TW_TYPE_DOUBLE, var, AntTweakBarParamSyntaxDef.c_str());
			}
			else if (type == READ_ONLY)
			{
				TwAddVarRO(getObserver(), name.c_str(), TW_TYPE_DOUBLE, var, AntTweakBarParamSyntaxDef.c_str());
			}
		}
		template<>
		void 				observe<double>(std::string name, double* var, double min, double max, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " min="+value_to_string(min)+" "+"max="+value_to_string(max)+" ";
			observe(name, var, type, setter, getter, def);
		}
		template<>
		void 				observe<double>(std::string name, double* var, double step, double min, double max, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " min="+value_to_string(min)+" "+"max="+value_to_string(max)+" step="+value_to_string(step)+" ";
			observe(name, var, type, setter, getter, def);
		}
		template<>
		void 				observe<double>(std::string name, double* var, double step, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " step="+value_to_string(step)+" ";
			observe(name, var, type, setter, getter, def);
		}

		// float
		template<>
		void 				observe<float>(std::string name, float* var, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			if (type == READ_WRITE)
			{
				TwAddVarRW(getObserver(), name.c_str(), TW_TYPE_FLOAT, var, AntTweakBarParamSyntaxDef.c_str());
			}
			else if (type == READ_ONLY)
			{
				TwAddVarRO(getObserver(), name.c_str(), TW_TYPE_FLOAT, var, AntTweakBarParamSyntaxDef.c_str());
			}
		}
		template<>
		void 				observe<float>(std::string name, float* var, float min, float max, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " min="+value_to_string(min)+" "+"max="+value_to_string(max)+" ";
			observe(name, var, type, setter, getter, def);
		}
		template<>
		void 				observe<float>(std::string name, float* var, float step, float min, float max, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " min="+value_to_string(min)+" "+"max="+value_to_string(max)+" step="+value_to_string(step)+" ";
			observe(name, var, type, setter, getter, def);
		}
		template<>
		void 				observe<float>(std::string name, float* var, float step, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " step="+value_to_string(step)+" ";
			observe(name, var, type, setter, getter, def);
		}

		// bool
		template<>
		void 				observe<bool>(std::string name, bool* var, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			if (type == READ_WRITE)
			{
				TwAddVarRW(getObserver(), name.c_str(), TW_TYPE_BOOLCPP, var, AntTweakBarParamSyntaxDef.c_str());
			}
			else if (type == READ_ONLY)
			{
				TwAddVarRO(getObserver(), name.c_str(), TW_TYPE_BOOLCPP, var, AntTweakBarParamSyntaxDef.c_str());
			}
		}

		// char
		template<>
		void 				observe<char>(std::string name, char* var, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			if (type == READ_WRITE)
			{
				TwAddVarRW(getObserver(), name.c_str(), TW_TYPE_CHAR, var, AntTweakBarParamSyntaxDef.c_str());
			}
			else if (type == READ_ONLY)
			{
				TwAddVarRO(getObserver(), name.c_str(), TW_TYPE_CHAR, var, AntTweakBarParamSyntaxDef.c_str());
			}
		}

		// Vec3f
		template<>
		void 				observe<ci::Vec3f>(std::string name, ci::Vec3f* var, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			if (type == READ_WRITE)
			{
				TwAddVarRW(getObserver(), name.c_str(), TW_TYPE_DIR3F, var->ptr(), AntTweakBarParamSyntaxDef.c_str());
			}
			else if (type == READ_ONLY)
			{
				TwAddVarRO(getObserver(), name.c_str(), TW_TYPE_DIR3F, var->ptr(), AntTweakBarParamSyntaxDef.c_str());
			}
		}

		// Vec3d
		template<>
		void 				observe<ci::Vec3d>(std::string name, ci::Vec3d* var, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			if (type == READ_WRITE)
			{
				TwAddVarRW(getObserver(), name.c_str(), TW_TYPE_DIR3D, var->ptr(), AntTweakBarParamSyntaxDef.c_str());
			}
			else if (type == READ_ONLY)
			{
				TwAddVarRO(getObserver(), name.c_str(), TW_TYPE_DIR3D, var->ptr(), AntTweakBarParamSyntaxDef.c_str());
			}
		}

		// ci::Color8u
		template<>
		void 				observe<ci::Color8u>(std::string name, ci::Color8u* var, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			if (type == READ_WRITE)
			{
				TwAddVarRW(getObserver(), name.c_str(), TW_TYPE_COLOR3F, var->ptr(), AntTweakBarParamSyntaxDef.c_str());
			}
			else if (type == READ_ONLY)
			{
				TwAddVarRO(getObserver(), name.c_str(), TW_TYPE_COLOR3F, var->ptr(), AntTweakBarParamSyntaxDef.c_str());
			}
		}

		// ci::ColorA8u
		template<>
		void 				observe<ci::ColorA8u>(std::string name, ci::ColorA8u* var, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			if (type == READ_WRITE)
			{
				TwAddVarRW(getObserver(), name.c_str(), TW_TYPE_COLOR4F, var->ptr(), AntTweakBarParamSyntaxDef.c_str());
			}
			else if (type == READ_ONLY)
			{
				TwAddVarRO(getObserver(), name.c_str(), TW_TYPE_COLOR4F, var->ptr(), AntTweakBarParamSyntaxDef.c_str());
			}
		}

		// unsigned int
		template<>
		void 				observe<unsigned int>(std::string name, unsigned int* var, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			if (type == READ_WRITE)
			{
				TwAddVarRW(getObserver(), name.c_str(), TW_TYPE_UINT32, var, AntTweakBarParamSyntaxDef.c_str());
			}
			else if (type == READ_ONLY)
			{
				TwAddVarRO(getObserver(), name.c_str(), TW_TYPE_UINT32, var, AntTweakBarParamSyntaxDef.c_str());
			}
		}
		template<>
		void 				observe<unsigned int>(std::string name, unsigned int* var, unsigned int min, unsigned int max, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " min="+value_to_string(min)+" "+"max="+value_to_string(max)+" ";
			observe(name, var, type, setter, getter, def);
		}
		template<>
		void 				observe<unsigned int>(std::string name, unsigned int* var, unsigned int step, unsigned int min, unsigned int max, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " min="+value_to_string(min)+" "+"max="+value_to_string(max)+" step="+value_to_string(step)+" ";
			observe(name, var, type, setter, getter, def);
		}
		template<>
		void 				observe<unsigned int>(std::string name, unsigned int* var, unsigned int step, ACCESS_TYPE type, ObserverSetter setter, ObserverGetter getter) {
			std::string def = " step="+value_to_string(step)+" ";
			observe(name, var, type, setter, getter, def);
		}

		// User Defined Observers with getters/setter
		template<typename T>
		void				observe(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef = "");
		template<>
		void				observe<bool>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_BOOLCPP, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }
		template<>
		void				observe<int>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_INT32, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }
		template<>
		void				observe<float>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_FLOAT, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }
		template<>
		void				observe<double>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_DOUBLE, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }
		template<>
		void				observe<unsigned int>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_UINT32, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }
		template<>
		void				observe<char>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_CHAR, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }
		template<>
		void				observe<ci::Color>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_COLOR3F, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }
		template<>
		void				observe<ci::Vec3f>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_DIR3F, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }
		template<>
		void				observe<ci::Vec3d>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_DIR3D, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }
		template<>
		void				observe<ci::Vec2f>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_DIR3F, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }
		template<>
		void				observe<ci::Vec2d>(std::string name, ObserverSetter setter, ObserverGetter getter, std::string AntTweakBarParamSyntaxDef) {
			TwAddVarCB(getObserver(), name.c_str(), TW_TYPE_DIR3D, setter, getter, this, AntTweakBarParamSyntaxDef.c_str()); }

	}; //!class Observer

} //!namespace ds

#endif //!_DS_OBSERVER_H_