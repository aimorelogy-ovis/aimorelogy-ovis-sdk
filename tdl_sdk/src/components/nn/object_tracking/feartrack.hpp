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
  void recordPerformance(uint64_t template_us, uint64_t search_us,
                         uint64_t input_sync_us, uint64_t forward_us,
                         uint64_t output_sync_us, uint64_t post_us,
                         uint64_t total_us, bool template_cache_hit);

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
  bool template_input_cached_ = false;
  uint64_t perf_window_start_us_ = 0;
  uint64_t perf_frames_ = 0;
  uint64_t perf_template_cache_hits_ = 0;
  uint64_t perf_template_us_ = 0;
  uint64_t perf_search_us_ = 0;
  uint64_t perf_input_sync_us_ = 0;
  uint64_t perf_forward_us_ = 0;
  uint64_t perf_output_sync_us_ = 0;
  uint64_t perf_post_us_ = 0;
  uint64_t perf_total_us_ = 0;
};
