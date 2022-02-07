#pragma once

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
