#ifndef ADHOCUTIL_FACTORY_H
#define ADHOCUTIL_FACTORY_H

#include "plugins.h"
#include "visibility.h"

namespace AdHoc {
	/**
	 * Base class for factories creating instances of Base.
	 */
	template <typename Base, typename ... Params>
	class DLL_PUBLIC Factory {
		public:
			virtual ~Factory() = 0;

			/**
			 * Create a new instance of Base, overridden in a subclass to construct a new specific class.
			 * @param p The parameters passed to Impl constructor.
			 */
			virtual Base * create(const Params & ... p) const = 0;

			/**
			 * A factory for a concrete implementation of Base
			 */
			template <typename Impl, typename _ = Factory<Base, Params...>>
			class DLL_PUBLIC For : public _
			{
				public:
					/**
					 * Create a new instance of Base implemented in Impl.
					 * @param p The parameters passed to Impl constructor.
					 */
					Base * create(const Params & ... p) const override
					{
						return new Impl(p...);
					}
			};

			/**
			 * Helper to get the factory for a specific implementation.
			 * @param name The name of the implementation.
			 */
			static const Factory * get(const std::string & name);
			/**
			 * Helper to create a new instance from a specific factory.
			 * @param name The name of the implementation.
			 * @param p The parameters to pass to the constructor.
			 */
			static Base * createNew(const std::string & name, const Params & ... p);
	};
}

#define NAMEDFACTORY(Name, Implementation, BaseFactory) \
	NAMEDPLUGIN(Name, BaseFactory::For<Implementation>, BaseFactory)

#define FACTORY(Implementation, BaseFactory) \
	NAMEDFACTORY(#Implementation, Implementation, BaseFactory)

#endif

