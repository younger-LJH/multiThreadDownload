/**
 * @brief 下载器基类
*/
#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include <mutex>
#include <vector>
#include <iostream>

#include "constants.h"

// 分段具体信息
class CSegmentInformation;

class CDownloader {
    public:
        std::string mUrl;
        std::string mFileName;
        long mSegmentThreshold;
        long mSegmentTotal;
        long mFileSize;
        FILE* mFilePtr;
        std::mutex mMutex;
        std::vector<CSegmentInformation*> mSegInfoVector;   

    public:
        CDownloader();
        CDownloader(const std::string& url, long segmentThreshold = SEGMENT_THRESHOLD);
        virtual ~CDownloader();
        virtual bool download() = 0;

    protected:
        virtual bool init();
        virtual void clearDownloader();
        virtual bool segmentInformation();
        virtual bool getFileSize() = 0;
};

class CSegmentInformation {
    public:
        long mSegmentNumber;
        long mStartPosition;
        long mEndPosition;
        CDownloader* const mDownloader;

    public:
        CSegmentInformation(long number, long start, long end, CDownloader* const downloader);
        virtual ~CSegmentInformation();
        friend std::ostream& operator<<(std::ostream& os, const CSegmentInformation& obj);
};

#endif
