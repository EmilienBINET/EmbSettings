#include "../include/EmbSettings.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <mutex>
#include <map>
#include <regex>

//#define DEBUG_OUTPUT

namespace emb {
    namespace settings {
        using namespace std;

        void start() {
        }

        void stop() {
        }

        void setJocker(std::string const& aJocker, std::string const& aValue) {
        }

        class SettingsFileManager {
            struct InfoWithMutex {
                SettingsFileInfo info{};
                std::mutex mutex{};
            };
            static std::map<std::string, InfoWithMutex> m_mapInfo;

        public:
            static SettingsFileInfo::Ptr getFileInfoAndLock(std::string const& strFilename, FileType a_eFileType) {
                auto& elm = m_mapInfo[strFilename];
                elm.mutex.lock();
                if (elm.info.strFilename.empty()) {
                    elm.info.strFilename = strFilename;
                    elm.info.eFileType = a_eFileType;
                }
                //if(elm.info.strFilecontent.empty()) {
                //std::ifstream is(elm.info.strFilename, std::ios::binary);
                //if (is.is_open()) {
                //    std::stringstream buffer;
                //    elm.info.strFilecontent.str(std::string()); // clear
                //    elm.info.strFilecontent << is.rdbuf();
                //}
                //}
                try {
                    switch(elm.info.eFileType) {
                    case FileType::XML:
                        boost::property_tree::read_xml(elm.info.strFilecontent, elm.info.tree);
                        break;
                    case FileType::JSON:
                        boost::property_tree::read_json(elm.info.strFilecontent, elm.info.tree);
                        break;
                    case FileType::INI:
                        boost::property_tree::read_ini(elm.info.strFilecontent, elm.info.tree);
                        break;
                    }
                }
                catch(...) {
                }
                return SettingsFileInfo::Ptr{ &elm.info };
            }
            static void setFileInfoAndUnlock(std::string const& strFilename) {
                auto& elm = m_mapInfo[strFilename];
                elm.info.strFilecontent.str(std::string()); // clear
                try {
                    switch(elm.info.eFileType) {
                    case FileType::XML:
                        boost::property_tree::write_xml(elm.info.strFilecontent, elm.info.tree,
                            boost::property_tree::xml_writer_settings<decltype(elm.info.tree)::key_type>(' ', 4));
                        break;
                    case FileType::JSON:
                        boost::property_tree::write_json(elm.info.strFilecontent, elm.info.tree);
                        break;
                    case FileType::INI:
                        boost::property_tree::write_ini(elm.info.strFilecontent, elm.info.tree);
                        break;
                    }
                }
                catch(...) {
                }
                std::ofstream os(elm.info.strFilename, std::ios::binary);
                if (os.is_open()) {
                    os << elm.info.strFilecontent.str();
                }
                #ifdef DEBUG_OUTPUT
                std::cout << elm.info.strFilecontent.str() << std::endl;
                #endif
                elm.mutex.unlock();
            }
        };
        std::map<std::string, SettingsFileManager::InfoWithMutex> SettingsFileManager::m_mapInfo{};

        void SettingsFileInfo::Deleter::operator()(SettingsFileInfo* a_pObj) {
            SettingsFileManager::setFileInfoAndUnlock(a_pObj->strFilename);
        }

        SettingsFileInfo::Ptr SettingsFileInfo::getFileInfo(std::string const& a_strPath, FileType a_eFileType) {
            return SettingsFileManager::getFileInfoAndLock(a_strPath, a_eFileType);
        }

        SettingsManager::SettingsManager() {
        }
    }
}