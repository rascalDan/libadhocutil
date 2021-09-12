#include "uriParse.h"
#include "compileTimeFormatter.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <utility>

namespace AdHoc {
	Uri::Uri(const std::string & uri)
	{
		auto is_scheme_char = [](int c) {
			return (!std::isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
		};
		auto endor = [](std::string_view haystack, auto needle) {
			if (const auto n = haystack.find_first_of(needle); n != std::string_view::npos) {
				return n;
			}
			return haystack.length();
		};
		auto parsePort = [&uri, this](const std::string_view in) {
			if (in.empty()) {
				throw InvalidUri("Invalid format; no port after :", uri);
			}
			port.emplace();
			if (const auto rc = std::from_chars(in.begin(), in.end(), *port);
					rc.ptr != in.end() || rc.ec != std::errc {}) {
				throw InvalidUri("Invalid port", uri);
			}
		};

		std::string_view curstr = uri;
		if (auto colon = curstr.find(':'); colon == std::string_view::npos) {
			throw InvalidUri("Scheme marker not found", uri);
		}
		else {
			if (!std::all_of(curstr.begin(), curstr.begin() + colon, is_scheme_char)) {
				throw InvalidUri("Invalid format; no scheme end", uri);
			}
			scheme = curstr.substr(0, colon);
			boost::algorithm::to_lower(scheme);
			curstr.remove_prefix(colon + 1);
		}

		if (!curstr.starts_with("//")) {
			throw InvalidUri("Invalid format; // not where expected", uri);
		}
		curstr.remove_prefix(2);

		if (const auto n = curstr.find_first_of("@/"); n != std::string_view::npos && curstr[n] == '@') {
			if (const auto colon = curstr.find_first_of("@:"); curstr[colon] == ':') {
				username = curstr.substr(0, colon);
				password = curstr.substr(colon + 1, n - colon - 1);
			}
			else {
				username = curstr.substr(0, n);
			}
			curstr.remove_prefix(n + 1);
		}

		if (curstr.empty()) {
			throw InvalidUri("Invalid format", uri);
		}
		if (curstr.starts_with('[')) {
			if (const auto closeb = curstr.find(']'); closeb == std::string_view::npos) {
				throw InvalidUri("IPv6 address not terminated", uri);
			}
			else {
				host = curstr.substr(0, closeb + 1);
				curstr.remove_prefix(closeb + 1);
			}
		}
		else {
			if (const auto hostend = curstr.find_first_of(":/"); hostend == std::string_view::npos) {
				host = curstr;
				return;
			}
			else {
				if (hostend == 0) {
					throw InvalidUri("Host cannot be blank", uri);
				}
				host = curstr.substr(0, hostend);
				boost::algorithm::to_lower(host);
				curstr.remove_prefix(hostend);
			}
		}
		boost::algorithm::to_lower(host);

		if (curstr.empty()) {
			return;
		}
		if (curstr.starts_with(':')) {
			curstr.remove_prefix(1);
			if (curstr.empty()) {
				throw InvalidUri("Invalid format; no port after :", uri);
			}
			if (const auto portend = curstr.find('/'); portend == std::string_view::npos) {
				parsePort(curstr);
				return;
			}
			else {
				parsePort(curstr.substr(0, portend));
				curstr.remove_prefix(portend);
			}
		}
		curstr.remove_prefix(1);

		if (const auto pathend = curstr.find_first_of("#?"); pathend == std::string_view::npos) {
			path = curstr;
			return;
		}
		else {
			path = curstr.substr(0, pathend);
			curstr.remove_prefix(pathend);
		}

		if (curstr.starts_with('?')) {
			curstr.remove_prefix(1);
			auto params = curstr.substr(0, endor(curstr, '#'));
			while (!params.empty()) {
				const auto pair = params.substr(0, endor(params, '&'));
				if (const auto eq = pair.find('='); eq != std::string_view::npos) {
					query.emplace(pair.substr(0, eq), pair.substr(eq + 1));
				}
				else {
					query.emplace(pair, std::string {});
				}
				params.remove_prefix(pair.length());
				if (!params.empty()) {
					params.remove_prefix(1);
				}
			}
		}

		if (curstr.starts_with('#')) {
			curstr.remove_prefix(1);
			fragment = curstr;
		}
	}

	InvalidUri::InvalidUri(std::string e, std::string u) :
		Exception<std::invalid_argument>(std::string()), err(std::move(e)), uri(std::move(u))
	{
	}

	AdHocFormatter(InvalidUriMsg, "InvalidUri (%?) parsing [%?]");

	std::string
	InvalidUri::message() const noexcept
	{
		return InvalidUriMsg::get(err, uri);
	}
}
