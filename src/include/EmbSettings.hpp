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
 * @brief Declare a scalar setting inside a previously declared setting file
 * @param _name     Name of the class representing the setting
 * @param _type     Data type of the setting
 * @param _file     Class name of the file used to save the setting
 * @param _key      Key string representing the position of the setting in the file (using boost property_tree synthax)
 * @param ...       Optionnal default value of the setting if not found in the file (if not provided default value is {} )
 */
#define EMBSETTINGS_SCALAR(_name, _type, _file, _key, ...)                                                                                  \
namespace EmbSettings_Private { namespace _name {                                                                                           \
    inline char NameStr[]{ #_name };                                                                                                        \
    inline char TypeStr[]{ #_type };                                                                                                        \
    inline char KeyStr[]{ _key };                                                                                                           \
    inline _type Default{ __VA_ARGS__ };                                                                                                    \
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

/**
 * @brief Declare a vector setting inside a previously declared setting file
 * @param _name     Name of the class representing the setting
 * @param _type     Base data type of the setting. The final setting's data type is std::vector<_type>
 * @param _file     Class name of the file used to save the setting
 * @param _key      Key string representing the position of the setting in the file (using boost property_tree synthax)
 */
#define EMBSETTINGS_VECTOR(_name, _type, _file, _key)                                                                                       \
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
 * @brief Declare a map setting inside a previously declared setting file
 * @param _name     Name of the class representing the setting
 * @param _type     Base data type of the setting. The final setting's data type is std::map<std::string,_type>
 * @param _file     Class name of the file used to save the setting
 * @param _key      Key string representing the position of the setting in the file (using boost property_tree synthax)
 */
#define EMBSETTINGS_MAP(_name, _type, _file, _key)                                                                                          \
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

namespace emb {
    namespace settings {
        /**
         * @brief Version of the EmbSettings library
         * @return char const* BVersion as a string
         */
        char const* version();

        /**
         * @brief Type of a settings file
         */
        enum class FileType {
            XML,    ///< XML file
            JSON,   ///< JSON file
            INI     ///< INI file
        };
        char const* str(FileType a_eFileType);

        /**
         * @brief Defines a jocker value, that can be used in settings files' path
         * @param a_strJocker   Name of the jocker, without the @{...} pattern
         * @param a_strValue    Value that will replace the pattern @{a_strJocker}
         */
        void set_jocker(std::string const& a_strJocker, std::string const& a_strValue);

        /**
         * @brief Defines the name of the node used to store each element of a vector in XML format. Default is value
         * @details In XML format, vectors are stored as <vector_name><value>value1</value><value>value2</value></vector_name>
         *          That method affects the <value> and </value> nodes
         * @param a_strName     Name of node used in XML vector
         */
        void set_xml_vector_element_name(std::string const& a_strName);

        /**
         * @brief Get the file names list object
         * 
         * @return std::vector<std::string> 
         */
        std::vector<std::string> get_file_names_list();

        /**
         * @brief Get the element names list object
         * 
         * @param a_strFileName 
         * @return std::vector<std::string> 
         */
        std::vector<std::string> get_element_names_list(std::string const& a_strFileName);

        namespace internal {

            /**
             * @brief Function pointer returning a unique pointer to a T
             */
            template<typename T>
            using creation_method = std::unique_ptr<T>(*)();

            /**
             * @brief Base class of a each setting element
             */
            class SettingElement {
            // public methods
            public:
                /**
                 * @brief Get the setting element's name
                 * @return std::string  Name of the setting element
                 */
                std::string get_name() const;
                /**
                 * @brief Get the setting element's type
                 * @return std::string  Type of the setting element
                 */
                std::string get_type() const;
                /**
                 * @brief Get the setting element's file name
                 * @return std::string  File name of the setting element
                 */
                std::string get_file() const;
                /**
                 * @brief Get the setting element's Key
                 * @return std::string  Key of the setting element
                 */
                std::string get_key() const;
            
            // protected types
            protected:
                friend std::default_delete<SettingElement>;
            
            // protected methods
            protected:
                /**
                 * @brief Construct a new Setting Element object
                 * @param a_strName     Class name of the setting element
                 * @param a_strType     Type name of the setting element
                 * @param a_strFile     Name of the file where the setting element is stored
                 * @param a_strKey      Key of the setting element in the file
                 */
                SettingElement(std::string const& a_strName, std::string const& a_strType, std::string const& a_strFile, std::string const& a_strKey);
                /**
                 * @brief Destroy the Setting Element object
                 */
                virtual ~SettingElement();
                /**
                 * @brief Read a setting value
                 * @tparam Type         Type of the setting
                 * @param a_strFile     Name of the file where the setting element is stored
                 * @param a_strElement  Name of the setting element in the file
                 * @param a_tDefault    Default value to return if the setting element is not found in file
                 * @return Type         Read value or \c a_tDefault it not found
                 */
                template<typename Type>
                static Type read_setting(std::string const& a_strFile, std::string const& a_strElement, Type const& a_tDefault);
                /**
                 * @brief Write a setting value
                 * @tparam Type         Type of the setting
                 * @param a_strFile     Name of the file where the setting element is stored
                 * @param a_strElement  Name of the setting element in the file
                 * @param a_tNew        Value to write
                 */
                template<typename Type>
                static void write_setting(std::string const& a_strFile, std::string const& a_strElement, Type const& a_tNew);
                /**
                 * @brief Link a setting value to a variable
                 * @tparam Type         Type of the setting
                 * @tparam Element      Element representing the setting
                 * @param a_strFile     Name of the file where the setting element is stored
                 * @param a_strElement  Name of the setting element in the file
                 * @param a_rtVariable   
                 */
                template<typename Type, typename Element>
                static void link_setting(std::string const& a_strFile, std::string const& a_strElement, Type& a_rtVariable);
                /**
                 * @brief 
                 * 
                 * @tparam Type 
                 * @param a_strFile 
                 * @param a_strElement  Name of the setting element in the file
                 * @return std::vector<Type> 
                 */
                template<typename Type>
                static std::vector<Type> read_setting_vector(std::string const& a_strFile, std::string const& a_strElement);
                /**
                 * @brief 
                 * 
                 * @tparam Type 
                 * @param a_strFile 
                 * @param a_strElement  Name of the setting element in the file
                 * @param a_tvecNew 
                 */
                template<typename Type>
                static void write_setting_vector(std::string const& a_strFile, std::string const& a_strElement, std::vector<Type> const& a_tvecNew);
                /**
                 * @brief 
                 * 
                 * @tparam Type 
                 * @param a_strFile 
                 * @param a_strElement  Name of the setting element in the file
                 * @param a_tNew 
                 */
                template<typename Type>
                static void add_setting_vector(std::string const& a_strFile, std::string const& a_strElement, Type const& a_tNew);
                /**
                 * @brief 
                 * 
                 * @tparam Type 
                 * @param a_strFile 
                 * @param a_strElement  Name of the setting element in the file
                 * @return std::map<std::string, Type> 
                 */
                template<typename Type>
                static std::map<std::string, Type> read_setting_map(std::string const& a_strFile, std::string const& a_strElement);
                /**
                 * @brief 
                 * 
                 * @tparam Type 
                 * @param a_strFile 
                 * @param a_strElement  Name of the setting element in the file
                 * @param a_tmapNew 
                 */
                template<typename Type>
                static void write_setting_map(std::string const& a_strFile, std::string const& a_strElement, std::map<std::string, Type> const& a_tmapNew);
                /**
                 * @brief Set the setting map object
                 * 
                 * @tparam Type 
                 * @param a_strFile 
                 * @param a_strElement  Name of the setting element in the file
                 * @param a_strK 
                 * @param a_tNew 
                 */
                template<typename Type>
                static void set_setting_map(std::string const& a_strFile, std::string const& a_strElement, std::string const& a_strK, Type const& a_tNew);
            
                /**
                 * @brief 
                 */
                virtual void _register_() noexcept = 0;
                static bool register_element(char const* a_szFile, char const* a_szElement, creation_method<SettingElement> a_funcCreationMethod);
            
            private:
                std::string const m_strName;
                std::string const m_strType;
                std::string const m_strFile;
                std::string const m_strKey;
            };

            /**
             * @brief Base class of a scalar setting element
             * 
             * @tparam Name         Class name of the element
             * @tparam NameStr      Class name of the element, as a string
             * @tparam Type         Type of the element
             * @tparam TypeStr      Type of the element, as a string
             * @tparam File         Class name of the file when the element must be stored
             * @tparam KeyStr       Key locating the element inside the file
             * @tparam Default      Default value of the element, if not present in the file
             */
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
                class TSettingScalar
                        : public SettingElement {
                // public methods
                public:
                    /**
                     * @brief Construct a new TSettingScalar object
                     */
                    TSettingScalar()
                        : SettingElement{ _NameStr, _TypeStr, _File::Name, _KeyStr }
                    {}
                    /**
                     * @brief Destroy the TSettingScalar object
                     */
                    virtual ~TSettingScalar()
                    {}
                    /**
                     * @brief Read the setting element
                     * @return Type     Value of the setting element
                     */
                    static _Type read() {
                        return read_setting<_Type>(_File::Name, _NameStr, *_Default);
                    }
                    /**
                     * @brief Write the setting element
                     * @param a_tVal    New value of the setting element
                     */
                    static void write(_Type const& a_tVal) {
                        write_setting<_Type>(_File::Name, _NameStr, a_tVal);
                    }
                    /**
                     * @brief Link the setting element to a variable
                     * 
                     * @param a_rtVar   Variable to link the setting element to
                     */
                    static void link(_Type& a_rtVar) {
                        link_setting<_Type, _Name>(_File::Name, _NameStr, a_rtVar);
                    }

                // protected methods
                protected:
                    /**
                     * @brief Create an object of type \c Name
                     * @return std::unique_ptr<SettingsElement> Newly created object
                     */
                    static std::unique_ptr<SettingElement> _create_() {
                        return std::make_unique<_Name>();
                    }

                // protected attributes
                protected:
                    static bool s_bRegistered;
            };
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            bool TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::s_bRegistered =
                SettingElement::register_element(_File::Name, _NameStr, _Name::_create_);

            /**
             * @brief Base class of a vector setting element
             * 
             * @tparam Name         Class name of the element
             * @tparam NameStr      Class name of the element, as a string
             * @tparam Type         Type of the element
             * @tparam TypeStr      Type of the element, as a string
             * @tparam File         Class name of the file when the element must be stored
             * @tparam KeyStr       Key locating the element inside the file
             */
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr>
                class TSettingVector
                        : public SettingElement {
                // public methods
                public:
                    /**
                     * @brief Construct a new TSettingVector object
                     */
                    TSettingVector()
                        : SettingElement{ _NameStr, _TypeStr, _File::Name, _KeyStr }
                    {}
                    /**
                     * @brief Destroy the TSettingVector object
                     */
                    virtual ~TSettingVector()
                    {}
                    /**
                     * @brief Read the vector setting element
                     * @return Type     Value of the setting element
                     */
                    static std::vector<_Type> read() {
                        return read_setting_vector<_Type>(_File::Name, _NameStr);
                    }
                    /**
                     * @brief Write the vector setting element
                     * @param a_tvecVal New value of the setting element
                     */
                    static void write(std::vector<_Type> const& a_tvecVal) {
                        write_setting_vector<_Type>(_File::Name, _NameStr, a_tvecVal);
                    }
                    /**
                     * @brief Add a value to the vector setting element
                     * @param a_tVal    New value of the setting element
                     */
                    static void add(_Type const& a_tVal) {
                        add_setting_vector<_Type>(_File::Name, _NameStr, a_tVal);
                    }
                    /**
                     * @brief Link the vector setting element to a vector variable
                     * 
                     * @param a_rtvecVal Variable to link the setting element to
                     */
                    static void link(std::vector<_Type>& a_rtvecVal) {
                        link_setting<std::vector<_Type>, _Name>(_File::Name, _NameStr, a_rtvecVal);
                    }

                // protected methods
                protected:
                    /**
                     * @brief Create an object of type \c Name
                     * @return std::unique_ptr<SettingsElement> Newly created object
                     */
                    static std::unique_ptr<SettingElement> _create_() {
                        return std::make_unique<_Name>();
                    }

                // protected attributes
                protected:
                    static bool s_bRegistered;
            };
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr>
            bool TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr>::s_bRegistered =
                SettingElement::register_element(_File::Name, _NameStr, _Name::_create_);
            
            /**
             * @brief Base class of a map setting element
             * 
             * @tparam Name         Class name of the element
             * @tparam NameStr      Class name of the element, as a string
             * @tparam Type         Type of the element
             * @tparam TypeStr      Type of the element, as a string
             * @tparam File         Class name of the file when the element must be stored
             * @tparam KeyStr       Key locating the element inside the file
             */
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr>
                class TSettingMap
                        : public SettingElement {
                // public methods
                public:
                    /**
                     * @brief Construct a new TSettingMap object
                     */
                    TSettingMap()
                        : SettingElement{ _NameStr, _TypeStr, _File::Name, _KeyStr }
                    {}
                    /**
                     * @brief Destroy the TSettingMap object
                     */
                    virtual ~TSettingMap()
                    {}
                    /**
                     * @brief Read the map setting element
                     * @return Type     Value of the map setting element
                     */
                    static std::map<std::string, _Type> read() {
                        return read_setting_map<_Type>(_File::Name, _NameStr);
                    }
                    /**
                     * @brief Write the map setting element
                     * @param a_tmapVal New value of the map setting element
                     */
                    static void write(std::map<std::string, _Type> const& a_tmapVal) {
                        write_setting_map<_Type>(_File::Name, _NameStr, a_tmapVal);
                    }
                    /**
                     * @brief Set the map setting element value at a given key
                     * @param a_strKey  Key of the map setting element
                     * @param a_tVal    New value of map the setting element
                     */
                    static void set(std::string const& a_strKey, _Type const& a_tVal) {
                        set_setting_map<_Type>(_File::Name, _NameStr, a_strKey, a_tVal);
                    }
                    /**
                     * @brief Link the setting element to a variable
                     * 
                     * @param a_rtmapVal Variable to link the setting element to
                     */
                    static void link(std::map<std::string, _Type>& a_rtmapVal) {
                        link_setting<std::map<std::string, _Type>, _Name>(_File::Name, _NameStr, a_rtmapVal);
                    }

                // protected methods
                protected:
                    /**
                     * @brief Create an object of type \c Name
                     * @return std::unique_ptr<SettingsElement> Newly created object
                     */
                    static std::unique_ptr<SettingElement> _create_() {
                        return std::make_unique<_Name>();
                    }

                // protected attributes
                protected:
                    static bool s_bRegistered;
            };
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr>
            bool TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr>::s_bRegistered =
                SettingElement::register_element(_File::Name, _NameStr, _Name::_create_);
            
            /**
             * @brief Base class of a each settings file
             */
            class SettingsFile {
            // public methods
            public:
                /**
                 * @brief Get the settings file's name
                 * @return std::string  Name of the settings file
                 */
                std::string get_name() const;
                /**
                 * @brief Get the settings file's type
                 * @return std::string  Type of the settings file
                 */
                FileType get_type() const;
                /**
                 * @brief Get the settings file's file path
                 * @return std::string  File path of the settings file
                 */
                std::string get_path() const;
                /**
                 * @brief Get the settings file's version
                 * @return std::string  Version of the settings file
                 */
                int get_version() const;
            
            // protected types
            protected:
                friend std::default_delete<SettingsFile>;

            // protected methods
            protected:
                /**
                 * @brief Construct a new Settings File object
                 * 
                 * @param a_strName 
                 * @param a_eType 
                 * @param a_strPath 
                 * @param a_iVersion 
                 */
                SettingsFile(std::string const& a_strName, FileType a_eType, std::string const& a_strPath, int a_iVersion);
                /**
                 * @brief Destroy the Settings File object
                 * 
                 */
                virtual ~SettingsFile();
                /**
                 * @brief 
                 */
                virtual void _register_() noexcept = 0;
                static bool register_file(char const* a_szFile, creation_method<SettingsFile> a_funcCreationMethod);

            // private attributes
            private:
                std::string const m_strName;
                FileType const m_eType;
                std::string const m_strPath;
                int const m_iVersion;
            };

            /**
             * @brief Base class of settings file
             * 
             * @tparam Name          Class name of the file
             * @tparam NameStr      Class name of the file, as a string
             * @tparam Type         Type of the file (from emb::settings::FileType)
             * @tparam PathStr      Path of the file on the system (may contain jocker in the form of @{jocker})
             * @tparam Version      Version of the file
             */
            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version>
            class TSettingsFile : public SettingsFile {
            // public attributes
            public:
                static char const* Name;
                static char const* Path;
                static emb::settings::FileType const Type;
                static int const Version;

            // public methods
            public:
                TSettingsFile()
                    : SettingsFile{ _NameStr, _Type, _PathStr, _Version }
                {}
                virtual ~TSettingsFile()
                {}

                static void begin() {

                }
                static void commit() {
                    
                }
                static void abort() {
                    
                }
                static void read_linked() {
                    //for (auto const& elm : getElementsMap()) {
                    //    elm.second()->read_linked();
                    //}
                }
                static void write_linked() {
                    //for (auto const& elm : getElementsMap()) {
                    //    elm.second()->write_linked();
                    //}
                }
                static void backup_to() {
                    
                }
                static void restore_from() {
                    
                }
            
            // protected methods
            protected:
                /**
                 * @brief Create an object of type \c Name
                 * @return std::unique_ptr<SettingsFile> Newly created object
                 */
                static std::unique_ptr<SettingsFile> _create_() {
                    return std::make_unique<_Name>();
                }

            // protected attributes
            protected:
                static bool s_bRegistered;
            };
            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version>
            bool TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version>::s_bRegistered = SettingsFile::register_file(_NameStr, _Name::_create_);
            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version>
            char const* TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version>::Name{ _NameStr };
            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version>
            char const* TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version>::Path{ _PathStr };
            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version>
            emb::settings::FileType const TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version>::Type{ _Type };
            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version>
            int const TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version>::Version{ _Version };

            struct tree_ptr_deleter {
                void operator()(boost::property_tree::ptree* a_pObj);
            };
            using tree_ptr = std::unique_ptr<boost::property_tree::ptree, tree_ptr_deleter>;
            tree_ptr get_tree(std::string const& a_strFileName, std::string const& a_strElementName);

            std::string& xml_vector_element_name();
            void remove_tree(boost::property_tree::ptree & a_rTree, std::string const& a_strKeyToRemove);
        }

        /**
         * @brief Get the file object
         * 
         * @param a_strFileName 
         * @return std::unique_ptr<SettingsFile> 
         */
        std::unique_ptr<internal::SettingsFile> get_file(std::string const& a_strFileName);

        /**
         * @brief Get the element object
         * 
         * @param a_strFileName 
         * @param a_strElementName 
         * @return std::unique_ptr<SettingsElement> 
         */
        std::unique_ptr<internal::SettingElement> get_element(std::string const& a_strFileName, std::string const& a_strElementName);
    }
}

#include "EmbSettings_impl.hpp"