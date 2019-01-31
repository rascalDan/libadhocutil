#include "lexer-regex.h"

namespace AdHoc {
	namespace LexerMatchers {
		class Regex : public Lexer::Pattern {
			public:
				Regex(const Glib::ustring & pattern, GRegexCompileFlags compile, GRegexMatchFlags match) :
					err(nullptr),
					regex(g_regex_new(pattern.c_str(), compile, match, &err)),
					info(nullptr)
				{
					if (!regex) {
						std::runtime_error e(std::string("Failed to create GRegex: ") + err->message);
						g_error_free(err);
						throw e;
					}
				}

				~Regex()
				{
					if (err) {
						g_error_free(err);
					}
					if (info) {
						g_match_info_free(info);
					}
					g_regex_unref(regex);
				}

				bool matches(const gchar * string, size_t length, size_t position) const override
				{
					if (info) {
						g_match_info_free(info);
					}
					g_regex_match_full(regex, string, length, position, G_REGEX_MATCH_ANCHORED, &info, &err);
					if (err) {
						std::runtime_error e(std::string("Failed to execute regex: ") + err->message);
						g_error_free(err);
						throw e;
					}
					str = string;
					return g_match_info_matches(info);
				}

				size_t matchedLength() const override
				{
					gint start, end;
					g_match_info_fetch_pos(info, 0, &start, &end);
					return end - start;
				}

				std::optional<Glib::ustring> match(int n) const override
				{
					gint start, end;
					if (g_match_info_fetch_pos(info, n, &start, &end)) {
						if (start == -1 && end == -1) {
							return {};
						}
						return Glib::ustring(str + start, str + end);
					}
					return {};
				}

			private:
				mutable GError * err;
				GRegex * regex;
				mutable GMatchInfo * info;
				mutable const gchar * str;
		};

		Lexer::PatternPtr
		regex(const Glib::ustring & pattern, GRegexCompileFlags compile, GRegexMatchFlags match)
		{
			return std::make_shared<Regex>(pattern, compile, match);
		}
	}
}

