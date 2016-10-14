#ifndef ADHOCUTIL_LEXER_REGEX_H
#define ADHOCUTIL_LEXER_REGEX_H

#include "lexer.h"

namespace AdHoc {
	namespace LexerMatchers {
		DLL_PUBLIC Lexer::PatternPtr regex(const Glib::ustring &, GRegexCompileFlags compile = (GRegexCompileFlags)0, GRegexMatchFlags match = (GRegexMatchFlags)0);
	}
};

#endif

