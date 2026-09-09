#include "feartrack.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include "utils/tdl_log.hpp"

namespace {

constexpr int kFearTrackCandidateCount = 3;
constexpr int kFearTrackPeakSuppressionRadius = 1;

inline double feartrack_monotonic_time_ms() {
  return std::chrono::duration<double, std::milli>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

struct FearTrackPeak {
  float score = -1.0f;
  int i = -1;
  int j = -1;
};

inline FearTrackPeak make_feartrack_peak(float score, int i, int j) {
  FearTrackPeak peak;
  peak.score = score;
  peak.i = i;
  peak.j = j;
  return peak;
}

}  // namespace

template <typename T>
inline void parse_score_data(T* p_score_ptr, int score_size, float qscale,
                             float* max_score, int* max_i, int* max_j) {
  float peak_score = -1.0f;
  int peak_i = -1;
  int peak_j = -1;

  for (int i = 0; i < score_size; i++) {
    for (int j = 0; j < score_size; j++) {
      const float score =
          static_cast<float>(p_score_ptr[i * score_size + j]) * qscale;
      if (score > peak_score) {
        peak_score = score;
        peak_i = i;
        peak_j = j;
      }
    }
  }

  float second_score = -1.0f;
  for (int i = 0; i < score_size; i++) {
    for (int j = 0; j < score_size; j++) {
      if (std::abs(i - peak_i) <= 1 && std::abs(j - peak_j) <= 1) {
        continue;
      }
      const float score =
          static_cast<float>(p_score_ptr[i * score_size + j]) * qscale;
      second_score = std::max(second_score, score);
    }
  }

  float best_average = -1.0f;
  int average_i = -1;
  int average_j = -1;
  for (int i = 0; i < score_size; i++) {
    for (int j = 0; j < score_size; j++) {
      const float center_score =
          static_cast<float>(p_score_ptr[i * score_size + j]) * qscale;
      if (center_score <= 0.2f) {
        continue;
      }

      float sum = 0.0f;
      int count = 0;
      for (int dy = -2; dy <= 2; dy++) {
        const int y = i + dy;
        if (y < 0 || y >= score_size) {
          continue;
        }
        for (int dx = -2; dx <= 2; dx++) {
          const int x = j + dx;
          if (x < 0 || x >= score_size) {
            continue;
          }
          sum += static_cast<float>(p_score_ptr[y * score_size + x]) * qscale;
          count++;
        }
      }
      const float average = sum / std::max(count, 1);
      if (average > best_average) {
        best_average = average;
        average_i = i;
        average_j = j;
      }
    }
  }

  const bool confident_peak = peak_score >= 0.35f &&
                              peak_score - second_score >= 0.08f;
  if (confident_peak || average_i < 0) {
    *max_score = peak_score;
    *max_i = peak_i;
    *max_j = peak_j;
    return;
  }

  float local_peak_score = -1.0f;
  int local_peak_i = peak_i;
  int local_peak_j = peak_j;
  for (int i = std::max(0, average_i - 2);
       i <= std::min(score_size - 1, average_i + 2); i++) {
    for (int j = std::max(0, average_j - 2);
         j <= std::min(score_size - 1, average_j + 2); j++) {
      const float score =
          static_cast<float>(p_score_ptr[i * score_size + j]) * qscale;
      if (score > local_peak_score) {
        local_peak_score = score;
        local_peak_i = i;
        local_peak_j = j;
      }
    }
  }
  *max_score = local_peak_score;
  *max_i = local_peak_i;
  *max_j = local_peak_j;
}

template <typename T>
inline void calculate_response_quality(T* p_score_ptr, int score_size,
                                       float qscale, int peak_i, int peak_j,
                                       float peak_score,
                                       float* second_score,
                                       float* peak_margin, float* psr) {
  constexpr int kPeakExclusionRadius = 1;
  double sum = 0.0;
  double sum_sq = 0.0;
  int count = 0;
  float sidelobe_max = -1.0f;

  if (peak_i < 0 || peak_j < 0) {
    *second_score = peak_score;
    *peak_margin = 0.0f;
    *psr = 0.0f;
    return;
  }

  for (int i = 0; i < score_size; i++) {
    for (int j = 0; j < score_size; j++) {
      if (std::abs(i - peak_i) <= kPeakExclusionRadius &&
          std::abs(j - peak_j) <= kPeakExclusionRadius) {
        continue;
      }
      const float score =
          static_cast<float>(p_score_ptr[i * score_size + j]) * qscale;
      sidelobe_max = std::max(sidelobe_max, score);
      sum += score;
      sum_sq += static_cast<double>(score) * score;
      count++;
    }
  }

  if (count == 0) {
    *second_score = peak_score;
    *peak_margin = 0.0f;
    *psr = 0.0f;
    return;
  }

  const double mean = sum / count;
  const double variance = std::max(0.0, sum_sq / count - mean * mean);
  const double stddev = std::sqrt(variance);
  *second_score = sidelobe_max;
  *peak_margin = peak_score - sidelobe_max;
  *psr = static_cast<float>((peak_score - mean) / (stddev + 1e-6));
}

template <typename T>
inline int collect_score_peaks(
    T* p_score_ptr, int score_size, float qscale,
    const FearTrackPeak& primary,
    std::array<FearTrackPeak, kFearTrackCandidateCount>* peaks) {
  if (p_score_ptr == nullptr || peaks == nullptr || primary.i < 0 ||
      primary.j < 0) {
    return 0;
  }

  (*peaks)[0] = primary;
  int peak_count = 1;
  while (peak_count < kFearTrackCandidateCount) {
    FearTrackPeak candidate;
    for (int i = 0; i < score_size; i++) {
      for (int j = 0; j < score_size; j++) {
        bool suppressed = false;
        for (int selected = 0; selected < peak_count; selected++) {
          if (std::abs(i - (*peaks)[selected].i) <=
                  kFearTrackPeakSuppressionRadius &&
              std::abs(j - (*peaks)[selected].j) <=
                  kFearTrackPeakSuppressionRadius) {
            suppressed = true;
            break;
          }
        }
        if (suppressed) {
          continue;
        }

        const float score =
            static_cast<float>(p_score_ptr[i * score_size + j]) * qscale;
        if (score > candidate.score) {
          candidate.score = score;
          candidate.i = i;
          candidate.j = j;
        }
      }
    }
    if (candidate.i < 0 || candidate.j < 0) {
      break;
    }
    (*peaks)[peak_count++] = candidate;
  }
  return peak_count;
}

template <typename T>
inline void parse_regression_data(T* p_reg_ptr, int score_size, int i, int j,
                                  float qscale, float* x1, float* y1, float* x2,
                                  float* y2,
                                  const std::vector<std::vector<int>>& grid_x,
                                  const std::vector<std::vector<int>>& grid_y) {
  // 直接从regression map中获取四个通道的值
  float reg_x1 =
      static_cast<float>(
          p_reg_ptr[0 * score_size * score_size + i * score_size + j]) *
      qscale;
  float reg_y1 =
      static_cast<float>(
          p_reg_ptr[1 * score_size * score_size + i * score_size + j]) *
      qscale;
  float reg_x2 =
      static_cast<float>(
          p_reg_ptr[2 * score_size * score_size + i * score_size + j]) *
      qscale;
  float reg_y2 =
      static_cast<float>(
          p_reg_ptr[3 * score_size * score_size + i * score_size + j]) *
      qscale;

  *x1 = grid_x[i][j] - std::exp(reg_x1);
  *y1 = grid_y[i][j] - std::exp(reg_y1);
  *x2 = grid_x[i][j] + std::exp(reg_x2);
  *y2 = grid_y[i][j] + std::exp(reg_y2);
}

FearTrack::FearTrack() {
  net_param_.model_config.mean = {123.675, 116.28, 103.53};
  net_param_.model_config.std = {58.395, 57.12, 57.375};
  net_param_.model_config.rgb_order = "rgb";
  makeGrid();
}

FearTrack::~FearTrack() {}

void FearTrack::invalidateInputCache() {
  template_input_cached_ = false;
}

int32_t FearTrack::onModelOpened() {
  // 获取输入输出层信息
  const auto& input_layers = net_->getInputNames();
  const auto& output_layers = net_->getOutputNames();

  if (input_layers.size() != 2 || output_layers.size() != 2) {
    LOGE("模型输入输出层数量不符合预期，输入层：%zu，输出层：%zu",
         input_layers.size(), output_layers.size());
    return -1;
  }
  template_input_name_ = input_layers[0];
  search_input_name_ = input_layers[1];
  regression_output_name_ = output_layers[0];
  score_output_name_ = output_layers[1];
  template_tensor_ = net_->getInputTensor(template_input_name_);
  search_tensor_ = net_->getInputTensor(search_input_name_);
  regression_tensor_ = net_->getOutputTensor(regression_output_name_);
  score_tensor_ = net_->getOutputTensor(score_output_name_);
  regression_info_ = net_->getTensorInfo(regression_output_name_);
  score_info_ = net_->getTensorInfo(score_output_name_);
  if (!template_tensor_ || !search_tensor_ || !regression_tensor_ ||
      !score_tensor_) {
    LOGE("FearTrack模型张量初始化失败");
    return -1;
  }
  invalidateInputCache();
  return 0;
}

int32_t FearTrack::inference(
    const std::vector<std::vector<std::shared_ptr<BaseImage>>>& images,
    std::vector<std::shared_ptr<ModelOutputInfo>>& out_datas,
    const std::map<std::string, float>& parameters) {
  const double total_start_ms = feartrack_monotonic_time_ms();
  double stage_start_ms = total_start_ms;
  last_performance_ = {};
  if (images.size() != 1 || images[0].size() != 2 || !images[0][0] ||
      !images[0][1]) {
    LOGE("FearTrack expects one template/search image pair");
    return -1;
  }
  if (!preprocessor_ || !net_) {
    LOGE("FearTrack model is not ready");
    return -1;
  }

  if (template_input_name_.empty() || search_input_name_.empty() ||
      !template_tensor_ || !search_tensor_) {
    LOGE("FearTrack input tensors are not initialized");
    return -1;
  }

  model_timer_.TicToc("runstart");
  if (!template_input_cached_) {
    int32_t ret = preprocessor_->preprocessToTensor(
        images[0][0], preprocess_params_[template_input_name_], 0,
        template_tensor_);
    if (ret != 0) {
      LOGE("FearTrack template preprocess failed with %#x", ret);
      return ret;
    }
    template_input_cached_ = true;
  }

  PreprocessParams search_params = preprocess_params_[search_input_name_];
  const auto crop_x = parameters.find("search_crop_x");
  const auto crop_y = parameters.find("search_crop_y");
  const auto crop_width = parameters.find("search_crop_width");
  const auto crop_height = parameters.find("search_crop_height");
  if (crop_x != parameters.end() && crop_y != parameters.end() &&
      crop_width != parameters.end() && crop_height != parameters.end()) {
    search_params.crop_x = static_cast<int>(crop_x->second);
    search_params.crop_y = static_cast<int>(crop_y->second);
    search_params.crop_width = static_cast<int>(crop_width->second);
    search_params.crop_height = static_cast<int>(crop_height->second);
    search_params.keep_aspect_ratio = false;
  }

  int32_t ret = preprocessor_->preprocessToTensor(
      images[0][1], search_params, 0, search_tensor_);
  if (ret != 0) {
    LOGE("FearTrack search preprocess failed with %#x", ret);
    return ret;
  }
  last_performance_.preprocess_ms =
      feartrack_monotonic_time_ms() - stage_start_ms;
  model_timer_.TicToc("preprocess");

  stage_start_ms = feartrack_monotonic_time_ms();
  ret = net_->updateInputTensors();
  if (ret != 0) {
    LOGE("FearTrack update input tensors failed with %#x", ret);
    return ret;
  }
  ret = net_->forward();
  if (ret != 0) {
    LOGE("FearTrack inference failed with %#x", ret);
    return ret;
  }
  last_performance_.tpu_ms =
      feartrack_monotonic_time_ms() - stage_start_ms;
  model_timer_.TicToc("tpu");

  stage_start_ms = feartrack_monotonic_time_ms();
  ret = net_->updateOutputTensors();
  if (ret != 0) {
    LOGE("FearTrack update output tensors failed with %#x", ret);
    return ret;
  }
  std::vector<std::shared_ptr<ModelOutputInfo>> results;
  ret = outputParse(images, results);
  if (ret != 0) {
    LOGE("FearTrack output parse failed with %#x", ret);
    return ret;
  }
  model_timer_.TicToc("post");
  out_datas.insert(out_datas.end(), results.begin(), results.end());
  last_performance_.postprocess_ms =
      feartrack_monotonic_time_ms() - stage_start_ms;
  last_performance_.total_ms =
      feartrack_monotonic_time_ms() - total_start_ms;
  return 0;
}

void FearTrack::makeGrid() {
  // 初始化网格坐标数组
  grid_x_.resize(score_size_, std::vector<int>(score_size_));
  grid_y_.resize(score_size_, std::vector<int>(score_size_));

  // 生成网格坐标
  for (int y = 0; y < score_size_; ++y) {
    for (int x = 0; x < score_size_; ++x) {
      grid_x_[y][x] = x * total_stride_;
      grid_y_[y][x] = y * total_stride_;
    }
  }
}

int32_t FearTrack::outputParse(
    const std::vector<std::shared_ptr<BaseImage>>& images,
    std::vector<std::shared_ptr<ModelOutputInfo>>& out_datas) {
  return 0;
}

int32_t FearTrack::outputParse(
    const std::vector<std::vector<std::shared_ptr<BaseImage>>>& images,
    std::vector<std::shared_ptr<ModelOutputInfo>>& out_datas) {
  if (!regression_tensor_ || !score_tensor_) {
    LOGE("FearTrack output tensors are not initialized");
    return -1;
  }

  // 遍历批次
  for (uint32_t b = 0; b < images.size(); b++) {
    // 创建输出结构
    std::shared_ptr<ModelTrackInfo> track_result =
        std::make_shared<ModelTrackInfo>();
    track_result->image_width = instance_size_;
    track_result->image_height = instance_size_;
    track_result->bboxes.reserve(kFearTrackCandidateCount);

    // 保留原始主峰选择，同时附带两个非重叠候选供小目标时序判定。
    float max_score = -1;
    int max_i = -1, max_j = -1;
    std::array<FearTrackPeak, kFearTrackCandidateCount> peaks;
    int peak_count = 0;

    // 根据数据类型处理score数据
    if (score_info_.data_type == TDLDataType::INT8) {
      parse_score_data<int8_t>(score_tensor_->getBatchPtr<int8_t>(b),
                               score_size_, score_info_.qscale, &max_score,
                               &max_i, &max_j);
      peak_count = collect_score_peaks<int8_t>(
          score_tensor_->getBatchPtr<int8_t>(b), score_size_,
          score_info_.qscale, make_feartrack_peak(max_score, max_i, max_j),
          &peaks);
    } else if (score_info_.data_type == TDLDataType::UINT8) {
      parse_score_data<uint8_t>(score_tensor_->getBatchPtr<uint8_t>(b),
                                score_size_, score_info_.qscale, &max_score,
                                &max_i, &max_j);
      peak_count = collect_score_peaks<uint8_t>(
          score_tensor_->getBatchPtr<uint8_t>(b), score_size_,
          score_info_.qscale, make_feartrack_peak(max_score, max_i, max_j),
          &peaks);
    } else if (score_info_.data_type == TDLDataType::FP32) {
      parse_score_data<float>(score_tensor_->getBatchPtr<float>(b), score_size_,
                              1.0f, &max_score, &max_i, &max_j);
      peak_count = collect_score_peaks<float>(
          score_tensor_->getBatchPtr<float>(b), score_size_, 1.0f,
          make_feartrack_peak(max_score, max_i, max_j), &peaks);
    } else {
      LOGE("不支持的数据类型:%d\n", static_cast<int>(score_info_.data_type));
      return -1;
    }

    for (int candidate_index = 0; candidate_index < peak_count;
         candidate_index++) {
      max_score = peaks[candidate_index].score;
      max_i = peaks[candidate_index].i;
      max_j = peaks[candidate_index].j;
      if (max_i < 0 || max_j < 0) {
        continue;
      }
      // 解析边界框
      float x1, y1, x2, y2;
      float second_score = 0.0f;
      float peak_margin = 0.0f;
      float response_psr = 0.0f;

      if (score_info_.data_type == TDLDataType::INT8) {
        calculate_response_quality<int8_t>(
            score_tensor_->getBatchPtr<int8_t>(b), score_size_,
            score_info_.qscale, max_i, max_j, max_score, &second_score,
            &peak_margin, &response_psr);
      } else if (score_info_.data_type == TDLDataType::UINT8) {
        calculate_response_quality<uint8_t>(
            score_tensor_->getBatchPtr<uint8_t>(b), score_size_,
            score_info_.qscale, max_i, max_j, max_score, &second_score,
            &peak_margin, &response_psr);
      } else {
        calculate_response_quality<float>(
            score_tensor_->getBatchPtr<float>(b), score_size_, 1.0f, max_i,
            max_j, max_score, &second_score, &peak_margin, &response_psr);
      }

      // 根据数据类型处理regression数据
      if (regression_info_.data_type == TDLDataType::INT8) {
        parse_regression_data<int8_t>(regression_tensor_->getBatchPtr<int8_t>(b),
                                      score_size_, max_i, max_j,
                                      regression_info_.qscale, &x1, &y1, &x2,
                                      &y2, grid_x_, grid_y_);
      } else if (regression_info_.data_type == TDLDataType::UINT8) {
        parse_regression_data<uint8_t>(
            regression_tensor_->getBatchPtr<uint8_t>(b), score_size_, max_i,
            max_j, regression_info_.qscale, &x1, &y1, &x2, &y2, grid_x_,
            grid_y_);
      } else if (regression_info_.data_type == TDLDataType::FP32) {
        parse_regression_data<float>(regression_tensor_->getBatchPtr<float>(b),
                                     score_size_, max_i, max_j, 1.0f, &x1, &y1,
                                     &x2, &y2, grid_x_, grid_y_);
      } else {
        LOGE("不支持的数据类型:%d\n",
             static_cast<int>(regression_info_.data_type));
        return -1;
      }
      ObjectBoxInfo bbox;
      bbox.score = max_score;
      bbox.x1 = x1;
      bbox.y1 = y1;
      bbox.x2 = x2;
      bbox.y2 = y2;

      // 添加到结果中
      const size_t result_index = track_result->bboxes.size();
      track_result->bboxes.push_back(bbox);
      track_result->candidate_second_scores[result_index] = second_score;
      track_result->candidate_peak_margins[result_index] = peak_margin;
      track_result->candidate_psrs[result_index] = response_psr;
      track_result->response_candidate_count++;
    }

    if (track_result->response_candidate_count > 0) {
      track_result->response_second_score =
          track_result->candidate_second_scores[0];
      track_result->response_peak_margin =
          track_result->candidate_peak_margins[0];
      track_result->response_psr = track_result->candidate_psrs[0];
    }

    out_datas.push_back(track_result);
  }

  return 0;
}
