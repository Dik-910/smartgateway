#if !defined(__OTA_VERSION_H)
#define __OTA_VERSION_H

#define VERSION_MAJOR 3 // 主版本号
#define VERSION_MINOR 0 // 次版本号
#define VERSION_PATCH 0 // 修订版本号

/**
 * 版本检查更新
 */
int ota_version_checkUpdate();

/**
 * 每隔一天检查一次版本更新
 */
int ota_version_checkUpdateDaily();

/**
 * 输出版本号
 */
void ota_version_printVersion();


#endif // __OTA_VERSION_H
