#include "nagios.h"
#include "compileTimeFormatter.h"
#include "ctf-impl/printf-compat.h" // IWYU pragma: keep
#include <ctime>
#include <fstream> // IWYU pragma: keep
#include <sys/utsname.h>

namespace AdHoc {
	// [<timestamp>] PROCESS_SERVICE_CHECK_RESULT;<host_name>;<svc_description>;<return_code>;<plugin_output>
	AdHocFormatter(NagiosPassiveServiceCheck, "[%?] PROCESS_SERVICE_CHECK_RESULT;%?;%?;%d;%?\n");

	/// LCOV_EXCL_START (calls real Nagios)
	bool
	submitNagiosPassiveServiceCheck(
			const std::string_view & svc, NagiosStatusCode code, const std::string_view & output)
	{
		std::ofstream command_file("/var/nagios/rw/nagios.cmd");
		return submitNagiosPassiveServiceCheck(command_file, svc, code, output);
	}
	/// LCOV_EXCL_STOP

	bool
	submitNagiosPassiveServiceCheck(std::ostream & command_file, const std::string_view & svc, NagiosStatusCode code,
			const std::string_view & output)
	{
		if (command_file.good()) {
			struct utsname buf {
			};
			uname(&buf);
			NagiosPassiveServiceCheck::write(command_file, time(nullptr), buf.nodename, svc, (uint8_t)code, output);
			command_file.flush();
		}
		return command_file.good();
	}
}
