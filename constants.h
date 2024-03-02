/**
 * @brief 通用常量
*/
#ifndef _CONSTANTS_H
#define _CONSTANTS_H

// 多线程分段下载阈值 (单位 M)
#define SEGMENT_THRESHOLD 2
// 最多支持的线程数
#define THREAD_NUMBER_MAX 8

// CURL 连接超时时间 (单位 s)
#define CURL_CONNECT_TIMEOUT 5
// CURL 支持重定向次数
#define CURL_REDIRECT_COUNT 3
// CURL 最低传输速度限制 (单位 byte/s)
#define CURL_LOW_SPEED_LIMIT 1
// CURL 低于最低传输速度的超时时间 (单位 s)
#define CURL_LOW_SPEED_TIMEOUT 5

#endif
