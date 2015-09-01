#include "processPipes.h"
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <sys/resource.h>
#include <stdexcept>

namespace AdHoc {
namespace System {

ProcessPipes::ProcessPipes(const std::vector<std::string> & args, bool i, bool o, bool e) :
	in(-1),
	out(-1),
	error(-1)
{
	int ipipes[2], opipes[2], epipes[2];
	if (i) {
		if (pipe(ipipes)) {
			throw std::runtime_error("Failed to create stdin pipe");
		}
	}
	if (o) {
		if (pipe(opipes)) {
			if (i) {
				close(ipipes[0]);
				close(ipipes[1]);
			}
			throw std::runtime_error("Failed to create stdout pipe");
		}
	}
	if (e) {
		if (pipe(epipes)) {
			if (i) {
				close(ipipes[0]);
				close(ipipes[1]);
			}
			if (o) {
				close(opipes[0]);
				close(opipes[1]);
			}
			throw std::runtime_error("Failed to create stderr pipe");
		}
	}
	switch (child = fork()) {
		case -1: // fail
			if (i) {
				close(ipipes[0]);
				close(ipipes[1]);
			}
			if (o) {
				close(opipes[0]);
				close(opipes[1]);
			}
			if (e) {
				close(epipes[0]);
				close(epipes[1]);
			}
			throw std::runtime_error("Failed to fork");
		default: // parent
			if (i) {
				close(ipipes[0]);
				in = ipipes[1];
			}
			if (o) {
				close(opipes[1]);
				out = opipes[0];
			}
			if (e) {
				close(epipes[1]);
				error = epipes[0];
			}
			break;
		case 0: // in child
			rlimit lim;
			getrlimit(RLIMIT_NOFILE, &lim);
			if (i) {
				close(ipipes[1]);
				dup2(ipipes[0], 0);
			}
			if (o) {
				close(opipes[0]);
				dup2(opipes[1], 1);
			}
			if (e) {
				close(epipes[0]);
				dup2(epipes[1], 2);
			}
			std::vector<struct pollfd> fds;
			fds.reserve(lim.rlim_max);
			for (int n = 3; n < (int)lim.rlim_max; n += 1) {
				fds.push_back({n, 0, 0});
			}
			poll(&fds.front(), fds.size(), 0);
			for(const auto & pfd : fds) {
				if (!(pfd.revents & POLLNVAL)) {
					close(pfd.fd);
				}
			}
			char * buf[100];
			char ** w = &buf[0];
			for (const auto & p : args) {
				*w++ = strdup(p.c_str());
			}
			*w = NULL;
			execv(buf[0], buf);
			abort();
			break;
	}
}

ProcessPipes::~ProcessPipes()
{
	if (in) close(in);
	if (out) close(out);
	if (error) close(error);
}

int
ProcessPipes::fdIn() const
{
	return in;
}

int
ProcessPipes::fdOut() const
{
	return out;
}

int
ProcessPipes::fdError() const
{
	return error;
}

pid_t
ProcessPipes::pid() const
{
	return child;
}

}
}

