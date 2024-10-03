#include "../include/EmbSettings.hpp"
#include "EmbSettings_impl.hpp"
#include <string>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS // Avoid warning
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <mutex>
#include <map>
#include <regex>
#include <iostream>
#include "filesystem.hpp"

#if 0 // 1 to debug registering
#define DEBUG_SELF_REGISTERING(_cmd) _cmd
#else
#define DEBUG_SELF_REGISTERING(_cmd)
#endif

using namespace std;

namespace {

    map<string, string>& jokers() {
        static map<string, string> jokers{};
        return jokers;
    }

    void parse_jokers(std::string& a_rstrFilePath) {
        for (auto const& elm : jokers()) {
            boost::replace_all(a_rstrFilePath, "@{" + elm.first + "}", elm.second);
        }
    }

    string& version_element_name() {
        static string version_element_name{ "version" };
        return version_element_name;
    }

    struct SettingElementInfo {
        emb::settings::internal::creation_method<emb::settings::internal::SettingElement> funcCreate{};
        std::function<void(void)> funcReadLinked{};
        std::function<void(void)> funcWriteLinked{};
    };

    struct SettingsFileInfo {
        emb::settings::internal::creation_method<emb::settings::internal::SettingsFile> funcCreate{};
        recursive_mutex mutex{};
        bool bTransactionPending{false};
        boost::property_tree::ptree backupTree{};
        boost::property_tree::ptree tree{};
        boost::property_tree::ptree* lockedTree{nullptr};
        map<string, SettingElementInfo> elm_info{};

        emb::settings::FileType eFileType{};
        string strFullFileName{};
        int iVersion{0};
        emb::settings::version_clbk_t pVersionClbk{nullptr};
        std::stringstream strFilecontent{};

        void read_file() {
            std::ifstream is(strFullFileName, std::ios::binary);
            if (is.is_open()) {
                std::stringstream buffer;
                strFilecontent.str(std::string()); // clear
                strFilecontent << is.rdbuf();
            }
            try {
                switch (eFileType) {
                case emb::settings::FileType::XML:
                    boost::property_tree::read_xml(strFilecontent, tree, boost::property_tree::xml_parser::trim_whitespace);
                    break;
                case emb::settings::FileType::JSON:
                    boost::property_tree::read_json(strFilecontent, tree);
                    break;
                case emb::settings::FileType::INI:
                    boost::property_tree::read_ini(strFilecontent, tree);
                    break;
                }
            }
            catch (...) {
                tree = decltype(tree)();
            }
            auto iOldVersion = tree.get<int>(version_element_name(), 0);
            if(iOldVersion != iVersion && pVersionClbk) {
                if(pVersionClbk(iOldVersion, iVersion)) {
                    tree.put<int>(version_element_name(), iVersion);
                    write_file();
                }
            }
        }

        void write_file() {
            try {
                std::stringstream strTmpFilecontent{};
                switch (eFileType) {
                case emb::settings::FileType::XML:
                    boost::property_tree::write_xml(strTmpFilecontent, tree,
                        boost::property_tree::xml_writer_settings<decltype(tree)::key_type>(' ', 4));
                    break;
                case emb::settings::FileType::JSON:
                    boost::property_tree::write_json(strTmpFilecontent, tree);
                    break;
                case emb::settings::FileType::INI:
                    boost::property_tree::write_ini(strTmpFilecontent, tree);
                    break;
                }
                if (strTmpFilecontent.str() != strFilecontent.str()) {
                    std::ofstream os(strFullFileName, std::ios::binary);
                    if (os.is_open()) {
                        os << strTmpFilecontent.str();
                    }
                }
                strFilecontent.str(strTmpFilecontent.str());
            }
            catch (...) {
            }
        }

        friend ostream& operator<<(ostream & a_streamOutput, SettingsFileInfo & a_stFileInfo) {
            a_stFileInfo.read_file();
            a_streamOutput << a_stFileInfo.strFilecontent.str();
            return a_streamOutput;
        }

        friend istream& operator>>(istream & a_streamInput, SettingsFileInfo & a_stFileInfo) {
            {
                std::ofstream os(a_stFileInfo.strFullFileName, std::ios::binary);
                if (os.is_open()) {
                    std::string strTemp{};
                    while(a_streamInput >> strTemp) {
                        os << strTemp;
                    }
                }
            }
            a_stFileInfo.read_file();
            return a_streamInput;
        }

        emb::settings::internal::tree_ptr lock_tree(bool a_bReadOnly) {
            mutex.lock();
            if(strFullFileName.empty()) {
                auto pFileInfo = funcCreate();
                eFileType = pFileInfo->get_type_m();
                strFullFileName = pFileInfo->get_path_m();
                iVersion = pFileInfo->get_version_m();
                pVersionClbk = pFileInfo->get_version_clbk_m();
                parse_jokers(strFullFileName);
                if(!bTransactionPending) {
                    read_file();
                }
            }
            if (bTransactionPending && a_bReadOnly) {
                lockedTree = &backupTree;
            }
            else {
                lockedTree = &tree;
            }
            return emb::settings::internal::tree_ptr{ lockedTree };
        }

        void unlock_tree() {
            if(!bTransactionPending) {
                write_file();
            }
            lockedTree = nullptr;
            mutex.unlock();
        }
    };

    map<string, SettingsFileInfo>& files_info() {
        static map<string, SettingsFileInfo> info;
        return info;
    }

}

namespace emb {
    namespace settings {

        char const* version() {
            return "0.1.0";
        }

        char const* str(FileType a_eFileType) {
            #define str_FileType_case(__elm) case FileType::__elm : return #__elm;
            switch (a_eFileType) {
                str_FileType_case(XML)
                str_FileType_case(JSON)
                str_FileType_case(INI)
            }
            return "FileType::?";
        }

        char const* str(DefaultMode a_eDefaultMode) {
            #define str_DefaultMode_case(__elm) case DefaultMode::__elm : return #__elm;
            switch (a_eDefaultMode) {
                str_DefaultMode_case(DefaultValueIfAbsentFromFile)
                str_DefaultMode_case(DefaultValueWrittenInFile)
            }
            return "DefaultMode::?";
        }

        void set_joker(std::string const& a_strJoker, std::string const& a_strValue) {
            jokers()[a_strJoker] = a_strValue;
        }

        void set_version_element_name(std::string const& a_strName) {
            version_element_name() = a_strName;
        }

        void set_xml_vector_element_name(std::string const& a_strName) {
            internal::xml_vector_element_name() = a_strName;
        }

        void set_default_value_mode(DefaultMode a_eDefaultMode) {
            internal::default_mode() = a_eDefaultMode;
        }

        std::vector<std::string> get_file_names_list() {
            vector<string> vecFiles{};
            for (auto const& file : files_info()) {
                vecFiles.push_back(file.first);
            }
            return vecFiles;
        }

        std::vector<std::string> get_element_names_list(std::string const& a_strFileName) {
            vector<string> vecElements{};
            if (auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                for (auto const& elm : itFile->second.elm_info) {
                    vecElements.push_back(elm.first);
                }
            }
            return vecElements;
        }

        std::unique_ptr<internal::SettingsFile> get_file(std::string const& a_strFileName) {
            if (auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                auto const& func = itFile->second.funcCreate;
                if(func) {
                    return func();
                }
            }
            return nullptr;
        }

        std::unique_ptr<internal::SettingElement> get_element(std::string const& a_strFileName, std::string const& a_strElementName) {
            if (auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                if (auto itElm = itFile->second.elm_info.find(a_strElementName); itElm != itFile->second.elm_info.end()) {
                    auto const& func = itElm->second.funcCreate;
                    if(func) {
                        return func();
                    }
                }
            }
            return nullptr;
        }

        namespace internal {

            string& xml_vector_element_name() {
                static string xml_vector_element_name{ "value" };
                return xml_vector_element_name;
            }

            emb::settings::DefaultMode& default_mode() {
                static emb::settings::DefaultMode mode{emb::settings::DefaultMode::DefaultValueIfAbsentFromFile};
                return mode;
            }

            void remove_tree(boost::property_tree::ptree & a_rTree, std::string const& a_strKeyToRemove) {
                auto pos = a_strKeyToRemove.find_last_of('.');
                if(std::string::npos != pos) {
                    try {
                        a_rTree.get_child(a_strKeyToRemove.substr(0, pos)).erase(a_strKeyToRemove.substr(pos+1));
                    }
                    catch(boost::property_tree::ptree_bad_path&) {
                        // get_child() may throw if the key does not exist
                    }
                }
                else {
                    a_rTree.erase(a_strKeyToRemove);
                }
            }

            std::string stringify_tree(boost::property_tree::ptree const& a_Tree) {
                std::stringstream strTmpFilecontent;
                boost::property_tree::write_json(strTmpFilecontent, a_Tree, false);
                std::string str{strTmpFilecontent.str()};
                if(str.find_last_of("\n") == str.size() - 1) {
                    str = str.substr(0, str.size() - 1);
                }
                if(0 == str.find("{\"root\":")) {
                    str = str.substr(8, str.length() - 8 - 1);
                }
                return str;
            }

            //////////////////////////////////////////////////
            ///// SettingElement                         /////
            //////////////////////////////////////////////////

            std::string SettingElement::get_name_m() const {
                return m_strName;
            }

            std::string SettingElement::get_type_m() const {
                return m_strType;
            }

            std::string SettingElement::get_file_m() const {
                return m_strFile;
            }

            std::string SettingElement::get_key_m() const {
                return m_strKey;
            }

            void SettingElement::read_linked_m() const {
                if (auto fct = files_info()[get_file_m()].elm_info[get_name_m()].funcReadLinked) {
                    fct();
                }
            }

            void SettingElement::write_linked_m() const {
                if (auto fct = files_info()[get_file_m()].elm_info[get_name_m()].funcWriteLinked) {
                    fct();
                }
            }

            //bool SettingElement::is_default() const {
            //    return false;
            //}

            //void SettingElement::reset_to_default() const {

            //}

            SettingElement::SettingElement(std::string const& a_strName, std::string const& a_strType, std::string const& a_strFile, std::string const& a_strKey)
                : m_strName{ a_strName }
                , m_strType{ a_strType }
                , m_strFile{ a_strFile }
                , m_strKey{ a_strKey }
            {}

            SettingElement::~SettingElement()
            {}

            void SettingElement::link_variable_m(std::function<void(void)> const& a_funcRead, std::function<void(void)> const& a_funcWrite) {
                if(auto itFile = files_info().find(get_file_m()); itFile != files_info().end()) {
                    if(auto itElm = itFile->second.elm_info.find(get_name_m()); itElm != itFile->second.elm_info.end()) {
                        itElm->second.funcReadLinked = a_funcRead;
                        itElm->second.funcWriteLinked = a_funcWrite;
                    }
                }
            }

            bool SettingElement::register_element(char const* a_szFile, char const* a_szElement, creation_method<SettingElement> a_funcCreationMethod) {
                DEBUG_SELF_REGISTERING(cout << "register_element(" << a_szFile << "," << a_szElement << ")" << endl);
                bool bRes{false};
                if(a_funcCreationMethod()->get_key_m() == version_element_name()) {
                    cerr << "SettingElement '" << a_szElement << "' cannot be registered with reserved key '" << version_element_name() << "'" << endl;
                }
                else if(auto itFile = files_info().find(a_szFile); itFile != files_info().end()) {
                    if(auto itElm = itFile->second.elm_info.find(a_szElement); itElm == itFile->second.elm_info.end()) {
                        itFile->second.elm_info[a_szElement].funcCreate = a_funcCreationMethod;
                        bRes = true;
                    }
                    else {
                        cerr << "SettingElement '" << a_szElement << "' is already registered in SettingsFile '" << a_szFile << "'" << endl;
                    }
                }
                else {
                    cerr << "SettingElement '" << a_szElement << "' cannot be registered in unknown SettingsFile '" << a_szFile << "'" << endl;
                }
                assert(bRes && "Cannot register setting element");
                return bRes;
            }

            //////////////////////////////////////////////////
            ///// SettingsFile                           /////
            //////////////////////////////////////////////////

            std::string SettingsFile::get_name_m() const {
                return m_strName;
            }

            FileType SettingsFile::get_type_m() const {
                return m_eType;
            }

            std::string SettingsFile::get_path_m() const {
                return m_strPath;
            }

            int SettingsFile::get_version_m() const {
                return m_iVersion;
            }

            version_clbk_t SettingsFile::get_version_clbk_m() const {
                return m_pVersionClbk;
            }

            bool SettingsFile::backup_to_m(std::ostream & a_streamOutput) const {
                return backup_file_to_stream(get_name_m(), a_streamOutput);
            }

            bool SettingsFile::restore_from_m(std::istream & a_streamInput) const {
                return restore_file_from_stream(get_name_m(), a_streamInput);
            }

            SettingsFile::SettingsFile(std::string const& a_strName, FileType a_eType, std::string const& a_strPath, int a_iVersion, version_clbk_t a_pVersionClbk)
                : m_strName{ a_strName }
                , m_eType{ a_eType }
                , m_strPath{ a_strPath }
                , m_iVersion{ a_iVersion }
                , m_pVersionClbk{ a_pVersionClbk }
            {}

            SettingsFile::~SettingsFile()
            {}

            bool SettingsFile::register_file(char const* a_szFile, creation_method<SettingsFile> a_funcCreationMethod) {
                DEBUG_SELF_REGISTERING(cout << "register_file(" << a_szFile << ")" << endl);
                bool bRes{false};
                if(auto it = files_info().find(a_szFile); it == files_info().end()) {
                    files_info()[a_szFile].funcCreate = a_funcCreationMethod;
                    bRes = true;
                }
                else {
                    cerr << "SettingsFile '" << a_szFile << "' is already registered" << endl;
                }
                assert(bRes && "Cannot register settings file");
                return bRes;
            }

            //////////////////////////////////////////////////
            ///// tree_ptr                               /////
            //////////////////////////////////////////////////

            void tree_ptr_deleter::operator()(boost::property_tree::ptree* a_pObj) {
                for(auto & file : files_info()) {
                    if(a_pObj == file.second.lockedTree) {
                        file.second.unlock_tree();
                    }
                }
            }

            tree_ptr get_tree(std::string const& a_strFileName, std::string const& a_strElementName, bool a_bReadOnly) {
                if(auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                    if(auto itElm = itFile->second.elm_info.find(a_strElementName); itElm != itFile->second.elm_info.end()) {
                        return itFile->second.lock_tree(a_bReadOnly);
                    }
                }
                return nullptr;
            }

            boost::optional<boost::property_tree::ptree&> get_sub_tree(tree_ptr const& a_pTree, std::string const& a_strKey, bool a_bCreate) {
                auto val = a_pTree->get_child_optional(a_strKey);
                if(!val) {
                    val = a_pTree->add_child(a_strKey, boost::property_tree::ptree{});
                }
                return val;
            }

            bool backup_file(std::string const& a_strFileName, std::string const& a_strFolderName){
                bool bRes{false};
                if(auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                    auto & rFile = itFile->second;
                    rFile.mutex.lock();

                    // Compute the destination file path
                    std::string outputPath;
                    try {
                        std::filesystem::path pathSourceFile{ rFile.strFullFileName };
                        std::filesystem::path pathOutputFile{ a_strFolderName };
                        pathOutputFile /= pathSourceFile.filename();
                        outputPath = pathOutputFile.string();
                        bRes = true;
                    }
                    catch(...) {
                        bRes = false;
                    }

                    // Create the destination folder if necessary
                    if(bRes) {
                        if(!std::filesystem::exists(a_strFolderName)) {
                            bRes = std::filesystem::create_directories(a_strFolderName);
                        }
                    }

                    // Test that the file to copy exists
                    bRes = bRes && std::filesystem::exists(rFile.strFullFileName);

                    // Copy the file
                    if(bRes) {
                        try {
                            std::filesystem::copy(rFile.strFullFileName, outputPath,
                                std::filesystem::copy_options::overwrite_existing);
                        }catch(...) {
                            bRes = false;
                        }
                    }

                    rFile.mutex.unlock();
                }
                return bRes;
            }

            bool restore_file(std::string const& a_strFileName, std::string const& a_strFolderName){
                bool bRes{false};
                if(auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                    auto & rFile = itFile->second;
                    rFile.mutex.lock();

                    // Compute the destination file path
                    std::string outputPath;
                    try {
                        std::filesystem::path pathSourceFile{ rFile.strFullFileName };
                        std::filesystem::path pathOutputFile{ a_strFolderName };
                        pathOutputFile /= pathSourceFile.filename();
                        outputPath = pathOutputFile.string();
                        bRes = true;
                    }
                    catch(...) {
                        bRes = false;
                    }

                    // Create the destination folder if necessary
                    if(bRes) {
                        if(!std::filesystem::exists(a_strFolderName)) {
                            bRes = std::filesystem::create_directories(a_strFolderName);
                        }
                    }

                    // Test that the file to copy exists
                    bRes = bRes && std::filesystem::exists(rFile.strFullFileName);

                    // Copy the file
                    if(bRes) {
                        try {
                            std::filesystem::copy(outputPath, rFile.strFullFileName,
                                std::filesystem::copy_options::overwrite_existing);
                        }catch(...) {
                            bRes = false;
                        }
                    }

                    files_info().find(a_strFileName)->second.read_file();
                    rFile.mutex.unlock();
                }
                return bRes;
            }

            bool backup_file_to_stream(std::string const& a_strFileName, std::ostream & a_streamOutput) {
                bool bRes{false};
                if(auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                    auto & rFile = itFile->second;
                    rFile.mutex.lock();
                    a_streamOutput << rFile;
                    rFile.mutex.unlock();
                    bRes = true;
                }
                return bRes;
            }

            bool restore_file_from_stream(std::string const& a_strFileName, std::istream & a_streamInput) {
                bool bRes{false};
                if(auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                    auto & rFile = itFile->second;
                    rFile.mutex.lock();
                    a_streamInput >> rFile;
                    rFile.mutex.unlock();
                    bRes = true;
                }
                return bRes;
            }

            void begin_file_transaction(std::string const& a_strFileName) {
                if(auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                    auto & rFile = itFile->second;
                    rFile.mutex.lock();

                    if(!rFile.bTransactionPending) {
                        rFile.bTransactionPending = true;
                        rFile.backupTree = rFile.tree;
                    }

                    rFile.mutex.unlock();
                }
            }

            void commit_file_transaction(std::string const& a_strFileName) {
                if(auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                    auto & rFile = itFile->second;
                    rFile.mutex.lock();

                    if(rFile.bTransactionPending) {
                        rFile.bTransactionPending = false;
                        rFile.write_file();
                        rFile.backupTree.clear();
                    }

                    rFile.mutex.unlock();
                }
            }

            void abort_file_transaction(std::string const& a_strFileName) {
                if(auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                    auto & rFile = itFile->second;
                    rFile.mutex.lock();

                    if(rFile.bTransactionPending) {
                        rFile.bTransactionPending = false;
                        rFile.tree = rFile.backupTree;
                        rFile.backupTree.clear();
                    }

                    rFile.mutex.unlock();
                }
            }

        }
/*
        std::string SettingsElement::read() const {
            return read<std::string>("");
        }

        void SettingsElement::write(std::string const& a_strNewValue) const {
            write<std::string>(a_strNewValue);
        }
*/

    }
}