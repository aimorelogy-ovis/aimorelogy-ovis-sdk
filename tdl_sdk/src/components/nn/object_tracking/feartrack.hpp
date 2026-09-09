#pragma once

#include "image/base_image.hpp"
#include "model/base_model.hpp"

class FearTrack final : public BaseModel {
 public:
  FearTrack();
  ~FearTrack();

  virtual int32_t outputParse(
      const std::vector<std::shared_ptr<BaseImage>>& images,
      std::vector<std::shared_ptr<ModelOutputInfo>>& out_datas) override;

  virtual int32_t outputParse(
      const std::vector<std::vector<std::shared_ptr<BaseImage>>>& images,
      std::vector<std::shared_ptr<ModelOutputInfo>>& out_datas) override;
  virtual int32_t inference(
      const std::vector<std::vector<std::shared_ptr<BaseImage>>>& images,
      std::vector<std::shared_ptr<ModelOutputInfo>>& out_datas,
      const std::map<std::string, float>& parameters = {}) override;
  virtual int32_t onModelOpened() override;
  virtual void invalidateInputCache() override;

 private:
  // 生成网格坐标
  void makeGrid();

  // // 模型参数
  int instance_size_ = 256;           // 实例大小
  int template_size_ = 128;           // 模板大小
  float template_bbox_offset_ = 0.2;  // 模板边界框偏移
  float search_bbox_offset_ = 2.0;    // 搜索边界框偏移
  int score_size_ = 16;               // 得分大小
  int total_stride_ = 16;             // 总步长

  // 网格坐标
  std::vector<std::vector<int>> grid_x_;
  std::vector<std::vector<int>> grid_y_;
  std::string template_input_name_;
  std::string search_input_name_;
  std::string regression_output_name_;
  std::string score_output_name_;
  std::shared_ptr<BaseTensor> template_tensor_;
  std::shared_ptr<BaseTensor> search_tensor_;
  std::shared_ptr<BaseTensor> regression_tensor_;
  std::shared_ptr<BaseTensor> score_tensor_;
  TensorInfo regression_info_ = {};
  TensorInfo score_info_ = {};
  bool template_input_cached_ = false;
};
