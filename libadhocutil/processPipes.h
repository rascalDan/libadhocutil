#ifndef ADHOCUTIL_PROCESSPIPES_H
#define ADHOCUTIL_PROCESSPIPES_H

#include <vector>
#include <string>
#include "visibility.h"

/**
 * Spawn a new process, providing access to it's stdin and stdout
 * @param params path and arguments to spawn (copied and passed to execv)
 * @param fds (out) the FDs on the childs stdin(0) and stdout(1)
 * @return the process ID of the child
 */
class DLL_PUBLIC ProcessPipes {
	public:
		ProcessPipes(const std::vector<std::string> & args, bool in, bool out, bool err);
		~ProcessPipes();

		ProcessPipes(const ProcessPipes &) = delete;
		void operator=(const ProcessPipes &) = delete;

		int fdIn() const;
		int fdOut() const;
		int fdError() const;
		pid_t pid() const;

	private:
		int in, out, error;
		pid_t child;
};

#endif

