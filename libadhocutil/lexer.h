#ifndef ADHOCUTIL_LEXER_H
#define ADHOCUTIL_LEXER_H

#include <vector>
#include <glibmm/ustring.h>
#include <set>
#include <tuple>
#include <functional>
#include <memory>
#include <optional>
#include "visibility.h"

namespace AdHoc {
	/// An extensible lexer.
	class DLL_PUBLIC Lexer {
		public:
			/// Pattern matcher interface.
			class Pattern {
				public:
					virtual ~Pattern() = default;

					/// Test the pattern against the given input.
					virtual bool matches(const gchar *, size_t, size_t) const = 0;
					/// Get the total amount of input matched.
					virtual size_t matchedLength() const = 0;
					/// Get an extracted value from the pattern.
					virtual std::optional<Glib::ustring> match(int) const = 0;
			};
			/// Smart pointer to Pattern.
			typedef std::shared_ptr<Pattern> PatternPtr;
			/// Lexer state identifiers.
			typedef std::string State;
			/// Collection of States.
			typedef std::set<State> States;

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
					const State & getState() const;
					/// Get the state stack depth.
					size_t depth() const;
					/// Get the current position.
					size_t position() const;
					/// Get the currently matched pattern.
					PatternPtr pattern() const;

				private:
					friend class Lexer;
					size_t pos;
					PatternPtr pat;

					std::vector<State> stateStack;
			};

			/// Callback for handling matched patterns.
			typedef std::function<void(ExecuteState *)> Handler;
			/**
			 * Rule definition:
			 * States: in which states should the rule be considered?
			 * Pattern: the pattern matcher to test against the input.
			 * Handler: the callback to execute when a match succeeds.
			 */
			typedef std::tuple<States, PatternPtr, Handler> Rule;
			/// Collection of Rules that make up the lexer configuration.
			typedef std::vector<Rule> Rules;
			/// The initial state of applied to the lexer.
			static const State InitialState;
			/// Default constructor (empty rule set)
			Lexer();
			/// Construct with an initial set of rules.
			Lexer(const Rules);
			/// The lexer's current rule set.
			Rules rules;

			/// Execute the lexer to extract matches for the current rules.
			void extract(const gchar * string, size_t length) const;
	};
}

#endif

