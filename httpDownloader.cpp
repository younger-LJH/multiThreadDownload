#include <thread>
#include <libgen.h>
#include <exception>

#include "httpDownloader.h"

// 初始化静态数据成员
bool CHttpDownloader::sIsSuccess = true;

CHttpDownloader::CHttpDownloader() : CDownloader() {
    std::cout << "[debug] CHttpDownloader, empty constructor" << std::endl;
}

CHttpDownloader::CHttpDownloader(const std::string& url, long segmentThreshold) : CDownloader(url, segmentThreshold) {
    std::cout << "[debug] CHttpDownloader, constructor" << std::endl;
}

CHttpDownloader::~CHttpDownloader() {
    std::cout << "[debug] CHttpDownloader, destory" << std::endl;
}

bool CHttpDownloader::download() {
    std::cout << "[info] download, start " << this->mUrl << std::endl;
    
    try {
        // 设置标记为真
        setDownloadStatus(true);

        // 初始化下载器
        bool ret = this->init();
        if (false == ret) {
            std::cerr << "[error] download, downloader init failed" << std::endl;
            return false;
        }

        // 多线程下载
        std::vector<std::thread> threadVector;
        for (const auto& segInfo : this->mSegInfoVector) {
            threadVector.emplace_back(routine, segInfo);
        }

        for (auto& thread : threadVector) {
            thread.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "[error] download, exception caught: " << e.what() << std::endl;
        return false;
    }

    return true;
}

std::string CHttpDownloader::getFileName() {
    return this->mFileName;
}

void CHttpDownloader::setDownloadUrl(const std::string& url, long segmentThreshold) {
    this->mUrl = url;
    this->mFileName = basename(const_cast<char*>(this->mUrl.c_str()));
    this->mSegmentThreshold = segmentThreshold * 1024 * 1024;

    std::cout << "[debug] setDownloadUrl mUrl: " << this->mUrl << std::endl;
    std::cout << "[debug] setDownloadUrl mFileName: " << this->mFileName << std::endl;
    std::cout << "[debug] setDownloadUrl mSegmentThreshold: " << this->mSegmentThreshold << std::endl;
    // 清理下载器，支持重复使用
    this->clearDownloader();
}

bool CHttpDownloader::isDownloadSuccess() {
    return sIsSuccess;
}

void CHttpDownloader::setDownloadStatus(bool status) {
    sIsSuccess = status;
}

bool CHttpDownloader::getFileSize() {
    std::shared_ptr<CURL> curl = CHttpDownloader::getDefaultCurl(this->mUrl.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_NOBODY, 1L);
    
    CURLcode ret = curl_easy_perform(curl.get());
    if (CURLE_OK != ret) {
        std::cerr << "[error] getFileSize, curl_easy_perform failed: " << ret << std::endl;
        return false;
    }

    double length = -1;
    ret = curl_easy_getinfo(curl.get(), CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
    if (CURLE_OK != ret) {
        std::cerr << "[error] getFileSize, curl_easy_getinfo failed: " << ret << std::endl;
        return false;
    }

    this->mFileSize = static_cast<long>(length);
    return true;
}

std::shared_ptr<CURL> CHttpDownloader::getDefaultCurl(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (nullptr == curl) {
        std::cerr << "[error] routine, curl_easy_init failed" << std::endl;
        return nullptr;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, CURL_REDIRECT_COUNT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, CURL_LOW_SPEED_LIMIT);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, CURL_LOW_SPEED_TIMEOUT);

    return std::shared_ptr<CURL>(curl, [](CURL* c){
        curl_easy_cleanup(c);
        std::cout << "[debug] getDefaultCurl curl_easy_cleanup" << std::endl;
    });
}

void* CHttpDownloader::routine(void* arg) {
    if (nullptr == arg) {
        std::cerr << "[error] routine, parameter error" << std::endl;
        return arg;
    }

    CSegmentInformation* segmentInfo = static_cast<CSegmentInformation*>(arg);

    std::shared_ptr<CURL> curl = CHttpDownloader::getDefaultCurl(segmentInfo->mDownloader->mUrl.c_str());
    std::string range = std::to_string(segmentInfo->mStartPosition) + "-" + std::to_string(segmentInfo->mEndPosition);
    curl_easy_setopt(curl.get(), CURLOPT_RANGE, range.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, segmentInfo);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeData);

    CURLcode ret = curl_easy_perform(curl.get());
	if (CURLE_OK != ret) {
        std::cerr << "[error] routine, curl_easy_perform failed: " << ret << std::endl;
        setDownloadStatus(false);
        return nullptr;
	}

    return nullptr;
}

size_t CHttpDownloader::writeData(char *ptr, size_t size, size_t nmemb, void *userdata) {
    if (nullptr == ptr || nullptr == userdata) {
        std::cerr << "[error] writeData, parameter error" << std::endl;
        return 0;
    }

    CSegmentInformation* segmentInfo = static_cast<CSegmentInformation*>(userdata);

    segmentInfo->mDownloader->mMutex.lock();

    size_t written = 0;
    fseek(segmentInfo->mDownloader->mFilePtr, segmentInfo->mStartPosition, SEEK_SET);
    if (segmentInfo->mStartPosition + size * nmemb <= static_cast<size_t>(segmentInfo->mEndPosition)) {
        written = fwrite(ptr, size, nmemb, segmentInfo->mDownloader->mFilePtr);
        segmentInfo->mStartPosition += size * nmemb;
    } else {
        written = fwrite(ptr, 1, segmentInfo->mEndPosition - segmentInfo->mStartPosition + 1, segmentInfo->mDownloader->mFilePtr);
        segmentInfo->mStartPosition = segmentInfo->mEndPosition;
    }

    segmentInfo->mDownloader->mMutex.unlock();
    return written;
}
