#ifndef ADHOCUTIL_GLOBALSTATIC_IMPL_H
#define ADHOCUTIL_GLOBALSTATIC_IMPL_H

#include "globalStatic.h"

namespace AdHoc {
	template<typename Object>
	Object * GlobalStatic<Object>::get()
	{
		return instance();
	}

	template<typename Object>
	void GlobalStatic<Object>::createObject()
	{
		instance() = new Object();
	}

	template<typename Object>
	void GlobalStatic<Object>::deleteObject()
	{
		auto & i = instance();
		delete i;
		i = nullptr;
	}

	template<typename Object>
	Object * & GlobalStatic<Object>::instance()
	{
		static Object * _instance = nullptr;
		return _instance;
	}
};

#endif

