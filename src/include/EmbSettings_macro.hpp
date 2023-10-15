#pragma once

//////////////////////////////////////////////////////////////////////
///// BASE MACROS TO CREATE MACRO OVERLOADS                      /////
//////////////////////////////////////////////////////////////////////

// https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
// https://stackoverflow.com/questions/5134523/msvc-doesnt-expand-va-args-correctly

#define EMBSETTINGS_EXPAND(x) x

// get number of arguments with __NARG__
#define EMBSETTINGS_NARG(...)  EMBSETTINGS_EXPAND(EMBSETTINGS_NARG_I(__VA_ARGS__,EMBSETTINGS_RSEQ_N()))
#define EMBSETTINGS_NARG_I(...) EMBSETTINGS_EXPAND(EMBSETTINGS_ARG_N(__VA_ARGS__))
#define EMBSETTINGS_ARG_N( \
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
     _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
     _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
     _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
     _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
     _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
     _61,_62,_63,N,...) N
#define EMBSETTINGS_RSEQ_N() \
     63,62,61,60,                   \
     59,58,57,56,55,54,53,52,51,50, \
     49,48,47,46,45,44,43,42,41,40, \
     39,38,37,36,35,34,33,32,31,30, \
     29,28,27,26,25,24,23,22,21,20, \
     19,18,17,16,15,14,13,12,11,10, \
     9,8,7,6,5,4,3,2,1,0

// general definition for any function name
#define EMBSETTINGS_CONCAT_I(name, n) EMBSETTINGS_EXPAND(name##n)
#define EMBSETTINGS_CONCAT(name, n) EMBSETTINGS_EXPAND(EMBSETTINGS_CONCAT_I(name, n))
#define EMBSETTINGS_VFUNC(func, ...) EMBSETTINGS_EXPAND(EMBSETTINGS_CONCAT(func, EMBSETTINGS_NARG(__VA_ARGS__)) (__VA_ARGS__))

//////////////////////////////////////////////////////////////////////
///// INTERNAL MACROS TO DECLARE SETTINGS FILE                   /////
//////////////////////////////////////////////////////////////////////

/**
 * @brief Declare a file that can contain settings
 * @param _name     Name of the class representing the file
 * @param _type     Type of file (amongst \c emb::settings::FileType enumeration, without the scope: e.g. XML )
 * @param _path     Path of the file on the system. Jockers can be used with the format @{jocker}
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
 * @param _path     Path of the file on the system. Jockers can be used with the format @{jocker}
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
 * @param _path     Path of the file on the system. Jockers can be used with the format @{jocker}
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
