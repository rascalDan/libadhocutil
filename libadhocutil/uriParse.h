#ifndef ADHOCUTIL_URIPARSE_H
#define ADHOCUTIL_URIPARSE_H

#include "visibility.h"
#include "exception.h"
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <string>
#include <map>

namespace AdHoc {
	class DLL_PUBLIC Uri {
		public:
			Uri(const std::string &);

			std::string scheme;
			boost::optional<std::string> username;
			boost::optional<std::string> password;
			std::string host;
			boost::optional<uint16_t> port;
			boost::optional<boost::filesystem::path> path;
			std::multimap<std::string, std::string> query;
			boost::optional<std::string> fragment;
	};

	class DLL_PUBLIC InvalidUri : public Exception<std::invalid_argument> {
		public:
			InvalidUri(const std::string & err, const std::string & uri);

			std::string message() const throw() override;

			const std::string err;
			const std::string uri;
	};
}

#endif

