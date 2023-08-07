#pragma once
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include <boost/property_tree/ptree.hpp>

/**
 * @brief Declare a file that can contain settings
 * @param _name     Name of the class representing the file
 * @param _type     Type of file (amongst \c emb::settings::FileType enumeration, without the scope: e.g. XML )
 * @param _path     Path of the file on the system. Jockers can be used with the format @{jocker}
 * @param _version  Current version of the file
 */
#define EMBSETTINGS_FILE(_name, _type, _path, _version)                                                                                     \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    char ClassName[]{ #_name };                                                                                                             \
    char TypeName[]{ #_type };                                                                                                              \
    char Path[]{ _path };                                                                                                                   \
} }                                                                                                                                         \
class _name final : public emb::settings::TSettingsFile<                                                                                    \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::ClassName,                                                                                              \
        emb::settings::FileType::_type,                                                                                                     \
        EmbSettings_Private::_name::TypeName,                                                                                               \
        EmbSettings_Private::_name::Path,                                                                                                   \
        _version                                                                                                                            \
    > {                                                                                                                                     \
    void Register() noexcept override { registered = registered; }                                                                          \
};

/**
 * @brief Declare a scalar setting inside a previously declared setting file
 * @param _name     Name of the class representing the setting
 * @param _type     Data type of the setting
 * @param _file     Class name of the file used to save the setting
 * @param _key      Key string representing the position of the setting in the file (using boost property_tree synthax)
 * @param _default  Default value of the setting if not found in the file
 */
#define EMBSETTINGS_SCALAR(_name, _type, _file, _key, _default)                                                                             \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    char ClassName[]{ #_name };                                                                                                             \
    char TypeName[]{ #_type };                                                                                                              \
    char Key[]{ _key };                                                                                                                     \
    _type Default{ _default };                                                                                                              \
} }                                                                                                                                         \
class _name final : public emb::settings::TSettingsScalar<                                                                                  \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::ClassName,                                                                                              \
        _type,                                                                                                                              \
        EmbSettings_Private::_name::TypeName,                                                                                               \
        _file,                                                                                                                              \
        EmbSettings_Private::_name::Key,                                                                                                    \
        &EmbSettings_Private::_name::Default                                                                                                \
    > {                                                                                                                                     \
    void Register() noexcept override { registered = registered; }                                                                          \
};

#define EMBSETTINGS_VECTOR(_name, _type, _file, _key)                                                                                       \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    char ClassName[]{ #_name };                                                                                                             \
    char TypeName[]{ "std::vector<" #_type ">" };                                                                                           \
    char Key[]{ _key };                                                                                                                     \
} }                                                                                                                                         \
class _name final : public emb::settings::TSettingsVector<                                                                                  \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::ClassName,                                                                                              \
        _type,                                                                                                                              \
        EmbSettings_Private::_name::TypeName,                                                                                               \
        _file,                                                                                                                              \
        EmbSettings_Private::_name::Key                                                                                                     \
    > {                                                                                                                                     \
    void Register() noexcept override { registered = registered; }                                                                          \
};

namespace emb { namespace settings {

/**
 * @brief Type of a settings file
 */
enum class FileType {
    XML,    ///< XML file
    JSON,   ///< JSON file
    INI     ///< INI file
};

/**
 * @brief Defines a jocker value, that can be used in settings files' path
 * @param a_strJocker   Name of the jocker, without the @{...} pattern
 * @param a_strValue    Value that will replace the pattern @{a_strJocker}
 */
void set_jocker(std::string const& a_strJocker, std::string const& a_strValue);

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
    template<typename Type>
    static std::vector<Type> read_setting_vector(std::string const& a_strFileClass, std::string const& a_strKey);
    template<typename Type>
    static void write_setting_vector(std::string const& a_strFileClass, std::string const& a_strKey, std::vector<Type> const& a_tvecNewValue);
    template<typename Type>
    static void add_setting_vector(std::string const& a_strFileClass, std::string const& a_strKey, Type const& tVal);
public:
    virtual ~SettingsElement() {}
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

template<typename Class, char const* ClassName, typename Type, char const* TypeName,
            typename File, char const* Key, Type const* Default>
class TSettingsScalar : public SettingsElement {
protected:
    static bool registered;
    virtual void Register() noexcept = 0;
public:
    TSettingsScalar() : SettingsElement{ClassName, TypeName, File::FilePath, Key} {}
    virtual ~TSettingsScalar() {}
    static Type read() {
        return read_setting<Type>(File::Name, Key, *Default);
    }
    static void write(Type const& tVal) {
        write_setting<Type>(File::Name, Key, tVal);
    }
    static void link(Type & rtVal) {
        link_setting<Type, Class>(File::Name, Key, rtVal);
    }
    static std::unique_ptr<SettingsElement> CreateMethod() { return std::make_unique<Class>(); }
};
template<typename Class, char const* ClassName, typename Type, char const* TypeName, typename File, char const* Key, Type const* Default>
bool TSettingsScalar<Class, ClassName, Type, TypeName, File, Key, Default>::registered =
    File::register_settings(File::Name, Key, Class::CreateMethod);


template<typename Class, char const* ClassName, typename Type, char const* TypeName,
            typename File, char const* Key>
class TSettingsVector : public SettingsElement {
protected:
    static bool registered;
    virtual void Register() noexcept = 0;
public:
    TSettingsVector() : SettingsElement{ClassName, TypeName, File::FilePath, Key} {}
    virtual ~TSettingsVector() {}
    static std::vector<Type> read() {
        return read_setting_vector<Type>(File::Name, Key);
    }
    static void write(std::vector<Type> const& tvecVal) {
        write_setting_vector<Type>(File::Name, Key, tvecVal);
    }
    static void add(Type const& tVal) {
        add_setting_vector<Type>(File::Name, Key, tVal);
    }
    static void link(std::vector<Type> & rtvecVal) {
        link_setting<std::vector<Type>, Class>(File::Name, Key, rtvecVal);
    }
    static std::unique_ptr<SettingsElement> CreateMethod() { return std::make_unique<Class>(); }
};
template<typename Class, char const* ClassName, typename Type, char const* TypeName, typename File, char const* Key>
bool TSettingsVector<Class, ClassName, Type, TypeName, File, Key>::registered =
    File::register_settings(File::Name, Key, Class::CreateMethod);


class SettingsFile {
    std::string const m_strClassName;
    FileType const m_eFileType;
    std::string const m_strFilePath;
    int const m_iFileVersion;
    static std::map<std::string, std::map<std::string, SettingsElement::CreateMethod>>& getMap();
protected:
    SettingsFile(std::string const& a_strClassName, FileType a_eFileType, std::string const& a_strFilePath, int a_iFileVersion);
public:
    virtual ~SettingsFile() {}
    using CreateMethod = std::unique_ptr<SettingsFile>(*)();
    std::string getClassName() const;
    FileType getFileType() const;
    std::string getFilePath() const;
    int getFileVersion() const;

    static bool register_settings(char const* a_szFile, char const* a_szPath, SettingsElement::CreateMethod a_pCreateMethod);
    static std::map<std::string, emb::settings::SettingsElement::CreateMethod>& getElementsMap(std::string const& a_strFileClass);
};

template<typename Class, char const* ClassName, emb::settings::FileType Type, char const* TypeName, char const* Path, int Version>
class TSettingsFile : public SettingsFile {
protected:
    static bool registered;
    virtual void Register() noexcept = 0;
public:
    static char const* Name;
    static char const* FilePath;
    static emb::settings::FileType const FileType;
    static int const FileVersion;
    TSettingsFile() : SettingsFile{ClassName, static_cast<emb::settings::FileType>(Type), Path, Version} {}
    virtual ~TSettingsFile() {}
    static std::unique_ptr<SettingsFile> CreateMethod() { return std::make_unique<Class>(); }
    static std::map<std::string, emb::settings::SettingsElement::CreateMethod>& getElementsMap() { return getElementsMap(ClassName); }
    static void read_linked() {
        for (auto const& elm : getElementsMap()) {
            elm.second()->read_linked();
        }
    }
    static void write_linked() {
        for (auto const& elm : getElementsMap()) {
            elm.second()->write_linked();
        }
    }
private:
    using SettingsFile::getElementsMap;
};
template<typename Class, char const* ClassName, FileType Type, char const* TypeName, char const* Path, int Version>
bool TSettingsFile<Class, ClassName, Type, TypeName, Path, Version>::registered = register_file(ClassName, Class::CreateMethod);
template<typename Class, char const* ClassName, FileType Type, char const* TypeName, char const* Path, int Version>
char const* TSettingsFile<Class, ClassName, Type, TypeName, Path, Version>::Name{ ClassName };
template<typename Class, char const* ClassName, FileType Type, char const* TypeName, char const* Path, int Version>
char const* TSettingsFile<Class, ClassName, Type, TypeName, Path, Version>::FilePath{ Path };
template<typename Class, char const* ClassName, FileType Type, char const* TypeName, char const* Path, int Version>
emb::settings::FileType const TSettingsFile<Class, ClassName, Type, TypeName, Path, Version>::FileType{ Type };
template<typename Class, char const* ClassName, FileType Type, char const* TypeName, char const* Path, int Version>
int const TSettingsFile<Class, ClassName, Type, TypeName, Path, Version>::FileVersion{ Version };

std::map<std::string, SettingsFile::CreateMethod>& getFilesMap();

bool register_file(std::string const& a_strName, SettingsFile::CreateMethod a_pCreateMethod);


namespace internal {

/**
 * @brief Struct that give access to a file ptree
 */
struct SettingsFileInfo {
    std::string /*const*/ strFilename{};
    FileType eFileType{};
    boost::property_tree::ptree tree{};

    struct Deleter {
        void operator()(SettingsFileInfo* a_pObj);
    };
    using Ptr = std::unique_ptr<SettingsFileInfo, Deleter>;

    static Ptr getFileInfo(std::unique_ptr<SettingsFile>);
};

}


} }

#include "EmbSettings.impl"
