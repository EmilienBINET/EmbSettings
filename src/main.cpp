#include <EmbSettings.hpp>
#include <iostream>

#ifdef USE_VLD
#include <vld.h>
#endif

namespace MyParams {
    EMBSETTINGS_DECLARE_FILE(Machine, XML, "Machine.xml", 1);
    EMBSETTINGS_DECLARE_VALUE(Param1, double, Machine, "test.param1", 1.5);
    EMBSETTINGS_DECLARE_VALUE(Param2, int, Machine, "test.param2", -5);
    EMBSETTINGS_DECLARE_VALUE(Param3, std::string, Machine, "test.param3", "coucou");
}

struct MachineSettings {
    double param1;
    int param2;
    std::string param3;

    MachineSettings() {
        MyParams::Param1::link(param1);
        MyParams::Param2::link(param2);
        MyParams::Param3::link(param3);
    }

    void read() {
        MyParams::Machine::read_linked();
    }
    void write() {
        MyParams::Machine::write_linked();
    }
};

int main(int argc, char** argv)
{
    emb::settings::start();
    emb::settings::setJocker("folder1", "c:/temp/");
    std::cout << MyParams::Param3::read() << std::endl;
    MyParams::Param2::write(12);
    emb::settings::stop();

    for (auto const& file : emb::settings::getFilesMap()) {
        std::cout << "FILE " << file.first << std::endl;
        if (auto const& fileInfo = file.second()) {
            std::cout << "{" << fileInfo->getFilePath() << "}" << std::endl;
        }
        for (auto const& elm : emb::settings::SettingsFile::getElementsMap(file.first)) {
            std::cout << "- ELM " << elm.first << std::endl;
            if (auto const& elmInfo = elm.second()) {
                std::cout << "  {" << elmInfo->getPath() << "}" << std::endl;
            }
        }
    }

    MachineSettings machineSettings;
    machineSettings.read();
    std::cout << machineSettings.param1 << std::endl;
    std::cout << machineSettings.param2 << std::endl;
    std::cout << machineSettings.param3 << std::endl;
    machineSettings.param3 = "hello";
    machineSettings.write();

    getc(stdin);
    return 0;
}