#include "../include/EmbSettings.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <mutex>
#include <map>
#include <regex>

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
            static SettingsFileInfo::Ptr getFileInfoAndLock(std::string strFilename) {
                auto& elm = m_mapInfo[strFilename];
                elm.mutex.lock();
                if (elm.info.strFilename.empty()) {
                    elm.info.strFilename = strFilename;
                }
                //if(elm.info.strFilecontent.empty()) {
                std::ifstream is(elm.info.strFilename, std::ios::binary);
                if (is.is_open()) {
                    std::stringstream buffer;
                    elm.info.strFilecontent.str(std::string()); // clear
                    elm.info.strFilecontent << is.rdbuf();
                }
                //}
                boost::property_tree::read_xml(elm.info.strFilecontent, elm.info.tree);
                return SettingsFileInfo::Ptr{ &elm.info };
            }
            static void setFileInfoAndUnlock(std::string strFilename) {
                auto& elm = m_mapInfo[strFilename];
                elm.info.strFilecontent.str(std::string()); // clear
                boost::property_tree::write_xml(elm.info.strFilecontent, elm.info.tree);
                std::cout << elm.info.strFilecontent.str() << std::endl;
                elm.mutex.unlock();
            }
        };
        std::map<std::string, SettingsFileManager::InfoWithMutex> SettingsFileManager::m_mapInfo{};

        void SettingsFileInfo::Deleter::operator()(SettingsFileInfo* a_pObj) {
            SettingsFileManager::setFileInfoAndUnlock(a_pObj->strFilename);
        }

        SettingsFileInfo::Ptr SettingsFileInfo::getFileInfo(std::string& a_rstrPath) {
            auto pos = a_rstrPath.find(">");
            auto elm = SettingsFileManager::getFileInfoAndLock(a_rstrPath.substr(0, pos));
            a_rstrPath = a_rstrPath.substr(pos + 1);
            return elm;
        }

        SettingsManager::SettingsManager() {
        }
    }
}