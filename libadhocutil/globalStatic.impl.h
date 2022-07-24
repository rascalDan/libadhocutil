#pragma once

#include "globalStatic.h" // IWYU pragma: export

namespace AdHoc {
	template<typename Object>
	Object *
	GlobalStatic<Object>::get()
	{
		return instance();
	}

	template<typename Object>
	void
	GlobalStatic<Object>::createObject()
	{
		instance() = new Object();
	}

	template<typename Object>
	void
	GlobalStatic<Object>::deleteObject()
	{
		delete instance();
		instance() = nullptr;
	}

	template<typename Object>
	typename GlobalStatic<Object>::Ptr &
	GlobalStatic<Object>::instance()
	{
		static Ptr _instance {};
		return _instance;
	}
}
