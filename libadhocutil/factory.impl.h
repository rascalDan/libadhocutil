#ifndef ADHOCUTIL_FACTORY_IMPL_H
#define ADHOCUTIL_FACTORY_IMPL_H

#include "factory.h" // IWYU pragma: export
#include "plugins.impl.h" // IWYU pragma: export

namespace AdHoc {
	template<typename Base, typename... Params>
	std::shared_ptr<const Factory<Base, Params...>>
	Factory<Base, Params...>::get(const std::string_view & name)
	{
		return PluginManager::getDefault()->get<Factory>(name)->implementation();
	}

	template<typename Base, typename... Params>
	std::shared_ptr<Base>
	Factory<Base, Params...>::createNew(const std::string_view & name, const Params &... p)
	{
		return get(name)->create(p...);
	}
}

#define INSTANTIATEVOIDFACTORY(Base) \
	template class AdHoc::Factory<Base>; \
	INSTANTIATEPLUGINOF(AdHoc::Factory<Base>)

#define INSTANTIATEFACTORY(Base, ...) \
	template class AdHoc::Factory<Base, __VA_ARGS__>; \
	INSTANTIATEPLUGINOF(AdHoc::Factory<Base, __VA_ARGS__>)

#endif
