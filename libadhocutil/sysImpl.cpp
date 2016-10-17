#include <sys.h>
#include <boost/format.hpp>

namespace AdHoc {
	static boost::format e("%s (%d:%s)");
	void SystemException::ice_print(std::ostream & s) const
	{
		s << e % task % errNo % message;
	}
}
