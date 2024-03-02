/**
 * @brief HTTP 下载器派生类
*/
#ifndef _HTTP_DOWNLOADER_H
#define _HTTP_DOWNLOADER_H

#include <memory>
#include <curl/curl.h>

#include "constants.h"
#include "downloader.h"

class CHttpDownloader : protected CDownloader {
    private:
        // 下载是否成功标记，如果某个线程失败则置为 false
        static bool sIsSuccess;

    public:
        CHttpDownloader();
        CHttpDownloader(const std::string& url, long segmentThreshold = SEGMENT_THRESHOLD);
        virtual ~CHttpDownloader();
        bool download() override;
        std::string getFileName();
        static bool isDownloadSuccess();
        void setDownloadUrl(const std::string& url, long segmentThreshold = SEGMENT_THRESHOLD);
    
    protected:
        bool getFileSize() override;

    private:
        static void setDownloadStatus(bool status);
        static std::shared_ptr<CURL> getDefaultCurl(const std::string& url);
        static void* routine(void* arg);
        static size_t writeData(char *ptr, size_t size, size_t nmemb, void *userdata);
};

#endif
