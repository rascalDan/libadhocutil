#ifndef ADHOCUTIL_GLOBALSTATIC_IMPL_H
#define ADHOCUTIL_GLOBALSTATIC_IMPL_H

#include "globalStatic.h"

namespace AdHoc {
	template<typename Object>
	Object * GlobalStatic<Object>::get()
	{
		return instance().get();
	}

	template<typename Object>
	void GlobalStatic<Object>::createObject()
	{
		instance() = std::make_unique<Object>();
	}

	template<typename Object>
	void GlobalStatic<Object>::deleteObject()
	{
		instance().reset();
	}

	template<typename Object>
	typename GlobalStatic<Object>::Ptr & GlobalStatic<Object>::instance()
	{
		static Ptr _instance;
		return _instance;
	}
};

#endif

