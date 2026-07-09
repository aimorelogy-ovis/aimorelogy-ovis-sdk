#include "vpss_bin.h"

#include "cvi_bin.h"
#include "cvi_defines.h"
#include "cvi_json_struct_comm.h"
#include "cvi_mw_base.h"
#include "cvi_type.h"
#include "cvi_vpss.h"
#include "rw_json.h"
#include "vpss_ctx.h"
#include "vpss_json_struct.h"
/**************************************************************************
 *   Bin related APIs.
 **************************************************************************/
CVI_S32 vpss_bin_getbinsize(CVI_U32 *size)
{
    *size = sizeof(VPSS_BIN_DATA) * VPSS_MAX_GRP_NUM;

    return CVI_SUCCESS;
}

CVI_S32 vpss_bin_getparamfrombin(CVI_U8 *addr, CVI_U32 size)
{
    CVI_U32        u32DataSize    = 0;
    VPSS_BIN_DATA *pstVpssBinData = get_vpssbindata_addr();

    vpss_bin_getbinsize(&u32DataSize);
    memset(pstVpssBinData, 0, u32DataSize);
    if (size > u32DataSize) {
        CVI_TRACE_VPSS(CVI_DBG_WARN, "Bin size(%d) > max size(%d).\n", size, u32DataSize);
        return CVI_FAILURE;
    }
    memcpy(pstVpssBinData, addr, size);
    set_loadbin_state(CVI_TRUE);

    return CVI_SUCCESS;
}

CVI_S32 vpss_bin_setparamtobuf(CVI_U8 *buffer)
{
    CVI_S32               ret            = CVI_SUCCESS;
    CVI_U32               u32DataSize    = 0;
    VPSS_BIN_DATA        *pstVpssBinData = get_vpssbindata_addr();
    struct cvi_vpss_ctx **vpss_ctx       = vpss_get_ctx();

    vpss_bin_getbinsize(&u32DataSize);
    for (int i = 0; i < VPSS_MAX_GRP_NUM; ++i) {
        if (vpss_ctx[i])
            memcpy(pstVpssBinData[i].proc_amp, vpss_ctx[i]->proc_amp,
                   sizeof(pstVpssBinData[i].proc_amp));
    }
    memcpy(buffer, pstVpssBinData, u32DataSize);
    return ret;
}

CVI_S32 vpss_bin_setparamtobin(FILE *fp)
{
    CVI_S32               ret            = CVI_SUCCESS;
    CVI_U32               u32DataSize    = 0;
    VPSS_BIN_DATA        *pstVpssBinData = get_vpssbindata_addr();
    struct cvi_vpss_ctx **vpss_ctx       = vpss_get_ctx();

    vpss_bin_getbinsize(&u32DataSize);
    for (int i = 0; i < VPSS_MAX_GRP_NUM; ++i) {
        if (vpss_ctx[i])
            memcpy(pstVpssBinData[i].proc_amp, vpss_ctx[i]->proc_amp,
                   sizeof(pstVpssBinData[i].proc_amp));
    }
    fwrite(pstVpssBinData, u32DataSize, 1, fp);
    return ret;
}

#if CONFIG_PQBIN_USE_JSON
/**************************************************************************
 *   Json related APIs.
 **************************************************************************/

static CVI_S32 vpss_json_getparam(CVI_U8 *addr)
{
    CVI_S32               ret            = CVI_SUCCESS;
    CVI_U32               u32DataSize    = 0;
    VPSS_BIN_DATA        *pstPtr         = (VPSS_BIN_DATA *)addr;
    VPSS_BIN_DATA        *pstVpssBinData = get_vpssbindata_addr();
    struct cvi_vpss_ctx **vpss_ctx       = vpss_get_ctx();

    vpss_bin_getbinsize(&u32DataSize);
    for (int i = 0; i < VPSS_MAX_GRP_NUM; ++i)
        if (vpss_ctx[i])
            memcpy(pstVpssBinData[i].proc_amp, vpss_ctx[i]->proc_amp,
                   sizeof(pstVpssBinData[i].proc_amp));

    memcpy(pstPtr, pstVpssBinData, u32DataSize);

    return ret;
}

static CVI_S32 vpss_json_setparam(CVI_U8 *addr)
{
    CVI_U32        u32DataSize    = 0;
    VPSS_BIN_DATA *pstVpssBinData = get_vpssbindata_addr();

    vpss_bin_getbinsize(&u32DataSize);
    memcpy(pstVpssBinData, addr, u32DataSize);
    for (int i = 0; i < VPSS_MAX_GRP_NUM; ++i) {
        // memcpy(vpssCtx[i].proc_amp, pstVpssBinData[i].proc_amp,
        // sizeof(pstVpssBinData[i].proc_amp));
    }
    set_loadbin_state(CVI_TRUE);

    return CVI_SUCCESS;
}

CVI_S32 vpss_json_getParamFromJsonbuffer(const char *buffer, enum CVI_BIN_SECTION_ID id)
{
    JSON                 *json_object;
    CVI_S32               ret            = CVI_SUCCESS;
    VPSS_PARAMETER_BUFFER vpss_parameter = {0};

    json_object = JSON_TokenerParse(buffer);
    if (json_object) {
        vpss_json_getparam((CVI_U8 *)vpss_parameter.vpss_bin_data);
        JSON_(R_FLAG, json_object, VPSS_PARAMETER_BUFFER, "vpss_parameter", &vpss_parameter);
        vpss_json_setparam((CVI_U8 *)vpss_parameter.vpss_bin_data);

        JSON_ObjectPut(json_object);
    } else {
        CVI_TRACE_VPSS(LOG_WARNING, "(id:%d)Creat json tokener fail.\n", id);
        ret = CVI_BIN_JSONHANLE_ERROR;
    }

    UNUSED(id);
    return ret;
}

CVI_S32 vpss_json_setParamToJsonbuffer(CVI_S8 **buffer, enum CVI_BIN_SECTION_ID id, CVI_S32 *len)
{
    VPSS_PARAMETER_BUFFER vpss_parameter = {0};
    JSON                 *json_object;
    CVI_S32               ret = CVI_SUCCESS;

    json_object = JSON_GetNewObject();
    if (json_object) {
        vpss_json_getparam((CVI_U8 *)vpss_parameter.vpss_bin_data);
        JSON_(W_FLAG, json_object, VPSS_PARAMETER_BUFFER, "vpss_parameter", &vpss_parameter);

        *len    = JSON_GetJsonStrLen(json_object);
        *buffer = (CVI_S8 *)malloc(*len);
        if (*buffer == NULL) {
            ret = CVI_BIN_MALLOC_ERR;
            CVI_TRACE_VPSS(LOG_WARNING, "%s\n", "Allocate memory fail");
            goto ERROR_HANDLER;
        }
        memcpy(*buffer, JSON_GetJsonStrContent(json_object), *len);
    } else {
        CVI_TRACE_VPSS(LOG_WARNING, "(id:%d)Get New Object fail.\n", id);
        ret = CVI_BIN_JSONHANLE_ERROR;
    }

ERROR_HANDLER:
    if (json_object) {
        JSON_ObjectPut(json_object);
    }

    UNUSED(id);

    return ret;
}
#endif
