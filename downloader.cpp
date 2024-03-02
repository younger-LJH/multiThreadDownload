#include <libgen.h>
#include <exception>

#include "downloader.h"

CDownloader::CDownloader() {
    this->mUrl = "";
    this->mFileName = "";
    this->mSegmentThreshold = SEGMENT_THRESHOLD * 1024 * 1024;
    this->mSegmentTotal = 0;
    this->mFileSize = 0;
    this->mFilePtr = nullptr;

    std::cout << "[debug] CDownloader, empty constructor mFileName: " << this->mFileName << std::endl;
    std::cout << "[debug] CDownloader, empty constructor mSegmentThreshold: " << this->mSegmentThreshold << std::endl;
}

CDownloader::CDownloader(const std::string& url, long segmentThreshold) {
    this->mUrl = url;
    this->mFileName = basename(const_cast<char*>(this->mUrl.c_str()));
    this->mSegmentThreshold = segmentThreshold * 1024 * 1024;
    this->mSegmentTotal = 0;
    this->mFileSize = 0;
    this->mFilePtr = nullptr;

    std::cout << "[debug] CDownloader, constructor mFileName: " << this->mFileName << std::endl;
    std::cout << "[debug] CDownloader, constructor mSegmentThreshold: " << this->mSegmentThreshold << std::endl;
}

CDownloader::~CDownloader() {
    this->clearDownloader();
    std::cout << "[debug] CDownloader, destory" << std::endl;
}

bool CDownloader::init() {
    try {
        this->mFilePtr = fopen(this->mFileName.c_str(), "wb"); 
        if (nullptr == this->mFilePtr) {
            std::cerr << "[error] init, fopen file failed: " << this->mFileName.c_str() << std::endl;
            return false;
        }

        bool ret = this->getFileSize();
        if (false == ret) {
            std::cerr << "[error] init, getFileSize failed" << std::endl;
            return false;
        }

        ret = this->segmentInformation();
        if (false == ret) {
            std::cerr << "[error] init, segmentInformation failed" << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "[error] init, exception caught: " << e.what() << std::endl;
        return false;
    }
    
    std::cout << "[debug] init, mFileSize: " << this->mFileSize << std::endl;
    std::cout << "[debug] init, mSegmentTotal: " << this->mSegmentTotal << std::endl;
    return true;
}

void CDownloader::clearDownloader() {
    if (nullptr != this->mFilePtr) {
        fclose(this->mFilePtr);
        this->mFilePtr = nullptr;
    }

    for (auto& segInfo : this->mSegInfoVector) {
        if (nullptr != segInfo) {
            delete segInfo;
            segInfo = nullptr;
        }
    }
    this->mSegInfoVector.clear();
}

bool CDownloader::segmentInformation() {
    this->mSegmentTotal = (this->mFileSize / this->mSegmentThreshold) + 
                            (this->mFileSize % this->mSegmentThreshold ? 1 : 0);

    if (this->mSegmentTotal > THREAD_NUMBER_MAX) {
        this->mSegmentTotal = THREAD_NUMBER_MAX;
        this->mSegmentThreshold = this->mFileSize / this->mSegmentTotal;
        std::cout << "[info] segmentInformation, reset mSegmentThreshold to " << this->mSegmentThreshold << std::endl;
    }

    for (int i = 0; i < this->mSegmentTotal; ++i) {
        long start = i * this->mSegmentThreshold;
        long end = ((i + 1) != this->mSegmentTotal) ? (start + this->mSegmentThreshold - 1) : (this->mFileSize - 1);
        this->mSegInfoVector.emplace_back(new CSegmentInformation(i, start, end, this));
    }

    return true;
}

CSegmentInformation::CSegmentInformation(long number, long start, long end, CDownloader* const downloader) 
    : mDownloader(downloader) {
    this->mSegmentNumber = number;
    this->mStartPosition = start;
    this->mEndPosition = end;
    
    std::cout << "[debug] CSegmentInformation, constructor mSegmentNumber: " << this->mSegmentNumber << std::endl;
    std::cout << "[debug] CSegmentInformation, constructor mStartPosition: " << this->mStartPosition << std::endl;
    std::cout << "[debug] CSegmentInformation, constructor mEndPosition: " << this->mEndPosition << std::endl;
}

CSegmentInformation::~CSegmentInformation() {
    std::cout << "[debug] CSegmentInformation, destory" << std::endl;
}

std::ostream& operator<<(std::ostream& os, const CSegmentInformation& obj) {
    os << "number " << obj.mSegmentNumber << 
        " start " << obj.mStartPosition << 
        " end " << obj.mEndPosition;
    return os;
}
