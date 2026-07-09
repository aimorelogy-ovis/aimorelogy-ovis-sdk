#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Ai_KeypointHandGesture(const char * file)
{
    APP_PARAM_AI_KEYPOINT_HAND_GESTURE_CFG_S *Ai = app_ipcam_Ai_Keypoint_Hand_Gesture_Param_Get();
    int enum_num = 0;
    int ret = 0;
    char tmp_section[40] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char ** ai_supported_model = app_ipcam_Param_get_ai_supported_model();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI Keypoint Hand Gesture Detection config ------------------> start \n");

    char tmp_buff[128] = {0};

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_keypoint_hand_gesture_config");

    Ai->bEnable             = ini_getl(tmp_section, "keypoint_hand_gesture_enable", 0, file);
    Ai->VpssGrp             = ini_getl(tmp_section, "vpss_grp", 0, file);
    Ai->VpssChn             = ini_getl(tmp_section, "vpss_chn", 0, file);
    Ai->threshold           = ini_getf(tmp_section, "threshold", 0, file);
    ini_gets(tmp_section, "model_path_cfg", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_cfg, tmp_buff, 128);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_path_cfg=%s\n", Ai->model_path_cfg);

    // 解析检测模型配置
    ini_gets(tmp_section, "detect_model_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][detect_model_id] Fail to convert string name [%s] to enum number, using default TDL_MODEL_YOLOV8N_DET_HAND!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][detect_model_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->detect_model_id = enum_num;
    }

    // 解析关键点模型配置
    ini_gets(tmp_section, "keypoint_model_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][keypoint_model_id] Fail to convert string name [%s] to enum number, using default TDL_MODEL_KEYPOINT_HAND!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][keypoint_model_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->keypoint_model_id = enum_num;
    }

    // 解析分类模型配置
    ini_gets(tmp_section, "classify_model_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][classify_model_id] Fail to convert string name [%s] to enum number, using default TDL_MODEL_CLS_KEYPOINT_HAND_GESTURE!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][classify_model_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->classify_model_id = enum_num;
    }

    // 解析是否启用分类
    Ai->bEnableClassification = ini_getbool(tmp_section, "enable_classification", 0, file);


    // 解析检测模型路径
    ini_gets(tmp_section, "detect_model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->detect_model_path, tmp_buff, 128);

    // 解析关键点模型路径
    memset(tmp_buff, 0, sizeof(tmp_buff));
    ini_gets(tmp_section, "keypoint_model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->keypoint_model_path, tmp_buff, 128);

    // 解析分类模型路径
    memset(tmp_buff, 0, sizeof(tmp_buff));
    ini_gets(tmp_section, "classify_model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->classify_model_path, tmp_buff, 128);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "keypoint_hand_gesture_enable=%d vpss_grp=%d vpss_chn=%d threshold=%f\n", \
        Ai->bEnable, Ai->VpssGrp, Ai->VpssChn,  Ai->threshold);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "detect_model_id=%d detect_model_path=%s\n",
        Ai->detect_model_id, Ai->detect_model_path);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "keypoint_model_id=%d keypoint_model_path=%s\n",
        Ai->keypoint_model_id, Ai->keypoint_model_path);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "classify_model_id=%d classify_model_path=%s enable_classification=%d\n",
        Ai->classify_model_id, Ai->classify_model_path, Ai->bEnableClassification);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI Keypoint Hand Gesture Detection config ------------------> done \n\n");

    return CVI_SUCCESS;
}
