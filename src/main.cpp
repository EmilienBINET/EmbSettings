#include <EmbSettings.hpp>
#include <iostream>

#ifdef USE_VLD
#include <vld.h>
#endif

template<typename T>
void print(T t) {
    std::cout << t << std::endl;
}

enum class EEnum {
    A1,
    B2,
    C3,
    D4
};

EEnum read_tree(boost::property_tree::ptree const& a_rTree, EEnum const& tDefaultVal) {
    return (EEnum)a_rTree.get<int>("", (int)tDefaultVal);
}

void write_tree(boost::property_tree::ptree& a_rTree, EEnum const& tVal) {
    a_rTree.put<int>("", (int)tVal);
}

namespace MyParams {
    EMBSETTINGS_FILE(Machine, XML, "Machine.xml", 1);
    EMBSETTINGS_SCALAR(Param1, double, Machine, "test.param1", 1.5);
    EMBSETTINGS_SCALAR(Param2, int, Machine, "test.param2", -5);
    EMBSETTINGS_SCALAR(Param3, std::string, Machine, "test.param3", "coucou");
    EMBSETTINGS_VECTOR(ParamVector, std::string, Machine, "test.vector");
    EMBSETTINGS_MAP(ParamMap, std::string, Machine, "test.map");
    EMBSETTINGS_SCALAR(ParamEnum, EEnum, Machine, "test.enum", EEnum::D4);
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

enum EEnum2 {
    E5,
    F6
};

void print(EEnum e) {
    std::cout << "enum" << (int)e << std::endl;
}

int main(int argc, char** argv)
{
    print(1);
    print("a");
    print(EEnum::C3);
    print(EEnum2::F6);

    emb::settings::set_jocker("folder1", "c:/temp/");
    std::cout << MyParams::Param3::read() << std::endl;
    MyParams::Param2::write(12);

    print(MyParams::ParamEnum::read());

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

    MyParams::ParamVector::write({ "a1", "b2", "c3", "d4" });
    MyParams::ParamMap::write({ { "A", "a1" }, { "B", "b2" }, { "C", "c3" }, { "D", "d4" } });

    getc(stdin);
    return 0;
}