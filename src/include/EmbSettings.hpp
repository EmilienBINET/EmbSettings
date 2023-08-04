#pragma once
#include <string>
#include <memory>
#include <map>
#include <boost/property_tree/ptree.hpp>

#include <iostream> // temporaire

#define EMBSETTINGS_DECLARE_FILE(_name, _type, _path, _version)                                     \
class _name : public emb::settings::SettingsFile {                                                  \
    static bool registered;                                                                         \
public:                                                                                             \
    static std::string const FilePath;                                                              \
    static emb::settings::FileType const FileType;                                                  \
    static int const FileVersion;                                                                   \
    _name() : emb::settings::SettingsFile{#_name, _type, _path, _version} {}                        \
	static std::unique_ptr<SettingsFile> CreateMethod() { return std::make_unique<_name>(); }       \
    static std::map<std::string, emb::settings::SettingsElement::CreateMethod>& getElementsMap() { return getMap()[#_name]; } \
private:                                                                                            \
    using SettingsFile::getMap;    \
};                                                                                                  \
bool _name::registered = emb::settings::register_file<_name>(#_name);                               \
std::string const _name::FilePath{_path};                                                           \
emb::settings::FileType const _name::FileType{_type};                                               \
int const _name::FileVersion{_version};                                                             \

#define EMBSETTINGS_DECLARE_SETTING(_name, _type, _file, _path, _default)                           \
class _name : public emb::settings::SettingsElement {                                               \
    static bool registered;                                                                         \
public:                                                                                             \
    _name() : emb::settings::SettingsElement{#_name, #_type, #_file, _path} {}                      \
    static _type read() {                                                                           \
        return emb::settings::SettingsElement::read<_type, _file>(_path, _default);                                                 \
    }                                                                                               \
    static void write(_type const& tVal) {                                                          \
        emb::settings::SettingsElement::write<_type, _file>(_path, tVal);                                                           \
    }                                                                                               \
	static std::unique_ptr<SettingsElement> CreateMethod() { return std::make_unique<_name>(); }    \
};                                                                                                  \
bool _name::registered = _file::register_settings<_name>(#_file, _path);                            \

namespace emb {
    namespace settings {
        void start();
        void stop();
        void setJocker(std::string const& aJocker, std::string const& aValue);

        enum class FileType {
            XML,
            JSON,
            INI
        };

        struct SettingsFileInfo {
            std::string /*const*/ strFilename{};
            FileType eFileType{};
            std::stringstream strFilecontent{};
            boost::property_tree::ptree tree{};

            struct Deleter {
                void operator()(SettingsFileInfo* a_pObj);
            };
            using Ptr = std::unique_ptr<SettingsFileInfo, Deleter>;

            static Ptr getFileInfo(std::string const& a_strPath, FileType a_eFileType);
        };

        class SettingsElement {
            std::string const m_strClassName;
            std::string const m_strType;
            std::string const m_strFileClassName;
            std::string const m_strPath;
        protected:
            SettingsElement(std::string const& a_strClassName, std::string const& a_strType, std::string const& a_strFileClassName, std::string const& a_strPath);
        public:
            using CreateMethod = std::unique_ptr<SettingsElement>(*)();
            std::string getClassName() const;
            std::string getType() const;
            std::string getFileClassName() const;
            std::string getPath() const;
            std::string getValue() const;

            template<typename T, typename F>
            static T read(std::string path, T defaultValue) {
                if (auto pInfo = SettingsFileInfo::getFileInfo(F::FilePath, F::FileType)) {
                    if (auto val = pInfo->tree.template get_optional<T>(path)) {
                        return *val;
                    }
                }
                return defaultValue;
            }

            template<typename T, typename F>
            static void write(std::string path, T value) {
                if (auto pInfo = SettingsFileInfo::getFileInfo(F::FilePath, F::FileType)) {
                    pInfo->tree.template put<T>(path, value);
                }
            }
        };

        class SettingsFile {
            std::string const m_strClassName;
            FileType const m_eFileType;
            std::string const m_strFilePath;
            int const m_iFileVersion;
        protected:
            SettingsFile(std::string const& a_strClassName, FileType a_eFileType, std::string const& a_strFilePath, int a_iFileVersion);
        public:
            using CreateMethod = std::unique_ptr<SettingsFile>(*)();
            std::string getClassName() const;
            FileType getFileType() const;
            std::string getFilePath() const;
            int getFileVersion() const;
            std::string read(std::string const& a_strPath) const;

            template<typename T>
            static bool register_settings(char const* a_szFile, char const* a_szPath) {
                std::cout << "Registering Setting: " << typeid(T).name() << " as " << a_szPath << " in " << a_szFile << std::endl;
                getMap()[a_szFile][a_szPath] = T::CreateMethod;
                return true;
            }
            static std::map<std::string, std::map<std::string,SettingsElement::CreateMethod>>& getMap();
        };

        std::map<std::string, SettingsFile::CreateMethod>& getFilesMap();

        template<typename T>
        bool register_file(std::string const& a_strName) {
            std::cout << "Registering File: " << typeid(T).name() << " as " << a_strName << std::endl;
            getFilesMap()[a_strName] = T::CreateMethod;
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