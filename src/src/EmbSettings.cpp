#include "../include/EmbSettings.hpp"
#include <string>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS // Avoid warning
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <mutex>
#include <map>
#include <regex>

//#define DEBUG_OUTPUT

#if 1 // 1 to debug registering
#include <iostream>
#define DEBUG_SELF_REGISTERING(_cmd) _cmd
#else
#define DEBUG_SELF_REGISTERING(_cmd)
#endif


using namespace std;
namespace {

    map<string, string>& jockers() {
        static map<string, string> jockers{};
        return jockers;
    }

    void parse_jockers(std::string& a_rstrFilePath) {
        for (auto const& elm : jockers()) {
            boost::replace_all(a_rstrFilePath, "@{" + elm.first + "}", elm.second);
        }
    }

    struct SettingElementInfo {
        emb::settings::internal::creation_method<emb::settings::internal::SettingElement> funcCreate{};
    };

    struct SettingsFileInfo {
        emb::settings::internal::creation_method<emb::settings::internal::SettingsFile> funcCreate{};
        recursive_mutex mutex{};
        boost::property_tree::ptree tree{};
        map<string, SettingElementInfo> elm_info{};

        emb::settings::FileType eFileType{};
        string strFullFileName{};
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

        emb::settings::internal::tree_ptr lock_tree() {
            mutex.lock();
            if(strFullFileName.empty()) {
                auto pFileInfo = funcCreate();
                eFileType = pFileInfo->get_type();
                strFullFileName = pFileInfo->get_path();
                parse_jockers(strFullFileName);
                read_file();
            }
            return emb::settings::internal::tree_ptr{ &tree };
        }

        void unlock_tree() {
            write_file();
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

        void set_jocker(std::string const& a_strJocker, std::string const& a_strValue) {
            jockers()[a_strJocker] = a_strValue;
        }

        void set_xml_vector_element_name(std::string const& a_strName) {
            internal::xml_vector_element_name() = a_strName;
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
            
            //////////////////////////////////////////////////
            ///// SettingElement                         /////
            //////////////////////////////////////////////////

            std::string SettingElement::get_name() const {
                return m_strName;
            }

            std::string SettingElement::get_type() const {
                return m_strType;
            }

            std::string SettingElement::get_file() const {
                return m_strFile;
            }

            std::string SettingElement::get_key() const {
                return m_strKey;
            }
            
            SettingElement::SettingElement(std::string const& a_strName, std::string const& a_strType, std::string const& a_strFile, std::string const& a_strKey)
                : m_strName{ a_strName }
                , m_strType{ a_strType }
                , m_strFile{ a_strFile }
                , m_strKey{ a_strKey }
            {}

            SettingElement::~SettingElement()
            {}

            bool SettingElement::register_element(char const* a_szFile, char const* a_szElement, creation_method<SettingElement> a_funcCreationMethod) {
                DEBUG_SELF_REGISTERING(cout << "register_element(" << a_szFile << "," << a_szElement << ")" << endl);
                bool bRes{false};
                if(auto itFile = files_info().find(a_szFile); itFile != files_info().end()) {
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

            std::string SettingsFile::get_name() const {
                return m_strName;
            }
            
            FileType SettingsFile::get_type() const {
                return m_eType;
            }
            
            std::string SettingsFile::get_path() const {
                return m_strPath;
            }
            
            int SettingsFile::get_version() const {
                return m_iVersion;
            }
            
            SettingsFile::SettingsFile(std::string const& a_strName, FileType a_eType, std::string const& a_strPath, int a_iVersion)
                : m_strName{ a_strName }
                , m_eType{ a_eType }
                , m_strPath{ a_strPath }
                , m_iVersion{ a_iVersion }
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
                    if(a_pObj == &file.second.tree) {
                        file.second.unlock_tree();
                    }
                }
            }

            tree_ptr get_tree(std::string const& a_strFileName, std::string const& a_strElementName) {
                if(auto itFile = files_info().find(a_strFileName); itFile != files_info().end()) {
                    if(auto itElm = itFile->second.elm_info.find(a_strElementName); itElm != itFile->second.elm_info.end()) {
                        return itFile->second.lock_tree();
                    }
                }
                return nullptr;
            }
        }
/*
        std::map<std::string, std::pair<std::function<void(void)>, std::function<void(void)>>>& SettingsElement::getLinks() {
            static std::map<std::string, std::pair<std::function<void(void)>, std::function<void(void)>>> links{};
            return links;
        }

        SettingsElement::SettingsElement(std::string const& a_strClassName, std::string const& a_strType, std::string const& a_strFileClassName, std::string const& a_strPath)
            : m_strClassName{ a_strClassName }
            , m_strType{ a_strType }
            , m_strFileClassName{ a_strFileClassName }
            , m_strPath{ a_strPath }
        {}

        std::string SettingsElement::getClassName() const {
            return m_strClassName;
        }

        std::string SettingsElement::getType() const {
            return m_strType;
        }

        std::string SettingsElement::getFileClassName() const {
            return m_strFileClassName;
        }

        std::string SettingsElement::getPath() const {
            return m_strPath;
        }

        std::string SettingsElement::read() const {
            return read<std::string>("");
        }

        void SettingsElement::write(std::string const& a_strNewValue) const {
            write<std::string>(a_strNewValue);
        }

        void SettingsElement::read_linked() const {
            if (auto fct = get<0>(SettingsElement::getLinks()[getPath()])) {
                fct();
            }
        }

        void SettingsElement::write_linked() const {
            if (auto fct = get<1>(SettingsElement::getLinks()[getPath()])) {
                fct();
            }
        }

        SettingsFile::SettingsFile(std::string const& a_strClassName, FileType a_eFileType, std::string const& a_strFilePath, int a_iFileVersion)
            : m_strClassName{ a_strClassName }
            , m_eFileType{ a_eFileType }
            , m_strFilePath{ a_strFilePath }
            , m_iFileVersion{ a_iFileVersion }
        {}

        std::string SettingsFile::getClassName() const {
            return m_strClassName;
        }

        FileType SettingsFile::getFileType() const {
            return m_eFileType;
        }

        std::string SettingsFile::getFilePath() const {
            return m_strFilePath;
        }

        int SettingsFile::getFileVersion() const {
            return m_iFileVersion;
        }

        std::map<std::string, std::map<std::string, SettingsElement::CreateMethod>>& SettingsFile::getMap() {
            static std::map<std::string, std::map<std::string, SettingsElement::CreateMethod>> map{};
            return map;
        }

        bool SettingsFile::register_settings(char const* a_szFile, char const* a_szPath, SettingsElement::CreateMethod a_pCreateMethod) {
#ifdef DEBUG_REGISTER
            std::cout << "Registering Setting: " << a_szPath << " in " << a_szFile << std::endl;
#endif
            getMap()[a_szFile][a_szPath] = a_pCreateMethod;
            return true;
        }

        std::map<std::string, emb::settings::SettingsElement::CreateMethod>& SettingsFile::getElementsMap(std::string const& a_strFileClass) {
            return getMap()[a_strFileClass];
        }

        class SettingsFileManager {
            struct InfoWithMutex {
                internal::SettingsFileInfo info{};
                std::mutex mutex{};
                std::string strFullFileName{};
                FileType eFileType{};
                std::stringstream strFilecontent{};
            };
            static std::map<std::string, InfoWithMutex>& getMapInfo() {
                static std::map<std::string, InfoWithMutex> mapInfo{};
                return mapInfo;
            }

        public:
            static internal::SettingsFileInfo::Ptr getFileInfoAndLock(std::string const& strFilename, FileType a_eFileType) {
                auto& elm = getMapInfo()[strFilename];
                elm.mutex.lock();
                if (elm.info.strFilename.empty()) {
                    elm.strFullFileName = strFilename;
                    parse_jockers(elm.strFullFileName);
                    elm.info.strFilename = strFilename;
                    elm.info.eFileType = a_eFileType;
                    elm.eFileType = a_eFileType;
                    std::ifstream is(elm.strFullFileName, std::ios::binary);
                    if (is.is_open()) {
                        std::stringstream buffer;
                        elm.strFilecontent.str(std::string()); // clear
                        elm.strFilecontent << is.rdbuf();
                    }
                    try {
                        switch (elm.eFileType) {
                        case FileType::XML:
                            boost::property_tree::read_xml(elm.strFilecontent, elm.info.tree, boost::property_tree::xml_parser::trim_whitespace);
                            break;
                        case FileType::JSON:
                            boost::property_tree::read_json(elm.strFilecontent, elm.info.tree);
                            break;
                        case FileType::INI:
                            boost::property_tree::read_ini(elm.strFilecontent, elm.info.tree);
                            break;
                        }
                    }
                    catch (...) {
                        elm.info.tree = decltype(elm.info.tree)();
                    }
                }
                return internal::SettingsFileInfo::Ptr{ &elm.info };
            }
            static void setFileInfoAndUnlock(std::string const& strFilename) {
                auto& elm = getMapInfo()[strFilename];
                try {
                    std::stringstream strFilecontent{};
                    switch (elm.eFileType) {
                    case FileType::XML:
                        boost::property_tree::write_xml(strFilecontent, elm.info.tree,
                            boost::property_tree::xml_writer_settings<decltype(elm.info.tree)::key_type>(' ', 4));
                        break;
                    case FileType::JSON:
                        boost::property_tree::write_json(strFilecontent, elm.info.tree);
                        break;
                    case FileType::INI:
                        boost::property_tree::write_ini(strFilecontent, elm.info.tree);
                        break;
                    }
                    if (elm.strFilecontent.str() != strFilecontent.str()) {
                        std::ofstream os(elm.strFullFileName, std::ios::binary);
                        if (os.is_open()) {
                            os << strFilecontent.str();
                        }
                    }
                    elm.strFilecontent.str(strFilecontent.str());
                }
                catch (...) {
                }
#ifdef DEBUG_OUTPUT
                std::cout << elm.info.strFilecontent.str() << std::endl;
#endif
                elm.mutex.unlock();
            }
        };

        void internal::SettingsFileInfo::Deleter::operator()(internal::SettingsFileInfo* a_pObj) {
            SettingsFileManager::setFileInfoAndUnlock(a_pObj->strFilename);
        }

        internal::SettingsFileInfo::Ptr internal::SettingsFileInfo::getFileInfo(std::unique_ptr<SettingsFile> a_pSettingsFile) {
            return SettingsFileManager::getFileInfoAndLock(a_pSettingsFile->getFilePath(), a_pSettingsFile->getFileType());
        }

        std::map<std::string, SettingsFile::CreateMethod>& getFilesMap() {
            static std::map<std::string, SettingsFile::CreateMethod> map{};
            return map;
        }

        bool register_file(std::string const& a_strName, SettingsFile::CreateMethod a_pCreateMethod) {
#ifdef DEBUG_REGISTER
            std::cout << "Registering File: " << " as " << a_strName << std::endl;
#endif
            getFilesMap()[a_strName] = a_pCreateMethod;
            return true;
        }











*/
        
    }
}