#pragma once
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include <boost/property_tree/ptree.hpp>

#define EMBSETTINGS_DECLARE_FILE(_name, _type, _path, _version)                                                                             \
class _name : public emb::settings::SettingsFile {                                                                                          \
    static bool registered;                                                                                                                 \
public:                                                                                                                                     \
    static char const FilePath[];                                                                                                      \
    static emb::settings::FileType const FileType;                                                                                          \
    static int const FileVersion;                                                                                                           \
    _name() : emb::settings::SettingsFile{#_name, emb::settings::FileType::_type, _path, _version} {}                                       \
	static std::unique_ptr<SettingsFile> CreateMethod() { return std::make_unique<_name>(); }                                               \
    static std::map<std::string, emb::settings::SettingsElement::CreateMethod>& getElementsMap() { return getElementsMap(#_name); }         \
    static void read_linked() {                                                                                                             \
        for (auto const& elm : getElementsMap()) {                                                                                          \
            elm.second()->read_linked();                                                                                                    \
        }                                                                                                                                   \
    }                                                                                                                                       \
    static void write_linked() {                                                                                                            \
        for (auto const& elm : getElementsMap()) {                                                                                          \
            elm.second()->write_linked();                                                                                                   \
        }                                                                                                                                   \
    }                                                                                                                                       \
private:                                                                                                                                    \
    using SettingsFile::getElementsMap;                                                                                                     \
};                                                                                                                                          \
bool _name::registered = emb::settings::register_file<_name>(#_name);                                                                       \
char const _name::FilePath[]{ _path };                                                                                                     \
emb::settings::FileType const _name::FileType{ emb::settings::FileType::_type };                                                            \
int const _name::FileVersion{ _version };                                                                                                   \

#define EMBSETTINGS_DECLARE_VALUE(_name, _type, _file, _path, _default)                                                                     \
namespace _name ## Private {                                                   \
    char _name ## ClassName[]{ #_name };                                                   \
    char _name ## TypeName[]{ #_type };                                                   \
    char _name ## Path[]{ _path };                                                        \
    _type _name ## Default{ _default };                                                   \
}                                                       \
class _name : public emb::settings::TSettingsElement<_name, _name ## Private :: _name ## ClassName, _type, \
_name ## Private :: _name ## TypeName, _file, _name ## Private :: _name ## Path, & _name ## Private :: _name ## Default> { \
void Register() noexcept override { registered = registered; } \
};
/*
class _name : public emb::settings::SettingsElement {                                                                                       \
    static bool registered;                                                                                                                 \
public:                                                                                                                                     \
    _name() : emb::settings::SettingsElement{#_name, #_type, #_file, _path} {}                                                              \
    static _type read() {                                                                                                                   \
        return read_setting<_type>(#_file, _path, _default);                                                                                \
    }                                                                                                                                       \
    static void write(_type const& tVal) {                                                                                                  \
        write_setting<_type>(#_file, _path, tVal);                                                                                          \
    }                                                                                                                                       \
    static void link(_type & rtVal) {                                                                                                       \
        link_setting<_type, _name>(#_file, _path, rtVal);                                                                                   \
    }                                                                                                                                       \
	static std::unique_ptr<SettingsElement> CreateMethod() { return std::make_unique<_name>(); }                                            \
};                                                                                                                                          \
bool _name::registered = _file::register_settings<_name>(#_file, _path);                                                                    \
*/
namespace emb {
    namespace settings {
        class SettingsFile;

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
            boost::property_tree::ptree tree{};

            struct Deleter {
                void operator()(SettingsFileInfo* a_pObj);
            };
            using Ptr = std::unique_ptr<SettingsFileInfo, Deleter>;

            static Ptr getFileInfo(std::unique_ptr<SettingsFile>);
        };

        class SettingsElement {
            std::string const m_strClassName;
            std::string const m_strType;
            std::string const m_strFileClassName;
            std::string const m_strPath;
        protected:
            static std::map<std::string, std::pair<std::function<void(void)>, std::function<void(void)>>>& getLinks();
        protected:
            SettingsElement(std::string const& a_strClassName, std::string const& a_strType, std::string const& a_strFileClassName, std::string const& a_strPath);
            template<typename Type>
            static Type read_setting(std::string const& a_strFileClass, std::string const& a_strKey, Type const& a_tDefaultValue);
            template<typename Type>
            static void write_setting(std::string const& a_strFileClass, std::string const& a_strKey, Type const& a_tNewValue);
            template<typename Type, typename Element>
            static void link_setting(std::string const& a_strFileClass, std::string const& a_strKey, Type& a_rtValue);
        public:
            using CreateMethod = std::unique_ptr<SettingsElement>(*)();
            std::string getClassName() const;
            std::string getType() const;
            std::string getFileClassName() const;
            std::string getPath() const;

            template<typename Type>
            Type read(Type const& a_tDefaultValue) const;
            std::string read() const;
            template<typename Type>
            void write(Type const& a_tNewValue) const;
            void write(std::string const& a_strNewValue) const;

            void read_linked() const;
            void write_linked() const;
        };

//using _name = emb::settings::TSettingsElement<_name ## ClassName, _type, _file, _name ## Path, _name ## Default>;
        template<typename Class, char const* ClassName, typename Type, char const* TypeName,
                 typename File, char const* Key, Type const* Default>
        class TSettingsElement : public SettingsElement {
        protected:
            static bool registered;
            virtual void Register() noexcept = 0;
        public:
            TSettingsElement() : emb::settings::SettingsElement{ClassName, TypeName, File::FilePath, Key} {}
            static Type read() {
                return read_setting<Type>(File::FilePath, Key, *Default);
            }
            static void write(Type const& tVal) {
                write_setting<Type>(File::FilePath, Key, tVal);
            }
            static void link(Type & rtVal) {
                link_setting<Type, Class>(File::FilePath, Key, rtVal);
            }
            static std::unique_ptr<SettingsElement> CreateMethod() { return std::make_unique<Class>(); }
        };
        template<typename Class, char const* ClassName, typename Type, char const* TypeName, typename File, char const* Key, Type const* Default>
        bool TSettingsElement<Class, ClassName, Type, TypeName, File, Key, Default>::registered =
            File::register_settings(File::FilePath, Key, Class::CreateMethod);

        class SettingsFile {
            std::string const m_strClassName;
            FileType const m_eFileType;
            std::string const m_strFilePath;
            int const m_iFileVersion;
            static std::map<std::string, std::map<std::string, SettingsElement::CreateMethod>>& getMap();
        protected:
            SettingsFile(std::string const& a_strClassName, FileType a_eFileType, std::string const& a_strFilePath, int a_iFileVersion);
        public:
            using CreateMethod = std::unique_ptr<SettingsFile>(*)();
            std::string getClassName() const;
            FileType getFileType() const;
            std::string getFilePath() const;
            int getFileVersion() const;

            static bool register_settings(char const* a_szFile, char const* a_szPath, SettingsElement::CreateMethod a_pCreateMethod);
            static std::map<std::string, emb::settings::SettingsElement::CreateMethod>& getElementsMap(std::string const& a_strFileClass);
        };

        std::map<std::string, SettingsFile::CreateMethod>& getFilesMap();

        template<typename T>
        bool register_file(std::string const& a_strName);
    }
}
#include "EmbSettings.impl"

/*

param�tres
: communs accessibles de partout
sp�cifque ajout�s facilement
interface simple c/c++
stocquage provenant de
- json : fichier/cl�
- xml : fichier/cl�
- bdd : db/table/cl�
- param�trs application : nomparam/val

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