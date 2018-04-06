#ifndef ADHOCUTIL_EXCEPTION_H
#define ADHOCUTIL_EXCEPTION_H

#include <exception>
#include <string>
#include <optional>

namespace AdHoc {
	/// Helper class for lazy creation of exception message text.
	template <typename BaseException>
	class Exception : public BaseException {
		public:
			/// Wrapper constructor to pass to BaseException
			//@param t parameters to pass.
			template <typename ... T>
			Exception(const T & ... t) : BaseException(t...) { }

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
			mutable std::optional<std::string> msg;
	};
	typedef Exception<std::exception> StdException;
}

#endif

