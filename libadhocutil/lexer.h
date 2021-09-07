#ifndef ADHOCUTIL_LEXER_H
#define ADHOCUTIL_LEXER_H

#include "c++11Helpers.h"
#include "visibility.h"
#include <cstddef>
#include <functional>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <glib.h>
#pragma GCC diagnostic pop
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace Glib {
	class ustring;
}

namespace AdHoc {
	/// An extensible lexer.
	class DLL_PUBLIC Lexer {
	public:
		/// Pattern matcher interface.
		class Pattern {
		public:
			Pattern() = default;
			virtual ~Pattern() = default;
			/// Standard move/copy support
			SPECIAL_MEMBERS_DEFAULT(Pattern);

			/// Test the pattern against the given input.
			[[nodiscard]] virtual bool matches(const gchar *, size_t, size_t) const = 0;
			/// Get the total amount of input matched.
			[[nodiscard]] virtual size_t matchedLength() const = 0;
			/// Get an extracted value from the pattern.
			[[nodiscard]] virtual std::optional<Glib::ustring> match(int) const = 0;
		};
		/// Smart pointer to Pattern.
		using PatternPtr = std::shared_ptr<Pattern>;
		/// Lexer state identifiers.
		using State = std::string;
		/// Collection of States.
		using States = std::set<State>;

		/// Class representing the runtime execution of the lexer.
		class ExecuteState {
		public:
			/// Default constructor.
			ExecuteState();

			/// Push a new state to the stack.
			void pushState(const State &);
			/// Pop the top of the state stack off.
			void popState();
			/// Replace the current top of the state stack.
			void setState(const State &);
			/// Get the current state.
			[[nodiscard]] const State & getState() const;
			/// Get the state stack depth.
			[[nodiscard]] size_t depth() const;
			/// Get the current position.
			[[nodiscard]] size_t position() const;
			/// Get the currently matched pattern.
			[[nodiscard]] PatternPtr pattern() const;

		private:
			friend class Lexer;
			size_t pos {0};
			PatternPtr pat;

			std::vector<State> stateStack;
		};

		/// Callback for handling matched patterns.
		using Handler = std::function<void(ExecuteState *)>;
		/**
		 * Rule definition:
		 * States: in which states should the rule be considered?
		 * Pattern: the pattern matcher to test against the input.
		 * Handler: the callback to execute when a match succeeds.
		 */
		using Rule = std::tuple<States, PatternPtr, Handler>;
		/// Collection of Rules that make up the lexer configuration.
		using Rules = std::vector<Rule>;
		/// The initial state of applied to the lexer.
		static const State InitialState;
		/// Default constructor (empty rule set)
		Lexer();
		/// Construct with an initial set of rules.
		explicit Lexer(Rules);
		/// The lexer's current rule set.
		Rules rules;

		/// Execute the lexer to extract matches for the current rules.
		void extract(const gchar * string, size_t length) const;
	};
}

#endif
