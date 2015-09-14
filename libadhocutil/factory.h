#ifndef ADHOCUTIL_FACTORY_H
#define ADHOCUTIL_FACTORY_H

#include "plugins.h"

namespace AdHoc {
	template <typename Base, typename ... Params>
	class Factory {
		public:
			virtual ~Factory() = 0;

			virtual Base * create(const Params & ...) const = 0;

			template <typename Impl, typename _ = Factory<Base, Params...>>
			class For : public _
			{
				public:
					Base * create(const Params & ... p) const override
					{
						return new Impl(p...);
					}
			};

			static const Factory * get(const std::string & name);
			static Base * create(const std::string & name, const Params & ... p);
	};
}

#define NAMEDFACTORY(Name, Implementation, BaseFactory) \
	NAMEDPLUGIN(Name, BaseFactory::For<Implementation>, BaseFactory)

#define FACTORY(Implementation, BaseFactory) \
	NAMEDFACTORY(#Implementation, Implementation, BaseFactory)

#endif

