#ifndef ADHOCUTIL_GLOBALSTATIC_H
#define ADHOCUTIL_GLOBALSTATIC_H

#include <memory>

namespace AdHoc {
	/**
	 * Wrapper class for initialising/destroying a global static object via
	 * __attribute__ constructor/destructor.
	 */
	template<typename Object> class GlobalStatic {
	public:
		/**
		 * Get the contained object.
		 * @return The object.
		 */
		static Object * get();

	private:
		using Ptr = Object *;
		// cppcheck-suppress unusedPrivateFunction
		static void createObject() __attribute__((constructor(101)));
		// cppcheck-suppress unusedPrivateFunction
		static void deleteObject() __attribute__((destructor(101)));

		inline static Ptr & instance();
	};
}

#endif
