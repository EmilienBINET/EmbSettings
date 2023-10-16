#pragma once

//////////////////////////////////////////////////////////////////////
///// BASE MACROS TO CREATE MACRO OVERLOADS                      /////
//////////////////////////////////////////////////////////////////////

// https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
// https://stackoverflow.com/questions/5134523/msvc-doesnt-expand-va-args-correctly

#define EMBSETTINGS_INTERNAL_EXPAND(x) x

// get number of arguments with __NARG__
#define EMBSETTINGS_INTERNAL_NARG(...)  EMBSETTINGS_INTERNAL_EXPAND(EMBSETTINGS_INTERNAL_NARG_I(__VA_ARGS__,EMBSETTINGS_INTERNAL_RSEQ_N()))
#define EMBSETTINGS_INTERNAL_NARG_I(...) EMBSETTINGS_INTERNAL_EXPAND(EMBSETTINGS_INTERNAL_ARG_N(__VA_ARGS__))
#define EMBSETTINGS_INTERNAL_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
#define EMBSETTINGS_INTERNAL_RSEQ_N() 9,8,7,6,5,4,3,2,1,0

// general definition for any function name
#define EMBSETTINGS_INTERNAL_CONCAT_I(name, n) EMBSETTINGS_INTERNAL_EXPAND(name##n)
#define EMBSETTINGS_INTERNAL_CONCAT(name, n) EMBSETTINGS_INTERNAL_EXPAND(EMBSETTINGS_INTERNAL_CONCAT_I(name, n))
#define EMBSETTINGS_INTERNAL_VFUNC(func, ...) EMBSETTINGS_INTERNAL_EXPAND(EMBSETTINGS_INTERNAL_CONCAT(func, EMBSETTINGS_INTERNAL_NARG(__VA_ARGS__)) (__VA_ARGS__))

//////////////////////////////////////////////////////////////////////
///// INTERNAL MACROS TO DECLARE SETTINGS FILE                   /////
//////////////////////////////////////////////////////////////////////

/**
 * @brief Declare a file that can contain settings
 * @param _name     Name of the class representing the file
 * @param _type     Type of file (amongst \c emb::settings::FileType enumeration, without the scope: e.g. XML )
 * @param _path     Path of the file on the system. Jokers can be used with the format @{joker}
 */
#define EMBSETTINGS_INTERNAL_FILE_3(_name, _type, _path)                                                                                    \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    inline char NameStr[]{ #_name };                                                                                                        \
    inline char PathStr[]{ _path };                                                                                                         \
} }                                                                                                                                         \
class _name final : public emb::settings::internal::TSettingsFile<                                                                          \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::NameStr,                                                                                                \
        emb::settings::FileType::_type,                                                                                                     \
        EmbSettings_Private::_name::PathStr                                                                                                 \
    > {                                                                                                                                     \
    void _register_() noexcept override { s_bRegistered = s_bRegistered; }                                                                  \
};

/**
 * @brief Declare a file that can contain settings
 * @param _name     Name of the class representing the file
 * @param _type     Type of file (amongst \c emb::settings::FileType enumeration, without the scope: e.g. XML )
 * @param _path     Path of the file on the system. Jokers can be used with the format @{joker}
 * @param _version  Current version of the file
 */
#define EMBSETTINGS_INTERNAL_FILE_4(_name, _type, _path, _version)                                                                          \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    inline char NameStr[]{ #_name };                                                                                                        \
    inline char PathStr[]{ _path };                                                                                                         \
} }                                                                                                                                         \
class _name final : public emb::settings::internal::TSettingsFile<                                                                          \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::NameStr,                                                                                                \
        emb::settings::FileType::_type,                                                                                                     \
        EmbSettings_Private::_name::PathStr,                                                                                                \
        _version                                                                                                                            \
    > {                                                                                                                                     \
    void _register_() noexcept override { s_bRegistered = s_bRegistered; }                                                                  \
};

/**
 * @brief Declare a file that can contain settings
 * @param _name     Name of the class representing the file
 * @param _type     Type of file (amongst \c emb::settings::FileType enumeration, without the scope: e.g. XML )
 * @param _path     Path of the file on the system. Jokers can be used with the format @{joker}
 * @param _version  Current version of the file
 * @param _version_clbk Function pointer to call when versions mismatch
 */
#define EMBSETTINGS_INTERNAL_FILE_5(_name, _type, _path, _version, _version_clbk)                                                           \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    inline char NameStr[]{ #_name };                                                                                                        \
    inline char PathStr[]{ _path };                                                                                                         \
} }                                                                                                                                         \
class _name final : public emb::settings::internal::TSettingsFile<                                                                          \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::NameStr,                                                                                                \
        emb::settings::FileType::_type,                                                                                                     \
        EmbSettings_Private::_name::PathStr,                                                                                                \
        _version,                                                                                                                           \
        _version_clbk                                                                                                                       \
    > {                                                                                                                                     \
    void _register_() noexcept override { s_bRegistered = s_bRegistered; }                                                                  \
};

//////////////////////////////////////////////////////////////////////
///// INTERNAL MACROS TO DECLARE SETTING ELEMENT SCALAR          /////
//////////////////////////////////////////////////////////////////////

/**
 * @brief Declare a scalar setting inside a previously declared setting file
 * @param _name     Name of the class representing the setting
 * @param _type     Data type of the setting
 * @param _file     Class name of the file used to save the setting
 * @param _key      Key string representing the position of the setting in the file (using boost property_tree synthax)
 */
#define EMBSETTINGS_INTERNAL_SCALAR_4(_name, _type, _file, _key)                                                                            \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    inline char NameStr[]{ #_name };                                                                                                        \
    inline char TypeStr[]{ #_type };                                                                                                        \
    inline char KeyStr[]{ _key };                                                                                                           \
} }                                                                                                                                         \
class _name final : public emb::settings::internal::TSettingScalar<                                                                         \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::NameStr,                                                                                                \
        _type,                                                                                                                              \
        EmbSettings_Private::_name::TypeStr,                                                                                                \
        _file,                                                                                                                              \
        EmbSettings_Private::_name::KeyStr                                                                                                  \
    > {                                                                                                                                     \
    void _register_() noexcept override { s_bRegistered = s_bRegistered; }                                                                  \
};

/**
 * @brief Declare a scalar setting inside a previously declared setting file
 * @param _name     Name of the class representing the setting
 * @param _type     Data type of the setting
 * @param _file     Class name of the file used to save the setting
 * @param _key      Key string representing the position of the setting in the file (using boost property_tree synthax)
 * @param _default  Default value of the setting if not found in the file
 */
#define EMBSETTINGS_INTERNAL_SCALAR_5(_name, _type, _file, _key, _default)                                                                  \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    inline char NameStr[]{ #_name };                                                                                                        \
    inline char TypeStr[]{ #_type };                                                                                                        \
    inline char KeyStr[]{ _key };                                                                                                           \
    inline _type Default{ _default };                                                                                                       \
} }                                                                                                                                         \
class _name final : public emb::settings::internal::TSettingScalar<                                                                         \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::NameStr,                                                                                                \
        _type,                                                                                                                              \
        EmbSettings_Private::_name::TypeStr,                                                                                                \
        _file,                                                                                                                              \
        EmbSettings_Private::_name::KeyStr,                                                                                                 \
        &EmbSettings_Private::_name::Default                                                                                                \
    > {                                                                                                                                     \
    void _register_() noexcept override { s_bRegistered = s_bRegistered; }                                                                  \
};

//////////////////////////////////////////////////////////////////////
///// INTERNAL MACROS TO DECLARE SETTING ELEMENT VECTOR          /////
//////////////////////////////////////////////////////////////////////

/**
 * @brief Declare a vector setting inside a previously declared setting file
 * @param _name     Name of the class representing the setting
 * @param _type     Base data type of the setting. The final setting's data type is std::vector<_type>
 * @param _file     Class name of the file used to save the setting
 * @param _key      Key string representing the position of the setting in the file (using boost property_tree synthax)
 */
#define EMBSETTINGS_INTERNAL_VECTOR_4(_name, _type, _file, _key)                                                                            \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    inline char NameStr[]{ #_name };                                                                                                        \
    inline char TypeStr[]{ "std::vector<" #_type ">" };                                                                                     \
    inline char KeyStr[]{ _key };                                                                                                           \
} }                                                                                                                                         \
class _name final : public emb::settings::internal::TSettingVector<                                                                         \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::NameStr,                                                                                                \
        _type,                                                                                                                              \
        EmbSettings_Private::_name::TypeStr,                                                                                                \
        _file,                                                                                                                              \
        EmbSettings_Private::_name::KeyStr                                                                                                  \
    > {                                                                                                                                     \
    void _register_() noexcept override { s_bRegistered = s_bRegistered; }                                                                  \
};

/**
 * @brief Declare a vector setting inside a previously declared setting file
 * @param _name     Name of the class representing the setting
 * @param _type     Base data type of the setting. The final setting's data type is std::vector<_type>
 * @param _file     Class name of the file used to save the setting
 * @param _key      Key string representing the position of the setting in the file (using boost property_tree synthax)
 * @param _pdefault Pointer to a default value of the setting if not found in the file
 */
#define EMBSETTINGS_INTERNAL_VECTOR_5(_name, _type, _file, _key, _pdefault)                                                                 \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    inline char NameStr[]{ #_name };                                                                                                        \
    inline char TypeStr[]{ "std::vector<" #_type ">" };                                                                                     \
    inline char KeyStr[]{ _key };                                                                                                           \
} }                                                                                                                                         \
class _name final : public emb::settings::internal::TSettingVector<                                                                         \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::NameStr,                                                                                                \
        _type,                                                                                                                              \
        EmbSettings_Private::_name::TypeStr,                                                                                                \
        _file,                                                                                                                              \
        EmbSettings_Private::_name::KeyStr,                                                                                                 \
        _pdefault                                                                                                                           \
    > {                                                                                                                                     \
    void _register_() noexcept override { s_bRegistered = s_bRegistered; }                                                                  \
};

//////////////////////////////////////////////////////////////////////
///// INTERNAL MACROS TO DECLARE SETTING ELEMENT MAP             /////
//////////////////////////////////////////////////////////////////////

/**
 * @brief Declare a map setting inside a previously declared setting file
 * @param _name     Name of the class representing the setting
 * @param _type     Base data type of the setting. The final setting's data type is std::map<std::string,_type>
 * @param _file     Class name of the file used to save the setting
 * @param _key      Key string representing the position of the setting in the file (using boost property_tree synthax)
 */
#define EMBSETTINGS_INTERNAL_MAP_4(_name, _type, _file, _key)                                                                               \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    inline char NameStr[]{ #_name };                                                                                                        \
    inline char TypeStr[]{ "std::map<std::string," #_type ">" };                                                                            \
    inline char KeyStr[]{ _key };                                                                                                           \
} }                                                                                                                                         \
class _name final : public emb::settings::internal::TSettingMap<                                                                            \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::NameStr,                                                                                                \
        _type,                                                                                                                              \
        EmbSettings_Private::_name::TypeStr,                                                                                                \
        _file,                                                                                                                              \
        EmbSettings_Private::_name::KeyStr                                                                                                  \
    > {                                                                                                                                     \
    void _register_() noexcept override { s_bRegistered = s_bRegistered; }                                                                  \
};

/**
 * @brief Declare a map setting inside a previously declared setting file
 * @param _name     Name of the class representing the setting
 * @param _type     Base data type of the setting. The final setting's data type is std::map<std::string,_type>
 * @param _file     Class name of the file used to save the setting
 * @param _key      Key string representing the position of the setting in the file (using boost property_tree synthax)
 * @param _pdefault Pointer to a default value of the setting if not found in the file
 */
#define EMBSETTINGS_INTERNAL_MAP_5(_name, _type, _file, _key, _pdefault)                                                                    \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    inline char NameStr[]{ #_name };                                                                                                        \
    inline char TypeStr[]{ "std::map<std::string," #_type ">" };                                                                            \
    inline char KeyStr[]{ _key };                                                                                                           \
} }                                                                                                                                         \
class _name final : public emb::settings::internal::TSettingMap<                                                                            \
        _name,                                                                                                                              \
        EmbSettings_Private::_name::NameStr,                                                                                                \
        _type,                                                                                                                              \
        EmbSettings_Private::_name::TypeStr,                                                                                                \
        _file,                                                                                                                              \
        EmbSettings_Private::_name::KeyStr,                                                                                                 \
        _pdefault                                                                                                                           \
    > {                                                                                                                                     \
    void _register_() noexcept override { s_bRegistered = s_bRegistered; }                                                                  \
};
