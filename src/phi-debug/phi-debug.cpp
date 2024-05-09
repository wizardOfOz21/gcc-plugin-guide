// GCC header includes to get the parse tree
// declarations. The order is important and
// doesn't follow any kind of logic.
//

#include "gcc-plugin.h"
#include "plugin-version.h"

using namespace std;

int plugin_is_GPL_compatible;

extern "C" int
plugin_init(plugin_name_args *info,
plugin_gcc_version *ver)
{
    plugin_default_version_check(ver, &gcc_version);
    return 0;
}
