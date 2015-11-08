#ifndef ADHOCUTIL_EXCEPTION_H
#define ADHOCUTIL_EXCEPTION_H

#include <exception>
#include <string>
#include <boost/optional.hpp>

namespace AdHoc {
	template <typename BaseException>
	class Exception : public BaseException {
		public:
			inline const char * what() const throw() override
			{
				if (!msg) {
					msg = message();
				}
				return msg->c_str();
			}

		private:
			virtual std::string message() const throw() = 0;
			mutable boost::optional<std::string> msg;
	};
	typedef Exception<std::exception> StdException;
}

#endif

