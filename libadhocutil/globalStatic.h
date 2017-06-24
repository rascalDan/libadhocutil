#ifndef ADHOCUTIL_GLOBALSTATIC_H
#define ADHOCUTIL_GLOBALSTATIC_H

namespace AdHoc {
	template<typename Object>
	class GlobalStatic {
		public:
			static Object * get();

		private:
			static void createObject() __attribute__((constructor(101)));
			static void deleteObject() __attribute__((destructor(101)));

			inline static Object * & instance();
	};
}

#endif

