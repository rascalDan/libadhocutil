#ifndef ADHOCUTIL_PROCESSPIPES_H
#define ADHOCUTIL_PROCESSPIPES_H

#include <vector>
#include <string>
#include "visibility.h"

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
		~ProcessPipes();

		/** FD handle to child's stdin. */
		int fdIn() const;
		/** FD handle to child's stdout. */
		int fdOut() const;
		/** FD handle to child's stderr. */
		int fdError() const;
		/** Process id of child. */
		pid_t pid() const;

	private:
		ProcessPipes(const ProcessPipes &) = delete;
		void operator=(const ProcessPipes &) = delete;

		int in, out, error;
		pid_t child;
};

}
}

#endif

