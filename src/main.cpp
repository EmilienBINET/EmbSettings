#include "include/embsettings.h"
#include <iostream>

#ifdef USE_VLD
#include <vld.h>
#endif

namespace MyParams {
#define SETTINGS_MACHINE(_name, _type, _path, _default) EMBSETTINGS_DECLARE(_name, _type, 1, "Machine.xml>" _path, _default)
    SETTINGS_MACHINE(Param1, double, "test.param1", 1.5);
    SETTINGS_MACHINE(Param2, int, "test.param2", -5);
    SETTINGS_MACHINE(Param3, std::string, "test.param3", "coucou");
}

int main(int argc, char** argv)
{
    emb::settings::start();
    emb::settings::setJocker("folder1", "c:/temp/");
    std::cout << MyParams::Param3::read() << std::endl;
    MyParams::Param2::write(12);
    emb::settings::stop();
    return 0;
}