#ifndef ADHOCUTIL_PROCESSPIPES_H
#define ADHOCUTIL_PROCESSPIPES_H

#include <vector>
#include <string>
#include "visibility.h"
#include "c++11Helpers.h"

namespace AdHoc {
namespace System {

/// Spawn a process and attach to its IO handles.
class DLL_PUBLIC ProcessPipes {
	public:
		/**
		 * Spawn a new process, providing (optional) access to its stdin, stdout and
		 * stderr file handles.
		 * @param args path and arguments to spawn (passed to execv)
		 * @param in Attach to stdin?
		 * @param out Attach to stdout?
		 * @param err Attach to stderr?
		 */
		ProcessPipes(const std::vector<std::string> & args, bool in, bool out, bool err);
		/// Standard move/copy support
		SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(ProcessPipes);
		~ProcessPipes();

		/** FD handle to child's stdin. */
		[[nodiscard]] int fdIn() const noexcept;
		/** FD handle to child's stdout. */
		[[nodiscard]] int fdOut() const noexcept;
		/** FD handle to child's stderr. */
		[[nodiscard]] int fdError() const noexcept;
		/** Process id of child. */
		[[nodiscard]] pid_t pid() const noexcept;

		/** Close all open file handles as determined by rlimit and poll. */
		static void closeAllOpenFiles();

	private:
		int in, out, error;
		pid_t child;
};

}
}

#endif

