#ifndef ADHOCUTIL_URIPARSE_H
#define ADHOCUTIL_URIPARSE_H

#include "visibility.h"
#include "exception.h"
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <string>
#include <map>

namespace AdHoc {
	/**
	 * Class representing a broken down URI.
	 */
	class DLL_PUBLIC Uri {
		public:
			/// Constructor accepting a URI to parse.
			/// @param uri the URI to parse.
			Uri(const std::string & uri);

			/// The scheme.
			std::string scheme;
			/// The optional username.
			boost::optional<std::string> username;
			/// The optional password.
			boost::optional<std::string> password;
			/// The host.
			std::string host;
			/// The optional port.
			boost::optional<uint16_t> port;
			/// The optional path.
			boost::optional<boost::filesystem::path> path;
			/// The parsed components of the query string.
			std::multimap<std::string, std::string> query;
			/// The optional fragment.
			boost::optional<std::string> fragment;
	};

	/**
	 * Exception representing a failure attempting to parse a URI.
	 */
	class DLL_PUBLIC InvalidUri : public Exception<std::invalid_argument> {
		public:
			/// Constructor accepting what went wrong and the URI being parsed.
			InvalidUri(const std::string & err, const std::string & uri);

			std::string message() const throw() override;

			/// The parse error.
			const std::string err;
			/// The URI being parsed when the error occurred.
			const std::string uri;
	};
}

#endif

