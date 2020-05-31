#ifndef ADHOCUTIL_PROCESSPIPES_H
#define ADHOCUTIL_PROCESSPIPES_H

#include "c++11Helpers.h"
#include "handle.h"
#include "visibility.h"
#include <optional>
#include <string>
#include <vector>

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

			/// Close input pipe to process
			int closeIn();

			/** FD handle to child's stdin. */
			[[nodiscard]] int fdIn() const noexcept;
			/** FD handle to child's stdout. */
			[[nodiscard]] int fdOut() const noexcept;
			/** FD handle to child's stderr. */
			[[nodiscard]] int fdError() const noexcept;
			/** Process id of child. */
			[[nodiscard]] pid_t pid() const noexcept;

		private:
			using PipePair = std::pair<int, int>;
			using InitPipe = std::optional<PipePair>;

			ProcessPipes(const std::vector<std::string> & args, InitPipe &&, InitPipe &&, InitPipe &&);

			static InitPipe pipeSetup(bool setup, bool swap);
			static void closeChild(const InitPipe & child) noexcept;
			static void dupChild(int, const InitPipe & child) noexcept;
			static void closeAllOpenFiles() noexcept;

			using FHandle = ::AdHoc::Handle<int, int (*)(int)>;
			using OFHandle = std::optional<FHandle>;
			const pid_t child;
			OFHandle in;
			const OFHandle out, error;
		};

	}
}

#endif
