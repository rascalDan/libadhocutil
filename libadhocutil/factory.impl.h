#ifndef ADHOCUTIL_FACTORY_IMPL_H
#define ADHOCUTIL_FACTORY_IMPL_H

#include "factory.h"
#include "plugins.impl.h"

namespace AdHoc {
	template <typename Base, typename ... Params>
	Factory<Base, Params ...>::~Factory() = default;

	template <typename Base, typename ... Params>
	const Factory<Base, Params...> *
	Factory<Base, Params...>::get(const std::string & name)
	{
		return PluginManager::getDefault()->get<Factory>(name)->implementation();
	}

	template <typename Base, typename ... Params>
	Base *
	Factory<Base, Params...>::create(const std::string & name, const Params & ... p)
	{
		return get(name)->create(p...);
	}
}

#define INSTANIATEFACTORY(Base, ...) \
	template class AdHoc::Factory<Base, __VA_ARGS__>; \
	typedef AdHoc::Factory<Base, __VA_ARGS__> FactoryType; \
	INSTANIATEPLUGINOF(FactoryType)

#endif

