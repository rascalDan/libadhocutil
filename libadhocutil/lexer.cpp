#include "lexer.h"

namespace AdHoc {
	const Lexer::State Lexer::InitialState = "";

	Lexer::Lexer()
	{
	}

	Lexer::Lexer(const Rules & r) : rules(r)
	{
	}

	void
	Lexer::extract(const gchar * string, size_t length) const
	{
		ExecuteState es;
		while (es.position < length) {
			const Rule * selected = nullptr;
			for (const auto & r : rules) {
				const auto & s = boost::get<0>(r);
				if (s.find(es.getState()) == s.end()) {
					continue;
				}
				const auto & p = boost::get<1>(r);
				if (p->matches(string, length, es.position)) {
					selected = &r;
					break;
				}
			}
			if (!selected) {
				throw std::runtime_error(std::string("Unexpected input in state (" + es.getState() + ") at ") + (string + es.position));
			}
			es.pattern = boost::get<1>(*selected);
			const auto & h = boost::get<2>(*selected);
			h(&es);
			es.position += es.pattern->matchedLength();
		}
		
	}

	Lexer::ExecuteState::ExecuteState() :
		position(0)
	{
		stateStack.push_back(InitialState);
	}

	void
	Lexer::ExecuteState::setState(const State & s)
	{
		stateStack.back() = s;
	}

	void
	Lexer::ExecuteState::pushState(const State & s)
	{
		stateStack.push_back(s);
	}

	void
	Lexer::ExecuteState::popState()
	{
		stateStack.pop_back();
	}

	const Lexer::State &
	Lexer::ExecuteState::getState() const
	{
		return stateStack.back();
	}

	size_t
	Lexer::ExecuteState::depth() const
	{
		return stateStack.size();
	}
}

