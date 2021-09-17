#ifndef ADHOCUTIL_LEXER_REGEX_H
#define ADHOCUTIL_LEXER_REGEX_H

#include "lexer.h" // IWYU pragma: export
#include "visibility.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#	pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <glib.h>
#include <glibmm/ustring.h>
#pragma GCC diagnostic pop

namespace AdHoc {
	namespace LexerMatchers {
		/**
		 * Create a AdHoc::Lexer pattern matcher using regexen.
		 * @param regex The regex string.
		 * @param compile The regex compile flags.
		 * @param match The regex match flags.
		 * @return Pointer to the newly created pattern matcher.
		 */
		DLL_PUBLIC Lexer::PatternPtr regex(
				const Glib::ustring & regex, GRegexCompileFlags compile = {}, GRegexMatchFlags match = {});
	}
}

#endif
