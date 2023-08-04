#pragma once
#include <string>
#include <memory>
#include <boost/property_tree/ptree.hpp>

#include <iostream> // temporaire

#define EMBSETTINGS_DECLARE(_name, _type, _version, _path, _default)                                \
class _name                                                                                         \
{                                                                                                   \
    static bool registered;                                                                         \
private:                                                                                            \
    _name() = delete;                                                                               \
    ~_name() = delete;                                                                              \
    _name& operator=(_name const&) = delete;                                                        \
public:                                                                                             \
    static size_t const id;                                                                         \
    static _type read() {                                                                           \
        return emb::settings::SettingsManager::instance().read<_type>(_version, _path, _default);   \
    }                                                                                               \
    static void write(_type const& tVal) {                                                          \
        emb::settings::SettingsManager::instance().write<_type>(_path, tVal);                       \
    }                                                                                               \
};                                                                                                  \
bool _name::registered = emb::settings::register_class<_name>();                                    \
size_t const _name::id = reinterpret_cast<size_t>(&_name::registered);                              \

namespace emb {
    namespace settings {
        void start();
        void stop();
        void setJocker(std::string const& aJocker, std::string const& aValue);

        struct SettingsFileInfo {
            std::string /*const*/ strFilename{};
            std::stringstream strFilecontent{};
            boost::property_tree::ptree tree{};

            struct Deleter {
                void operator()(SettingsFileInfo* a_pObj);
            };
            using Ptr = std::unique_ptr<SettingsFileInfo, Deleter>;

            static Ptr getFileInfo(std::string& a_rstrPath);
        };

        class SettingsManager final {
        public:
            static SettingsManager& instance() {
                static SettingsManager manager;
                return manager;
            }

            template<typename T>
            T read(int version, std::string path, T defaultValue) {
                if (auto pInfo = SettingsFileInfo::getFileInfo(path)) {
                    if (auto val = pInfo->tree.get_optional<T>(path)) {
                        return *val;
                    }
                }
                return defaultValue;
            }

            template<typename T>
            void write(std::string path, T value) {
                if (auto pInfo = SettingsFileInfo::getFileInfo(path)) {
                    pInfo->tree.put<T>(path, value);
                }
            }

        private:
            SettingsManager();
        };

        template<typename T>
        bool register_class() {
            std::cout << "Registering: " << typeid(T).name() << std::endl;
            return true;
        }
    }
}

/*

paramètres
: communs accessibles de partout
spécifque ajoutés facilement
interface simple c/c++
stocquage provenant de
- json : fichier/clé
- xml : fichier/clé
- bdd : db/table/clé
- paramètrs application : nomparam/val

#pragma once

#include <string>

enum class ParamId {
    MachineNumber,
    Test
};

class IParameter {
public:
    IParameter() noexcept = default;
    virtual ~IParameter() noexcept = default;
};

template<typename T>
class TParameter : public IParameter {
public:
    TParameter() noexcept = default;
    virtual ~TParameter() noexcept = default;
    virtual T read() const noexcept = 0;
    virtual void write(T const&) noexcept = 0;
    std::string paramFolder() const noexcept { return ""; }
};

template<typename T>
class TParameterXml : public TParameter<T> {
public:
    TParameterXml(std::string const& aFilePath, std::string const& aXmlKey, T const& aDefault = T()) noexcept : mFilePath{aFilePath}, mXmlKey{aXmlKey} {}
    virtual ~TParameterXml() noexcept = default;
    T read() const noexcept override {
        return ParameterXmlManager::instance().read<T>(mFilePath, mXmlKey);
    }
    void write(T const&) noexcept override {
        ParameterXmlManager::instance().read<T>(mFilePath, mXmlKey);
    }
private:
    std::string mFilePath{};
    std::string mXmlKey{};
};

template<typename T>
class TParamMachine : public TParameterXml<T> {
    TParamMachine(std::string const& aXmlKey, T const& aDefault = T()) : TParameterXml{"/param.xml", aXmlKey, aDefault} {}
};

struct ParamMachineNumber : public TParamMachine<int> {
    ParamMachineNumber() : TParamMachine{"/<test>/<truc>/machin", 0} {}
};

ParamMachineNumber::read();
ParamMachineNumber::write();

::Param::start();

int i = ::Param::MachineNumber::read();
::Param::MachineNumber::write(10);

::Param::stop();
*/