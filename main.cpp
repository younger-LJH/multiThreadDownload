/**
 * @brief 文件下载工具
*/
#include <memory>
#include <iostream>
#include <unistd.h>
#include <exception>
#include <curl/curl.h>

#include "constants.h"
#include "httpDownloader.h"

void showUsage() {
    std::cout << "usage: multiThreadDownload [options] argv" << std::endl;
    std::cout << "    -H 通过 HTTP 下载" << std::endl;
    std::cout << "    -F 通过 FTP  下载" << std::endl;
    std::cout << "    -s 多线程下载阈值 (单位 M) (默认值 2)" << std::endl;
    std::cout << "    -h 帮助文档" << std::endl;
    std::cout << "example: " << std::endl;
    std::cout << "    multiThreadDownload -H URL" << std::endl;
    std::cout << "    multiThreadDownload -H URL -s 20" << std::endl;
    std::cout << "    multiThreadDownload -H URL1 -H URL2" << std::endl;
    std::cout << "    multiThreadDownload -H URL1 -R URL2 -s 20" << std::endl;
}

void httpDownload(const std::vector<std::string>& urlVector, const long& segmentThreshold) {
    if (urlVector.empty()) {
        return;
    }

    CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != ret) {
        std::cerr << "[error] httpDownload, curl_global_init failed: " << ret << std::endl;
        return;
    }

    try {
        std::unique_ptr<CHttpDownloader> downloader(new CHttpDownloader());
        for (const auto& url : urlVector) {
            downloader->setDownloadUrl(url, segmentThreshold);

            bool ret = downloader->download();
            if (ret && CHttpDownloader::isDownloadSuccess()) {
                std::cout << "[info] httpDownload, download success " << url << std::endl;
            } else {
                std::remove(downloader->getFileName().c_str());
                std::cerr << "[error] httpDownload, download failed " << url << std::endl;
            }
        }
    } catch(const std::exception& e) {
        std::cerr << "[error] httpDownload, exception caught: " << e.what() << std::endl;
    }

    curl_global_cleanup();
}

void ftpDownload(const std::vector<std::string>& urlVector, const long& segmentThreshold) {
    if (urlVector.empty()) {
        return;
    }

    std::cout << "[warn] The FTP download function has not been implemented yet" << std::endl;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> httpUrlVector;
    std::vector<std::string> ftpUrlVector;
    long segmentThreshold = 0;

    int option = -1;
    while ((option = getopt(argc, argv, "H:F:s:h")) != -1) {
        switch (option) {
            case 'H':
                httpUrlVector.emplace_back(optarg);
                break;
            case 'F':
                ftpUrlVector.emplace_back(optarg);
                break;
            case 's':
                segmentThreshold = std::atol(optarg);
                break;
            case 'h':
                showUsage();
                break;
            default:
                showUsage();
                break;
        }
    }
    
    // 规避用户乱输入
    if (segmentThreshold <= 0) {
        segmentThreshold = SEGMENT_THRESHOLD;
    }

    httpDownload(httpUrlVector, segmentThreshold);
    ftpDownload(ftpUrlVector, segmentThreshold);
}
