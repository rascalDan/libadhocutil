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
		delete instance();
		instance() = nullptr;
	}

	template<typename Object>
	typename GlobalStatic<Object>::Ptr & GlobalStatic<Object>::instance()
	{
		static Ptr _instance;
		return _instance;
	}
};

#endif

