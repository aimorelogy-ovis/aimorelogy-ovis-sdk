#ifndef CVI_SYSTEM_H_
#define CVI_SYSTEM_H_
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdint.h>
#include <stdbool.h>

#define CVI_MAX_PATH_LEN (128)

int cvi_insmod(const char *pszPath, const char *pszOptions);
int cvi_rmmod(const char *pszPath);
int cvi_PathIsDirectory(const char *pszPath);
int cvi_rmdir(const char *pszPath);
int cvi_mkdir(const char *pszPath, mode_t mode);
int cvi_system(const char *pszCmd);
int cvi_usleep(uint32_t usec);
int cvi_du(const char *pszPath, uint64_t *pu64Size_KB);
int cvi_async(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* CVI_DTCF_H_ */