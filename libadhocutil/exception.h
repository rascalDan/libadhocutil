#ifndef ADHOCUTIL_EXCEPTION_H
#define ADHOCUTIL_EXCEPTION_H

#include <exception>
#include <string>
#include <boost/optional.hpp>

namespace AdHoc {
	/// Helper class for lazy creation of exception message text.
	template <typename BaseException>
	class Exception : public BaseException {
		public:
			/// Override of std::exception::what() to create text as required.
			inline const char * what() const throw() override
			{
				if (!msg) {
					msg = message();
				}
				return msg->c_str();
			}

		private:
			/// Message text provider.
			virtual std::string message() const throw() = 0;
			mutable boost::optional<std::string> msg;
	};
	typedef Exception<std::exception> StdException;
}

#endif

