#include "processPipes.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <poll.h>
#include <stdexcept>
#include <sys.h>
#include <sys/resource.h>
#include <unistd.h>

namespace AdHoc::System {
	ProcessPipes::InitPipe
	ProcessPipes::pipeSetup(bool setup, bool swap)
	{
		if (setup) {
			PipePair pair;
			if (::pipe(&pair.first)) {
				throw SystemException("pipe(2) failed", strerror(errno), errno);
			}
			if (swap) {
				std::swap(pair.first, pair.second);
			}
			return pair;
		}
		return {};
	}

	void
	ProcessPipes::closeChild(const InitPipe & pipe) noexcept
	{
		if (pipe) {
			close(pipe->second);
		}
	}

	void
	ProcessPipes::dupChild(int fd, const InitPipe & pipe) noexcept
	{
		if (pipe) {
			close(pipe->first);
			dup2(pipe->second, fd);
		}
	}

	ProcessPipes::ProcessPipes(const std::vector<std::string> & args, bool i, bool o, bool e) :
		ProcessPipes(
				[&args]() {
					if (args.empty()) {
						throw std::invalid_argument("args is empty");
					}
					return args;
				}(),
				pipeSetup(i, true), pipeSetup(o, false), pipeSetup(e, false))
	{
	}

	ProcessPipes::ProcessPipes(
			const std::vector<std::string> & args, InitPipe && ipp, InitPipe && opp, InitPipe && epp) :
		child([]() {
			if (pid_t p = fork(); p != -1) {
				return p;
			}
			/// LCOV_EXCL_START (fork won't fail)
			throw SystemException("fork(2) failed", strerror(errno), errno);
			/// LCOV_EXCL_STOP
		}()),
		in(ipp ? std::make_optional<FHandle>(ipp->first, &close) : OFHandle()),
		out(opp ? std::make_optional<FHandle>(opp->first, &close) : OFHandle()),
		error(epp ? std::make_optional<FHandle>(epp->first, &close) : OFHandle())
	{
		switch (child) {
			default: // parent
				closeChild(ipp);
				closeChild(opp);
				closeChild(epp);
				break;
			case 0: // in child
				dupChild(STDIN_FILENO, ipp);
				dupChild(STDOUT_FILENO, opp);
				dupChild(STDERR_FILENO, epp);
				closeAllOpenFiles();
				std::vector<char *> buf(args.size() + 1);
				auto w = buf.begin();
				for (const auto & p : args) {
					*w++ = strdup(p.c_str());
				}
				*w = nullptr;
				if (buf[0]) {
					execv(buf[0], buf.data());
				}
				abort();
				break;
		}
	}

	int
	ProcessPipes::closeIn()
	{
		if (in) {
			in.reset();
			return 0;
		}
		return EBADF;
	}

	int
	ProcessPipes::fdIn() const noexcept
	{
		return in ? *in : -1;
	}

	int
	ProcessPipes::fdOut() const noexcept
	{
		return out ? *out : -1;
	}

	int
	ProcessPipes::fdError() const noexcept
	{
		return error ? *error : -1;
	}

	pid_t
	ProcessPipes::pid() const noexcept
	{
		return child;
	}

	void
	ProcessPipes::closeAllOpenFiles() noexcept
	{
		rlimit lim {};
		getrlimit(RLIMIT_NOFILE, &lim);
		std::vector<struct pollfd> fds;
		fds.reserve(lim.rlim_max);
		for (int n = 3; n < (int)lim.rlim_max; n += 1) {
			fds.push_back({n, 0, 0});
		}
		poll(&fds.front(), fds.size(), 0);
		for (const auto & pfd : fds) {
			if (!(pfd.revents & POLLNVAL)) {
				close(pfd.fd);
			}
		}
	}
}
