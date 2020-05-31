#ifndef LIBADHOCUTIL_UNITTESTS_UTILTESTCLASSES_H
#define LIBADHOCUTIL_UNITTESTS_UTILTESTCLASSES_H

#include <plugins.h>

class BaseThing : public AdHoc::AbstractPluginImplementation {
};

class ImplOfThing : public BaseThing {
};

class OtherImplOfThing : public BaseThing {
};

class OtherBase : public AdHoc::AbstractPluginImplementation {
};

class OtherImpl : public OtherBase {
};

#endif
