#pragma once
#include "EmbSettings_macro.hpp"
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include <boost/property_tree/ptree.hpp>

/**
 * @brief Declare a file that can contain settings
 * @param 1 [mandatory] Name of the class representing the file
 * @param 2 [mandatory] Type of file (amongst \c emb::settings::FileType enumeration, without the scope: e.g. XML )
 * @param 3 [mandatory] Path of the file on the system. Jockers can be used with the format @{jocker}
 * @param 4 [optional]  Current version of the file
 * @param 5 [optional]  Function pointer to call when versions mismatch
 */
#define EMBSETTINGS_FILE(...) EMBSETTINGS_VFUNC(EMBSETTINGS_INTERNAL_FILE_, __VA_ARGS__)

/**
 * @brief Declare a scalar setting inside a previously declared setting file
 * @param 1 [mandatory] Name of the class representing the setting
 * @param 2 [mandatory] Data type of the setting
 * @param 3 [mandatory] Class name of the file used to save the setting
 * @param 4 [mandatory] Key string representing the position of the setting in the file (using boost property_tree synthax)
 * @param 5 [optional]  Default value of the setting if not found in the file (if not provided default value is _type{})
 */
#define EMBSETTINGS_SCALAR(...) EMBSETTINGS_VFUNC(EMBSETTINGS_INTERNAL_SCALAR_, __VA_ARGS__)

/**
 * @brief Declare a vector setting inside a previously declared setting file
 * @param 1 [mandatory] Name of the class representing the setting
 * @param 2 [mandatory] Base data type of the setting. The final setting's data type is std::vector<_type>
 * @param 3 [mandatory] Class name of the file used to save the setting
 * @param 4 [mandatory] Key string representing the position of the setting in the file (using boost property_tree synthax)
 * @param 5 [optional]  Pointer to a default value of the setting if not found in the file
 */
#define EMBSETTINGS_VECTOR(...) EMBSETTINGS_VFUNC(EMBSETTINGS_INTERNAL_VECTOR_, __VA_ARGS__)

/**
 * @brief Declare a map setting inside a previously declared setting file
 * @param 1 [mandatory] Name of the class representing the setting
 * @param 2 [mandatory] Base data type of the setting. The final setting's data type is std::map<std::string,_type>
 * @param 3 [mandatory] Class name of the file used to save the setting
 * @param 4 [mandatory] Key string representing the position of the setting in the file (using boost property_tree synthax)
 * @param 5 [optional]  Pointer to a default value of the setting if not found in the file
 */
#define EMBSETTINGS_MAP(...) EMBSETTINGS_VFUNC(EMBSETTINGS_INTERNAL_MAP_, __VA_ARGS__)

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
         * @brief How default values are handled
         */
        enum class DefaultMode {
            DefaultValueIfAbsentFromFile,   ///< A setting element has its default value if is is not present in the associated settings file
            DefaultValueWrittenInFile,      ///< A setting element has its default value written in the associated settings file
        };
        char const* str(DefaultMode a_eDefaultMode);

        /**
         * @brief Defines a jocker value, that can be used in settings files' path
         * @param a_strJocker   Name of the jocker, without the @{...} pattern
         * @param a_strValue    Value that will replace the pattern @{a_strJocker}
         */
        void set_jocker(std::string const& a_strJocker, std::string const& a_strValue);

        /**
         * @brief Defines the name of the node used to store the settings file version. Default is version
         * @details That name then becomes invalid as a key name
         * @param a_strName     Name of the node
         */
        void set_version_element_name(std::string const& a_strName);

        /**
         * @brief Defines the name of the node used to store each element of a vector in XML format. Default is value
         * @details In XML format, vectors are stored as <vector_name><value>value1</value><value>value2</value></vector_name>
         *          That method affects the <value> and </value> nodes
         * @param a_strName     Name of node used in XML vector
         */
        void set_xml_vector_element_name(std::string const& a_strName);

        /**
         * @brief Defines how the default values are handled. Must be called before any read or write operation
         * @param a_eDefaultMode New default mode
         */
        void set_default_value_mode(DefaultMode a_eDefaultMode);

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

        /**
         * @brief Type of the version callback that will be called when a settings file version changed (version is 0 if not defined)
         * @param a_iOldVersion Version read from the file
         * @param a_iNewNersion Version from the parameter description
         * @return bool         true to write the new version in the file. false to NOT write it
         */
        using version_clbk_t = bool(*)(int a_iOldVersion, int a_iNewNersion);

        /**
         * @brief The internal namespace contains elements that are not part of the public API and are not meant to be called directly
         */
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
                std::string get_name_m() const;
                /**
                 * @brief Get the setting element's type
                 * @return std::string  Type of the setting element
                 */
                std::string get_type_m() const;
                /**
                 * @brief Get the setting element's file name
                 * @return std::string  File name of the setting element
                 */
                std::string get_file_m() const;
                /**
                 * @brief Get the setting element's Key
                 * @return std::string  Key of the setting element
                 */
                std::string get_key_m() const;
                /**
                 * @brief Read the linked variables
                 */
                void read_linked_m() const;
                /**
                 * @brief Write the linked variables
                 */
                void write_linked_m() const;
                /**
                 * @brief Read the setting element as a string
                 * @return std::string Value of the element
                 */
                virtual std::string read_str_m() const = 0;
                /**
                 * @brief Indicate if the setting element has its default value
                 * @return true     the setting element has its default value
                 * @return false    otherwise
                 */
                virtual bool is_default_m() const { return false; }
                /**
                 * @brief Reset the setting element to its default value
                 */
                virtual void reset_m() const {}

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
                 * @brief Reset a setting element to its default value
                 * @tparam Element      Setting element to reset
                 */
                template<typename Element>
                static void reset_setting();
                /**
                 * @brief Indicate if a setting element has its default value
                 *
                 * @tparam Element      Setting element to test
                 * @return true         The setting element has its default value
                 * @return false        Otherwise
                 */
                template<typename Element>
                static bool is_default_setting();
                /**
                 * @brief link a variable to a setting element
                 * @param a_funcRead    Read function
                 * @param a_funcWrite   Write function
                 */
                void link_variable_m(std::function<void(void)> const& a_funcRead, std::function<void(void)> const& a_funcWrite);
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
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default = nullptr>
            class TSettingScalar
                    : public SettingElement {
            // public attributes
            public:
                static char const* Name;
                using Type = _Type;
                using File = _File;
                static char const* Key;
                static _Type const Default;

            // public methods
            public:
                /**
                 * @brief Construct a new TSettingScalar object
                 */
                TSettingScalar();
                /**
                 * @brief Destroy the TSettingScalar object
                 */
                virtual ~TSettingScalar();
                /**
                 * @brief Read the setting element
                 * @return Type     Value of the setting element
                 */
                static _Type read();
                /**
                 * @brief Write the setting element
                 * @param a_tVal    New value of the setting element
                 */
                static void write(_Type const& a_tVal);
                /**
                 * @brief Reset the setting element to its default value
                 */
                static void reset();
                /**
                 * @brief Indicate if setting element has its default value
                 * @return true     The setting element has its default value
                 * @return false    Otherwise
                 */
                static bool is_default();
                /**
                 * @brief Link the setting element to a variable
                 *
                 * @param a_rtVar   Variable to link the setting element to
                 */
                static void link(_Type& a_rtVar);

                std::string read_str_m() const override;

                bool is_default_m() const override;

                void reset_m() const override;

            // protected methods
            protected:
                /**
                 * @brief Create an object of type \c Name
                 * @return std::unique_ptr<SettingsElement> Newly created object
                 */
                static std::unique_ptr<SettingElement> _create_();

            // protected attributes
            protected:
                static bool s_bRegistered;
            };

            /**
             * @brief Base class of a vector setting element
             *
             * @tparam Name         Class name of the element
             * @tparam NameStr      Class name of the element, as a string
             * @tparam Type         Type of the element
             * @tparam TypeStr      Type of the element, as a string
             * @tparam File         Class name of the file when the element must be stored
             * @tparam KeyStr       Key locating the element inside the file
             * @tparam Default      Default value of the element, if not present in the file
             */
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default = nullptr>
            class TSettingVector
                    : public SettingElement {
            // public attributes
            public:
                static char const* Name;
                using Type = std::vector<_Type>;
                using File = _File;
                static char const* Key;
                static Type const Default;
            
            // public methods
            public:
                /**
                 * @brief Construct a new TSettingVector object
                 */
                TSettingVector();
                /**
                 * @brief Destroy the TSettingVector object
                 */
                virtual ~TSettingVector();
                /**
                 * @brief Read the vector setting element
                 * @return Type     Value of the setting element
                 */
                static std::vector<_Type> read();
                /**
                 * @brief Write the vector setting element
                 * @param a_tvecVal New value of the setting element
                 */
                static void write(std::vector<_Type> const& a_tvecVal);
                /**
                 * @brief Add a value to the vector setting element
                 * @param a_tVal    New value of the setting element
                 */
                static void add(_Type const& a_tVal);
                /**
                 * @brief Link the vector setting element to a vector variable
                 *
                 * @param a_rtvecVal Variable to link the setting element to
                 */
                static void link(std::vector<_Type>& a_rtvecVal);

                std::string read_str_m() const override;

            // protected methods
            protected:
                /**
                 * @brief Create an object of type \c Name
                 * @return std::unique_ptr<SettingsElement> Newly created object
                 */
                static std::unique_ptr<SettingElement> _create_();

            // protected attributes
            protected:
                static bool s_bRegistered;
            };

            /**
             * @brief Base class of a map setting element
             *
             * @tparam Name         Class name of the element
             * @tparam NameStr      Class name of the element, as a string
             * @tparam Type         Type of the element
             * @tparam TypeStr      Type of the element, as a string
             * @tparam File         Class name of the file when the element must be stored
             * @tparam KeyStr       Key locating the element inside the file
             * @tparam Default      Default value of the element, if not present in the file
             */
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default = nullptr>
            class TSettingMap
                    : public SettingElement {
            // public attributes
            public:
                static char const* Name;
                using Type = std::map<std::string, _Type>;
                using File = _File;
                static char const* Key;
                static Type const Default;
            
            // public methods
            public:
                /**
                 * @brief Construct a new TSettingMap object
                 */
                TSettingMap();
                /**
                  * @brief Destroy the TSettingMap object
                  */
                virtual ~TSettingMap();
                /**
                  * @brief Read the map setting element
                  * @return Type     Value of the map setting element
                  */
                static std::map<std::string, _Type> read();
                /**
                  * @brief Write the map setting element
                  * @param a_tmapVal New value of the map setting element
                  */
                static void write(std::map<std::string, _Type> const& a_tmapVal);
                /**
                  * @brief Set the map setting element value at a given key
                  * @param a_strKey  Key of the map setting element
                  * @param a_tVal    New value of map the setting element
                  */
                static void set(std::string const& a_strKey, _Type const& a_tVal);
                /**
                  * @brief Link the setting element to a variable
                  *
                  * @param a_rtmapVal Variable to link the setting element to
                  */
                static void link(std::map<std::string, _Type>& a_rtmapVal);

                std::string read_str_m() const override;

            // protected methods
            protected:
                /**
                  * @brief Create an object of type \c Name
                  * @return std::unique_ptr<SettingsElement> Newly created object
                  */
                static std::unique_ptr<SettingElement> _create_();

            // protected attributes
            protected:
                static bool s_bRegistered;
            };

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
                std::string get_name_m() const;
                /**
                 * @brief Get the settings file's type
                 * @return std::string  Type of the settings file
                 */
                FileType get_type_m() const;
                /**
                 * @brief Get the settings file's file path
                 * @return std::string  File path of the settings file
                 */
                std::string get_path_m() const;
                /**
                 * @brief Get the settings file's version
                 * @return std::string  Version of the settings file
                 */
                int get_version_m() const;
                /**
                 * @brief Get the settings file's version callback
                 *
                 * @return version_clbk_t Version callback of the settings file
                 */
                version_clbk_t get_version_clbk_m() const;

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
                 * @param a_pVersionClbk
                 */
                SettingsFile(std::string const& a_strName, FileType a_eType, std::string const& a_strPath, int a_iVersion, version_clbk_t a_pVersionClbk);
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
                version_clbk_t const m_pVersionClbk;
            };

            /**
             * @brief Base class of settings file
             *
             * @tparam Name         Class name of the file
             * @tparam NameStr      Class name of the file, as a string
             * @tparam Type         Type of the file (from emb::settings::FileType)
             * @tparam PathStr      Path of the file on the system (may contain jocker in the form of @{jocker})
             * @tparam Version      Version of the file
             * @tparam VersionClbk  Version callback
             */
            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version = 0, version_clbk_t _VersionClbk = nullptr>
            class TSettingsFile
                    : public SettingsFile {
            // public attributes
            public:
                static char const* Name;
                static char const* Path;
                static emb::settings::FileType const Type;
                static int const Version;
                static version_clbk_t const VersionClbk;

            // public methods
            public:
                TSettingsFile();
                virtual ~TSettingsFile();
                /**
                 * @brief Begin a transaction of the settings file.
                 * @details If a transaction was already begun, that function does nothing.
                 *          During a transaction, write operations are not written on the file.
                 */
                static void begin();
                /**
                 * @brief Commit the pending transaction of the settings file
                 * @details All the changes made to the property tree since the \c begin call are written to the file.
                 */
                static void commit();
                /**
                 * @brief Abort the pending transaction of the settings file
                 * @details All the changes made to the property tree since the \c begin call are lost.
                 *          The property tree is restored to the value is had before the \c begin call.
                 */
                static void abort();
                static void read_linked();
                static void write_linked();
                static bool backup_to(std::string const& a_strFolderName);
                static bool restore_from(std::string const& a_strFolderName);

            // protected methods
            protected:
                /**
                 * @brief Create an object of type \c Name
                 * @return std::unique_ptr<SettingsFile> Newly created object
                 */
                static std::unique_ptr<SettingsFile> _create_();

            // protected attributes
            protected:
                static bool s_bRegistered;
            };

            struct tree_ptr_deleter {
                void operator()(boost::property_tree::ptree* a_pObj);
            };
            using tree_ptr = std::unique_ptr<boost::property_tree::ptree, tree_ptr_deleter>;
            tree_ptr get_tree(std::string const& a_strFileName, std::string const& a_strElementName, bool a_bReadOnly);
            boost::optional<boost::property_tree::ptree&> get_sub_tree(tree_ptr const& a_pTree, std::string const& a_strKey, bool a_bCreate = false);

            std::string& xml_vector_element_name();
            emb::settings::DefaultMode& default_mode();
            void remove_tree(boost::property_tree::ptree & a_rTree, std::string const& a_strKeyToRemove);
            std::string stringify_tree(boost::property_tree::ptree const& a_Tree);

            /**
            * @brief Backs the current file up
            *
            * @param a_strFileName fileName
            * @param a_strFolderName destination folder
            * @return bool true is success, false if failure
            */
            bool backup_file(std::string const& a_strFileName, std::string const& a_strFolderName);

            /**
            * @brief Retrieves settings from a backup folder
            *
            * @param a_strFileName fileName
            * @param a_strFolderName src folder
            * @return bool true is success, false if failure
            */
            bool restore_file(std::string const& a_strFileName, std::string const& a_strFolderName);
            void begin_file_transaction(std::string const& a_strFileName);
            void commit_file_transaction(std::string const& a_strFileName);
            void abort_file_transaction(std::string const& a_strFileName);
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