#pragma once
#include <array>
#include <bitset>

#include "model/base_model.hpp"

class YoloV8Detection final : public BaseModel {
 public:
  YoloV8Detection(const int num_cls = 0);
  YoloV8Detection(std::pair<int, int> yolov8_pair);
  ~YoloV8Detection();
  // int inference(VIDEO_FRAME_INFO_S *srcFrame, TDLObject *obj_meta)
  // override;
  virtual int32_t outputParse(
      const std::vector<std::shared_ptr<BaseImage>> &images,
      std::vector<std::shared_ptr<ModelOutputInfo>> &out_datas) override;
  virtual int32_t onModelOpened() override;
  virtual void setModelThreshold(float threshold) override;

 private:
  struct OutputBranch {
    int stride = 0;
    int cls_offset = 0;
    int num_anchor = 0;
    int box_num_anchor = 0;
    int box_feat_w = 0;
    float cls_qscale = 1.0f;
    float box_qscale = 1.0f;
    TensorInfo class_info = {};
    TensorInfo box_info = {};
    std::shared_ptr<BaseTensor> class_tensor;
    std::shared_ptr<BaseTensor> box_tensor;
    std::array<float, 256> exp_lut = {};
  };

  void decodeBboxFeatureMap(int batch_idx, int stride, int anchor_idx,
                            std::vector<float> &decode_box);

  std::map<std::string, std::string> out_names_;

  // if output seperate featuremap
  std::vector<int> strides;
  std::map<int, std::string> class_out_names;
  std::map<int, std::string> bbox_out_names;
  std::map<int, std::string> bbox_class_out_names;
  std::string input_tensor_name_;
  TensorInfo input_tensor_info_ = {};
  std::vector<OutputBranch> output_branches_;
  std::vector<std::vector<ObjectBoxInfo>> class_boxes_;
  std::vector<int> passing_anchor_indices_;
  std::vector<int> passing_class_indices_;
  std::vector<float> passing_logits_;
  std::vector<uint8_t> max_class_uint8_;
  std::vector<float> max_class_float_;
  std::vector<int> max_class_indices_;
  std::vector<int8_t> max_class_int8_;
  std::vector<uint8_t> max_class_ids8_;
  std::vector<uint8_t> nms_mask_;
  float inverse_threshold_ = 0.0f;
  int num_box_channel_ = 64;
  int num_cls_ = 0;  // would parse automatically,should not be equal with
                     // num_box_channel_
  float nms_threshold_ = 0.5;
};
