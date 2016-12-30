#include <sys.h>
#include "compileTimeFormatter.h"

namespace AdHoc {
	AdHocFormatter(SystemExceptionMsg, "%? (%?:%?)");

	void SystemException::ice_print(std::ostream & s) const
	{
		SystemExceptionMsg::write(s, task, errNo, message);
	}

	AdHocFormatter(SystemExceptionOnMsg, "%? on '%?' (%?:%?)");

	void SystemExceptionOn::ice_print(std::ostream & s) const
	{
		SystemExceptionOnMsg::write(s, task, objectName, errNo, message);
	}
}
