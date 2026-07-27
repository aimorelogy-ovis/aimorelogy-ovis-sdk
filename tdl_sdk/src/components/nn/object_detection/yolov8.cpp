#include "object_detection/yolov8.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

#include "utils/detection_helper.hpp"
#include "utils/tdl_log.hpp"

namespace {

void nms_objects_in_place(std::vector<ObjectBoxInfo> &objects,
                          float iou_threshold,
                          std::vector<uint8_t> &mask) {
  std::sort(objects.begin(), objects.end(),
            [](const ObjectBoxInfo &a, const ObjectBoxInfo &b) {
              return a.score > b.score;
            });

  mask.assign(objects.size(), 0);
  size_t output_count = 0;
  for (size_t selected = 0; selected < objects.size(); selected++) {
    if (mask[selected] != 0) {
      continue;
    }

    const ObjectBoxInfo selected_box = objects[selected];
    objects[output_count++] = selected_box;
    const float area1 = (selected_box.x2 - selected_box.x1 + 1.0f) *
                        (selected_box.y2 - selected_box.y1 + 1.0f);
    for (size_t candidate = selected + 1; candidate < objects.size();
         candidate++) {
      if (mask[candidate] != 0) {
        continue;
      }
      const ObjectBoxInfo &box = objects[candidate];
      const float x1 = std::max(selected_box.x1, box.x1);
      const float y1 = std::max(selected_box.y1, box.y1);
      const float width = std::min(selected_box.x2, box.x2) - x1 + 1.0f;
      const float height = std::min(selected_box.y2, box.y2) - y1 + 1.0f;
      if (width <= 0.0f || height <= 0.0f) {
        continue;
      }
      const float area2 = (box.x2 - box.x1 + 1.0f) *
                          (box.y2 - box.y1 + 1.0f);
      const float intersection = width * height;
      if (intersection / (area1 + area2 - intersection) > iou_threshold) {
        mask[candidate] = 1;
      }
    }
  }
  objects.resize(output_count);
}

}  // namespace

template <typename T>
inline void parse_cls_info(T *p_cls_ptr, int num_anchor, int num_cls,
                           int anchor_idx, int cls_offset, float qscale,
                           float *p_max_logit, int *p_max_cls) {
  int max_logit_c = -1;
  float max_logit = -1000;
  for (int c = 0; c < num_cls; c++) {
    float logit = p_cls_ptr[(c + cls_offset) * num_anchor + anchor_idx];
    if (logit > max_logit) {
      max_logit = logit;
      max_logit_c = c;
    }
  }
  *p_max_logit = max_logit * qscale;
  *p_max_cls = max_logit_c;
}

template <typename T>
inline std::vector<float> get_box_vals(T *p_box_ptr, int num_anchor,
                                       int anchor_idx, int num_box_channel,
                                       float qscale) {
  std::vector<float> box_vals;
  for (int c = 0; c < num_box_channel; c++) {
    box_vals.push_back(p_box_ptr[c * num_anchor + anchor_idx] * qscale);
  }
  return box_vals;
}

inline int exp_lut_index(int8_t value) { return static_cast<int>(value) + 128; }

inline int exp_lut_index(uint8_t value) { return static_cast<int>(value); }

template <typename T>
inline void parse_dfl_box(const T *box_ptr, int num_anchor, int anchor_idx,
                          int feat_w, int stride, float qscale, float *x1,
                          float *y1, float *x2, float *y2) {
  constexpr int kBoxValueCount = 4;
  constexpr int kRegMax = 16;
  float box_values[kBoxValueCount];

  for (int i = 0; i < kBoxValueCount; i++) {
    float sum_softmax = 0.0f;
    float sum_value = 0.0f;
    const int channel_offset = i * kRegMax;
    for (int j = 0; j < kRegMax; j++) {
      float logit =
          box_ptr[(channel_offset + j) * num_anchor + anchor_idx] * qscale;
      float exp_value = std::exp(logit);
      sum_softmax += exp_value;
      sum_value += exp_value * j;
    }
    box_values[i] = sum_value / sum_softmax;
  }

  int anchor_y = anchor_idx / feat_w;
  int anchor_x = anchor_idx % feat_w;
  float grid_y = anchor_y + 0.5f;
  float grid_x = anchor_x + 0.5f;
  *x1 = (grid_x - box_values[0]) * stride;
  *y1 = (grid_y - box_values[1]) * stride;
  *x2 = (grid_x + box_values[2]) * stride;
  *y2 = (grid_y + box_values[3]) * stride;
}

template <typename T>
inline void parse_dfl_box_lut(const T *box_ptr, int num_anchor,
                              int anchor_idx, int feat_w, int stride,
                              const std::array<float, 256> &exp_lut,
                              float *x1, float *y1, float *x2, float *y2) {
  constexpr int kBoxValueCount = 4;
  constexpr int kRegMax = 16;
  float box_values[kBoxValueCount];

  for (int i = 0; i < kBoxValueCount; i++) {
    float sum_softmax = 0.0f;
    float sum_value = 0.0f;
    const int channel_offset = i * kRegMax;
    for (int j = 0; j < kRegMax; j++) {
      int lut_index = exp_lut_index(
          box_ptr[(channel_offset + j) * num_anchor + anchor_idx]);
      float exp_value = exp_lut[lut_index];
      sum_softmax += exp_value;
      sum_value += exp_value * j;
    }
    box_values[i] = sum_value / sum_softmax;
  }

  int anchor_y = anchor_idx / feat_w;
  int anchor_x = anchor_idx % feat_w;
  float grid_y = anchor_y + 0.5f;
  float grid_x = anchor_x + 0.5f;
  *x1 = (grid_x - box_values[0]) * stride;
  *y1 = (grid_y - box_values[1]) * stride;
  *x2 = (grid_x + box_values[2]) * stride;
  *y2 = (grid_y + box_values[3]) * stride;
}

inline std::array<float, 256> make_int8_exp_lut(float qscale) {
  std::array<float, 256> lut;
  for (int i = 0; i < 256; i++) {
    lut[i] = std::exp((i - 128) * qscale);
  }
  return lut;
}

inline std::array<float, 256> make_uint8_exp_lut(float qscale) {
  std::array<float, 256> lut;
  for (int i = 0; i < 256; i++) {
    lut[i] = std::exp(i * qscale);
  }
  return lut;
}

template <typename T>
inline void collect_passing_cls(const T *cls_ptr, int num_anchor, int num_cls,
                                int cls_offset, float qscale,
                                float inverse_threshold,
                                std::vector<int> &anchor_indices,
                                std::vector<int> &class_indices,
                                std::vector<float> &logits,
                                std::vector<T> &max_raw,
                                std::vector<int> &max_cls) {
  max_raw.assign(num_anchor, std::numeric_limits<T>::lowest());
  max_cls.assign(num_anchor, -1);
  for (int cls = 0; cls < num_cls; cls++) {
    const T *class_ptr = cls_ptr + (cls + cls_offset) * num_anchor;
    for (int anchor = 0; anchor < num_anchor; anchor++) {
      T raw = class_ptr[anchor];
      if (raw > max_raw[anchor]) {
        max_raw[anchor] = raw;
        max_cls[anchor] = cls;
      }
    }
  }

  for (int anchor = 0; anchor < num_anchor; anchor++) {
    float max_logit = max_raw[anchor] * qscale;
    if (max_logit >= inverse_threshold) {
      anchor_indices.push_back(anchor);
      class_indices.push_back(max_cls[anchor]);
      logits.push_back(max_logit);
    }
  }
}

inline void collect_passing_cls_int8(
    const int8_t *cls_ptr, int num_anchor, int num_cls, int cls_offset,
    float qscale, float inverse_threshold, std::vector<int> &anchor_indices,
    std::vector<int> &class_indices, std::vector<float> &logits,
    std::vector<int8_t> &max_raw, std::vector<uint8_t> &max_cls) {
  const int8_t *first_class = cls_ptr + cls_offset * num_anchor;
  int anchor = 0;

  max_raw.resize(num_anchor);
  max_cls.assign(num_anchor, 0);
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
  for (; anchor + 16 <= num_anchor; anchor += 16) {
    vst1q_s8(max_raw.data() + anchor,
             vld1q_s8(first_class + anchor));
  }
#endif
  for (; anchor < num_anchor; anchor++) {
    max_raw[anchor] = first_class[anchor];
  }

  for (int cls = 1; cls < num_cls; cls++) {
    const int8_t *class_ptr =
        cls_ptr + (cls + cls_offset) * num_anchor;
    anchor = 0;
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    const uint8x16_t class_ids = vdupq_n_u8(static_cast<uint8_t>(cls));
    for (; anchor + 16 <= num_anchor; anchor += 16) {
      const int8x16_t raw = vld1q_s8(class_ptr + anchor);
      const int8x16_t current = vld1q_s8(max_raw.data() + anchor);
      const uint8x16_t select = vcgtq_s8(raw, current);
      const uint8x16_t current_ids =
          vld1q_u8(max_cls.data() + anchor);
      vst1q_s8(max_raw.data() + anchor, vmaxq_s8(raw, current));
      vst1q_u8(max_cls.data() + anchor,
                vbslq_u8(select, class_ids, current_ids));
    }
#endif
    for (; anchor < num_anchor; anchor++) {
      if (class_ptr[anchor] > max_raw[anchor]) {
        max_raw[anchor] = class_ptr[anchor];
        max_cls[anchor] = static_cast<uint8_t>(cls);
      }
    }
  }

  for (anchor = 0; anchor < num_anchor; anchor++) {
    const float max_logit = max_raw[anchor] * qscale;
    if (max_logit >= inverse_threshold) {
      anchor_indices.push_back(anchor);
      class_indices.push_back(max_cls[anchor]);
      logits.push_back(max_logit);
    }
  }
}

YoloV8Detection::YoloV8Detection(const int num_cls)
    : YoloV8Detection(std::make_pair(64, num_cls)) {}

YoloV8Detection::YoloV8Detection(std::pair<int, int> yolov8_pair) {
  net_param_.model_config.mean = {0.0, 0.0, 0.0};
  net_param_.model_config.std = {254.97195, 254.97195, 254.97195};
  net_param_.model_config.rgb_order = "rgb";
  keep_aspect_ratio_ = true;

  num_box_channel_ = yolov8_pair.first;
  num_cls_ = yolov8_pair.second;
  if (num_box_channel_ == num_cls_) {
    LOGE("error,num_box_channel_(%d) == num_cls_(%d)", num_box_channel_,
         num_cls_);
    throw std::runtime_error("num_box_channel_ == num_cls_");
  }
}

// would parse 3 cases,1:box,cls seperate feature map,2 box+cls seperate
// featuremap,3 output decoded results
int32_t YoloV8Detection::onModelOpened() {
  if (num_cls_ == 0 && !net_param_.model_config.types.empty()) {
    num_cls_ = static_cast<int>(net_param_.model_config.types.size());
  }
  const auto &input_layer = net_->getInputNames()[0];
  auto input_shape = net_->getTensorInfo(input_layer).shape;
  int input_h = input_shape[2];
  int input_w = input_shape[3];
  strides.clear();
  class_out_names.clear();
  bbox_out_names.clear();
  bbox_class_out_names.clear();
  output_branches_.clear();
  const auto &output_layers = net_->getOutputNames();
  size_t num_output = output_layers.size();

  LOGI("to parse output branch,box_channel:%d,num_cls:%d", num_box_channel_,
       num_cls_);
  if (num_cls_ == 0) {
    LOGI(
        "num_cls is 0,would take branches whose channel not equal to "
        "num_box_channel_ as cls branch");
  }
  for (size_t j = 0; j < num_output; j++) {
    auto oinfo = net_->getTensorInfo(output_layers[j]);
    int feat_h = oinfo.shape[2];
    int feat_w = oinfo.shape[3];
    int channel = oinfo.shape[1];
    int stride_h = input_h / feat_h;
    int stride_w = input_w / feat_w;

    // if (stride_h == 0 && num_output == 2) {
    //   if (channel == num_cls_) {
    //     class_out_names[stride_h] = output_layers[j];
    //     strides.push_back(stride_h);
    //     LOGI("parse class decode branch:%s,channel:%d\n",
    //          output_layers[j].c_str(), channel);
    //   } else {
    //     bbox_out_names[stride_h] = output_layers[j];
    //     LOGI("parse box decode branch:%s,channel:%d\n",
    //          output_layers[j].c_str(), channel);
    //   }
    //   continue;
    // }

    if (stride_h != stride_w) {
      LOGE("stride not equal,stridew:%d,strideh:%d,featw:%d,feath:%d\n",
           stride_w, stride_h, feat_w, feat_h);
      return -1;
    }
    if (num_cls_ == 0) {
      if (channel == num_box_channel_) {
        bbox_out_names[stride_h] = output_layers[j];
        strides.push_back(stride_h);
        LOGI("parse box branch,name:%s,stride:%d\n", output_layers[j].c_str(),
             stride_h);
      } else {
        num_cls_ = channel;
        class_out_names[stride_h] = output_layers[j];
        LOGI("parse class branch,name:%s,stride:%d\n", output_layers[j].c_str(),
             stride_h);
      }
    } else {
      if (channel == num_box_channel_) {
        bbox_out_names[stride_h] = output_layers[j];
        strides.push_back(stride_h);
        LOGI("parse box branch,name:%s,stride:%d\n", output_layers[j].c_str(),
             stride_h);
      } else if (channel == num_cls_) {
        class_out_names[stride_h] = output_layers[j];
        LOGI("parse class branch,name:%s,stride:%d\n", output_layers[j].c_str(),
             stride_h);
      } else if (channel == (num_box_channel_ + num_cls_)) {
        strides.push_back(stride_h);
        bbox_class_out_names[stride_h] = output_layers[j];
        LOGI("parse box+class branch,name: %s,stride:%d\n",
             output_layers[j].c_str(), stride_h);
      } else {
        LOGE("unexpected branch:%s,channel:%d\n", output_layers[j].c_str(),
             channel);
        return -1;
      }
    }
  }
  if (bbox_out_names.size() != class_out_names.size()) {
    LOGE(
        "Unsupported output type: mismatched number of box and class branches. "
        "bbox branches: %zu, class branches: %zu. "
        "Possible cause: num_classes = 64 or other unsupported configuration.",
        bbox_out_names.size(), class_out_names.size());
    return -1;
  }

  input_tensor_name_ = input_layer;
  input_tensor_info_ = net_->getTensorInfo(input_tensor_name_);
  size_t total_anchors = 0;
  for (int stride : strides) {
    OutputBranch branch;
    std::string class_name;
    std::string box_name;

    branch.stride = stride;
    if (class_out_names.count(stride) != 0) {
      class_name = class_out_names[stride];
      box_name = bbox_out_names[stride];
    } else {
      class_name = bbox_class_out_names[stride];
      box_name = class_name;
      branch.cls_offset = num_box_channel_;
    }

    branch.class_info = net_->getTensorInfo(class_name);
    branch.box_info = net_->getTensorInfo(box_name);
    branch.class_tensor = net_->getOutputTensor(class_name);
    branch.box_tensor = net_->getOutputTensor(box_name);
    branch.num_anchor = branch.class_info.shape[2] * branch.class_info.shape[3];
    branch.box_num_anchor = branch.box_info.shape[2] * branch.box_info.shape[3];
    branch.box_feat_w = branch.box_info.shape[3];
    branch.cls_qscale =
        branch.class_info.tensor_size / branch.class_info.tensor_elem == 1
            ? branch.class_info.qscale
            : 1.0f;
    branch.box_qscale = branch.box_info.qscale;
    if (branch.box_info.data_type == TDLDataType::INT8) {
      branch.exp_lut = make_int8_exp_lut(branch.box_qscale);
    } else if (branch.box_info.data_type == TDLDataType::UINT8) {
      branch.exp_lut = make_uint8_exp_lut(branch.box_qscale);
    }
    total_anchors += branch.num_anchor;
    output_branches_.push_back(std::move(branch));
  }

  class_boxes_.resize(num_cls_);
  for (auto &boxes : class_boxes_) {
    boxes.reserve(64);
  }
  passing_anchor_indices_.reserve(total_anchors);
  passing_class_indices_.reserve(total_anchors);
  passing_logits_.reserve(total_anchors);
  inverse_threshold_ = std::log(model_threshold_ / (1.0f - model_threshold_));

  return 0;
}

YoloV8Detection::~YoloV8Detection() {}

void YoloV8Detection::setModelThreshold(float threshold) {
  BaseModel::setModelThreshold(threshold);
  if (threshold <= 0.0f) {
    inverse_threshold_ = -std::numeric_limits<float>::infinity();
  } else if (threshold >= 1.0f) {
    inverse_threshold_ = std::numeric_limits<float>::infinity();
  } else {
    inverse_threshold_ = std::log(threshold / (1.0f - threshold));
  }
}

// the bbox featuremap shape is b x 4*regmax x h   x w
void YoloV8Detection::decodeBboxFeatureMap(int batch_idx, int stride,
                                           int anchor_idx,
                                           std::vector<float> &decode_box) {
  std::string box_name;
  if (bbox_out_names.count(stride)) {
    box_name = bbox_out_names[stride];
  } else if (bbox_class_out_names.count(stride)) {
    box_name = bbox_class_out_names[stride];
  }
  TensorInfo boxinfo = net_->getTensorInfo(box_name);
  std::shared_ptr<BaseTensor> box_tensor = net_->getOutputTensor(box_name);

  int num_channel = boxinfo.shape[1];
  int num_anchor = boxinfo.shape[2] * boxinfo.shape[3];
  int box_val_num = 4;
  int reg_max = 16;
  if (num_box_channel_ != box_val_num * reg_max) {
    LOGE("box channel size not ok,got:%d\n", num_channel);
  }

  int32_t feat_w = boxinfo.shape[3];

  int anchor_y = anchor_idx / feat_w;
  int anchor_x = anchor_idx % feat_w;

  float grid_y = anchor_y + 0.5;
  float grid_x = anchor_x + 0.5;

  std::vector<float> grid_logits;  // 4x16
  float qscale = boxinfo.qscale;
  if (boxinfo.data_type == TDLDataType::INT8) {
    int8_t *p_box_int8 = box_tensor->getBatchPtr<int8_t>(batch_idx);
    grid_logits = get_box_vals(p_box_int8, num_anchor, anchor_idx,
                               num_box_channel_, qscale);
  } else if (boxinfo.data_type == TDLDataType::UINT8) {
    uint8_t *p_box_uint8 = box_tensor->getBatchPtr<uint8_t>(batch_idx);
    grid_logits = get_box_vals(p_box_uint8, num_anchor, anchor_idx,
                               num_box_channel_, qscale);
  } else if (boxinfo.data_type == TDLDataType::FP32) {
    float *p_box_float = box_tensor->getBatchPtr<float>(batch_idx);
    grid_logits = get_box_vals(p_box_float, num_anchor, anchor_idx,
                               num_box_channel_, qscale);
  } else {
    LOGE("unsupported data type:%d\n", static_cast<int>(boxinfo.data_type));
    return;
  }

  // compute softmax and accumulate val per 16
  std::vector<float> box_vals;
  for (int i = 0; i < box_val_num; i++) {
    float sum_softmax = 0;
    float sum_val = 0;
    for (int j = 0; j < reg_max; j++) {
      float expv = exp(grid_logits[i * reg_max + j]);
      sum_softmax += expv;
      sum_val += expv * j;
    }
    sum_softmax = sum_val / sum_softmax;
    box_vals.push_back(sum_softmax);
  }

  std::vector<float> box = {
      (grid_x - box_vals[0]) * stride, (grid_y - box_vals[1]) * stride,
      (grid_x + box_vals[2]) * stride, (grid_y + box_vals[3]) * stride};
  decode_box = box;
}
int32_t YoloV8Detection::outputParse(
    const std::vector<std::shared_ptr<BaseImage>> &images,
    std::vector<std::shared_ptr<ModelOutputInfo>> &out_datas) {
  const uint32_t input_width = input_tensor_info_.shape[3];
  const uint32_t input_height = input_tensor_info_.shape[2];
  float input_width_f = float(input_width);
  float input_height_f = float(input_height);
  for (uint32_t b = 0; b < images.size(); b++) {
    uint32_t image_width = images[b]->getWidth();
    uint32_t image_height = images[b]->getHeight();

    for (auto &boxes : class_boxes_) {
      boxes.clear();
    }
    for (const OutputBranch &branch : output_branches_) {
      int8_t *cls_int8 = nullptr;
      uint8_t *cls_uint8 = nullptr;
      float *cls_float = nullptr;
      if (branch.class_info.data_type == TDLDataType::INT8) {
        cls_int8 = branch.class_tensor->getBatchPtr<int8_t>(b);
      } else if (branch.class_info.data_type == TDLDataType::UINT8) {
        cls_uint8 = branch.class_tensor->getBatchPtr<uint8_t>(b);
      } else if (branch.class_info.data_type == TDLDataType::FP32) {
        cls_float = branch.class_tensor->getBatchPtr<float>(b);
      } else {
        LOGE("unsupported class data type:%d\n",
             static_cast<int>(branch.class_info.data_type));
        return -1;
      }

      int8_t *box_int8 = nullptr;
      uint8_t *box_uint8 = nullptr;
      float *box_float = nullptr;
      if (branch.box_info.data_type == TDLDataType::INT8) {
        box_int8 = branch.box_tensor->getBatchPtr<int8_t>(b);
      } else if (branch.box_info.data_type == TDLDataType::UINT8) {
        box_uint8 = branch.box_tensor->getBatchPtr<uint8_t>(b);
      } else if (branch.box_info.data_type == TDLDataType::FP32) {
        box_float = branch.box_tensor->getBatchPtr<float>(b);
      } else {
        LOGE("unsupported box data type:%d\n",
             static_cast<int>(branch.box_info.data_type));
        return -1;
      }
      passing_anchor_indices_.clear();
      passing_class_indices_.clear();
      passing_logits_.clear();
      if (cls_int8 != nullptr) {
        collect_passing_cls_int8(
            cls_int8, branch.num_anchor, num_cls_, branch.cls_offset,
            branch.cls_qscale, inverse_threshold_, passing_anchor_indices_,
            passing_class_indices_, passing_logits_, max_class_int8_,
            max_class_ids8_);
      } else if (cls_uint8 != nullptr) {
        collect_passing_cls(
            cls_uint8, branch.num_anchor, num_cls_, branch.cls_offset,
            branch.cls_qscale, inverse_threshold_, passing_anchor_indices_,
            passing_class_indices_, passing_logits_, max_class_uint8_,
            max_class_indices_);
      } else {
        collect_passing_cls(
            cls_float, branch.num_anchor, num_cls_, branch.cls_offset,
            branch.cls_qscale, inverse_threshold_, passing_anchor_indices_,
            passing_class_indices_, passing_logits_, max_class_float_,
            max_class_indices_);
      }
      for (size_t candidate = 0; candidate < passing_anchor_indices_.size();
           candidate++) {
        int anchor = passing_anchor_indices_[candidate];
        int max_logit_c = passing_class_indices_[candidate];
        float max_logit = passing_logits_[candidate];
        float score = 1 / (1 + exp(-max_logit));
        float box_x1 = 0.0f;
        float box_y1 = 0.0f;
        float box_x2 = 0.0f;
        float box_y2 = 0.0f;
        if (box_int8 != nullptr) {
          parse_dfl_box_lut(
              box_int8, branch.box_num_anchor, anchor, branch.box_feat_w,
              branch.stride, branch.exp_lut, &box_x1, &box_y1, &box_x2,
              &box_y2);
        } else if (box_uint8 != nullptr) {
          parse_dfl_box_lut(
              box_uint8, branch.box_num_anchor, anchor, branch.box_feat_w,
              branch.stride, branch.exp_lut, &box_x1, &box_y1, &box_x2,
              &box_y2);
        } else {
          parse_dfl_box(box_float, branch.box_num_anchor, anchor,
                        branch.box_feat_w, branch.stride, branch.box_qscale,
                        &box_x1, &box_y1, &box_x2, &box_y2);
        }
        ObjectBoxInfo bbox;
        bbox.score = score;
        bbox.x1 = std::max(0.0f, std::min(box_x1, input_width_f));
        bbox.y1 = std::max(0.0f, std::min(box_y1, input_height_f));
        bbox.x2 = std::max(0.0f, std::min(box_x2, input_width_f));
        bbox.y2 = std::max(0.0f, std::min(box_y2, input_height_f));
        bbox.class_id = max_logit_c;
        class_boxes_[max_logit_c].push_back(bbox);
      }
    }

    size_t output_count = 0;
    for (auto &boxes : class_boxes_) {
      nms_objects_in_place(boxes, nms_threshold_, nms_mask_);
      output_count += boxes.size();
    }
    const std::vector<float> &scale_params =
        batch_rescale_params_[input_tensor_name_][b];

    std::shared_ptr<ModelBoxInfo> obj = std::make_shared<ModelBoxInfo>();
    obj->image_width = image_width;
    obj->image_height = image_height;
    obj->bboxes.reserve(output_count);
    for (auto &boxes : class_boxes_) {
      for (auto &box : boxes) {
        DetectionHelper::rescaleBbox(box, scale_params);
        box.x1 = std::max(0.0f, std::min(box.x1, (float)image_width));
        box.y1 = std::max(0.0f, std::min(box.y1, (float)image_height));
        box.x2 = std::max(0.0f, std::min(box.x2, (float)image_width));
        box.y2 = std::max(0.0f, std::min(box.y2, (float)image_height));
        auto type = type_mapping_.find(box.class_id);
        if (type != type_mapping_.end()) {
          box.object_type = type->second;
        }
        obj->bboxes.push_back(box);
      }
    }
    out_datas.push_back(obj);
  }
  return 0;
}
