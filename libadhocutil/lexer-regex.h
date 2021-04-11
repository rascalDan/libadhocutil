#ifndef ADHOCUTIL_LEXER_REGEX_H
#define ADHOCUTIL_LEXER_REGEX_H

#include "glibmm/ustring.h"
#include "lexer.h" // IWYU pragma: export
#include "visibility.h"
#include <glib.h>

namespace AdHoc {
	namespace LexerMatchers {
		/**
		 * Create a AdHoc::Lexer pattern matcher using regexen.
		 * @param regex The regex string.
		 * @param compile The regex compile flags.
		 * @param match The regex match flags.
		 * @return Pointer to the newly created pattern matcher.
		 */
		DLL_PUBLIC Lexer::PatternPtr regex(const Glib::ustring & regex,
				GRegexCompileFlags compile = (GRegexCompileFlags)0, GRegexMatchFlags match = (GRegexMatchFlags)0);
	}
};

#endif
