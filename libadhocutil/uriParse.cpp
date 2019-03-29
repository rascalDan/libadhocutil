#include "uriParse.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <utility>
#include "compileTimeFormatter.h"

namespace AdHoc {
	static inline int _is_scheme_char(int c)
	{
		return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
	}

	Uri::Uri(const std::string & uri)
	{
		auto * puri = this;
		const char *curstr;
		int userpass_flag;
		int bracket_flag;

		curstr = uri.c_str();

		const char * tmpstr = ::strchr(curstr, ':');
		if (!tmpstr) {
			throw InvalidUri("Schema marker not found", uri);
		}
		auto len = tmpstr - curstr;
		for (decltype(len) i = 0; i < len; i++) {
			if (!_is_scheme_char(curstr[i])) {
				throw InvalidUri("Invalid format", uri);
			}
		}
		puri->scheme = std::string(curstr, len);
		boost::algorithm::to_lower(puri->scheme);
		tmpstr++;
		curstr = tmpstr;

		for (int i = 0; i < 2; i++) {
			if ('/' != *curstr) {
				throw InvalidUri("Invalid format", uri);
			}
			curstr++;
		}

		userpass_flag = 0;
		tmpstr = curstr;
		while ('\0' != *tmpstr) {
			if ('@' == *tmpstr) {
				userpass_flag = 1;
				break;
			} else if ('/' == *tmpstr) {
				userpass_flag = 0;
				break;
			}
			tmpstr++;
		}

		tmpstr = curstr;
		if (userpass_flag) {
			while ('\0' != *tmpstr && ':' != *tmpstr && '@' != *tmpstr) {
				tmpstr++;
			}
			len = tmpstr - curstr;
			puri->username = std::string(curstr, len);
			curstr = tmpstr;
			if (':' == *curstr) {
				curstr++;
				tmpstr = curstr;
				while ('\0' != *tmpstr && '@' != *tmpstr) {
					tmpstr++;
				}
				len = tmpstr - curstr;
				puri->password = std::string(curstr, len); 
				curstr = tmpstr;
			}
			if ('@' != *curstr) {
				throw InvalidUri("Invalid format", uri);
			}
			curstr++;
		}

		if ('[' == *curstr) {
			bracket_flag = 1;
		} else {
			bracket_flag = 0;
		}

		tmpstr = curstr;
		while ('\0' != *tmpstr) {
			if (bracket_flag && ']' == *tmpstr) {
				tmpstr++;
				break;
			} else if (!bracket_flag && (':' == *tmpstr || '/' == *tmpstr)) {
				break;
			}
			tmpstr++;
		}
		if (tmpstr == curstr) {
			throw InvalidUri("Host cannot be blank", uri);
		}
		len = tmpstr - curstr;
		puri->host = std::string(curstr, len);
		boost::algorithm::to_lower(puri->host);
		curstr = tmpstr;

		if (':' == *curstr) {
			curstr++;
			tmpstr = curstr;
			while ('\0' != *tmpstr && '/' != *tmpstr) {
				tmpstr++;
			}
			len = tmpstr - curstr;
			puri->port = boost::lexical_cast<uint16_t>(std::string(curstr, len));
			curstr = tmpstr;
		}

		if ('\0' == *curstr) {
			return;
		}

		if ('/' != *curstr) {
			throw InvalidUri("Invalid format", uri);
		}
		curstr++;

		tmpstr = curstr;
		while ('\0' != *tmpstr && '#' != *tmpstr  && '?' != *tmpstr) {
			tmpstr++;
		}
		len = tmpstr - curstr;
		puri->path = std::string(curstr, len);
		curstr = tmpstr;

		if ('?' == *curstr) {
			curstr++;
			tmpstr = curstr;
			while ('\0' != *tmpstr && '#' != *tmpstr) {
				while ('\0' != *tmpstr && '#' != *tmpstr && '=' != *tmpstr && '&' != *tmpstr) {
					tmpstr++;
				}
				len = tmpstr - curstr;
				auto q = puri->query.insert({ std::string(curstr, len), std::string() });
				curstr = tmpstr;
				if ('=' == *curstr) {
					curstr++;
					while ('\0' != *tmpstr && '#' != *tmpstr && '&' != *tmpstr) {
						tmpstr++;
					}
					len = tmpstr - curstr;
					q->second = std::string(curstr, len);
					curstr = tmpstr;
				}
				if ('&' == *tmpstr) {
					tmpstr++;
					curstr = tmpstr;
				}
				else if ('\0' != *tmpstr && '#' != *tmpstr) {
					throw InvalidUri("Parse error in query params", uri);
				}
			}
			curstr = tmpstr;
		}

		if ('#' == *curstr) {
			curstr++;
			tmpstr = curstr;
			while ('\0' != *tmpstr) {
				tmpstr++;
			}
			len = tmpstr - curstr;
			puri->fragment = std::string(curstr, len);
		}
	}

	InvalidUri::InvalidUri(std::string  e, std::string  u) :
		Exception<std::invalid_argument>(std::string()),
		err(std::move(e)),
		uri(std::move(u))
	{
	}

	AdHocFormatter(InvalidUriMsg, "InvalidUri (%?) parsing [%?]");

	std::string
	InvalidUri::message() const noexcept
	{
		return InvalidUriMsg::get(err, uri);
	}
}

