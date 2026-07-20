#include "feartrack.hpp"
#include "utils/tdl_log.hpp"

template <typename T>
inline void parse_score_data(T* p_score_ptr, int score_size, float qscale,
                             float* max_score, int* max_i, int* max_j) {
  float best_score = -1;
  int best_i = -1, best_j = -1;

  // 遍历整个score map (16x16)，使用argmax与Python对齐
  for (int i = 0; i < score_size; i++) {
    for (int j = 0; j < score_size; j++) {
      float score =
          static_cast<float>(p_score_ptr[i * score_size + j]) * qscale;
      if (score > best_score) {
        best_score = score;
        best_i = i;
        best_j = j;
      }
    }
  }

  *max_score = best_score;
  *max_i = best_i;
  *max_j = best_j;
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

void FearTrack::invalidateInputCache() { template_input_cached_ = false; }

int32_t FearTrack::onModelOpened() {
  // 获取输入输出层信息
  const auto& input_layers = net_->getInputNames();
  const auto& output_layers = net_->getOutputNames();

  if (input_layers.size() != 2 || output_layers.size() != 2) {
    LOGE("模型输入输出层数量不符合预期，输入层：%zu，输出层：%zu",
         input_layers.size(), output_layers.size());
    return -1;
  }
  invalidateInputCache();
  return 0;
}

int32_t FearTrack::inference(
    const std::vector<std::vector<std::shared_ptr<BaseImage>>>& images,
    std::vector<std::shared_ptr<ModelOutputInfo>>& out_datas,
    const std::map<std::string, float>& parameters) {
  if (images.size() != 1 || images[0].size() != 2 || !images[0][0] ||
      !images[0][1]) {
    LOGE("FearTrack expects one template/search image pair");
    return -1;
  }
  if (!preprocessor_ || !net_) {
    LOGE("FearTrack model is not ready");
    return -1;
  }

  const std::vector<std::string>& input_names = net_->getInputNames();
  if (input_names.size() != 2) {
    LOGE("FearTrack input count mismatch: %zu", input_names.size());
    return -1;
  }

  std::shared_ptr<BaseTensor> template_tensor =
      net_->getInputTensor(input_names[0]);
  std::shared_ptr<BaseTensor> search_tensor =
      net_->getInputTensor(input_names[1]);
  if (!template_tensor || !search_tensor) {
    LOGE("FearTrack input tensor is null");
    return -1;
  }

  model_timer_.TicToc("runstart");
  if (!template_input_cached_) {
    int32_t ret = preprocessor_->preprocessToTensor(
        images[0][0], preprocess_params_[input_names[0]], 0,
        template_tensor);
    if (ret != 0) {
      LOGE("FearTrack template preprocess failed with %#x", ret);
      return ret;
    }
    template_input_cached_ = true;
  }

  PreprocessParams search_params = preprocess_params_[input_names[1]];
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
      images[0][1], search_params, 0, search_tensor);
  if (ret != 0) {
    LOGE("FearTrack search preprocess failed with %#x", ret);
    return ret;
  }
  model_timer_.TicToc("preprocess");

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
  model_timer_.TicToc("tpu");

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
  // 获取回归和分类输出层名称
  std::string regression_output_name = net_->getOutputNames()[0];
  std::string score_output_name = net_->getOutputNames()[1];

  // 获取回归和分类输出张量
  std::shared_ptr<BaseTensor> regression_tensor =
      net_->getOutputTensor(regression_output_name);
  std::shared_ptr<BaseTensor> score_tensor =
      net_->getOutputTensor(score_output_name);

  // 获取张量信息
  TensorInfo regression_info = net_->getTensorInfo(regression_output_name);
  TensorInfo score_info = net_->getTensorInfo(score_output_name);

  // 遍历批次
  for (uint32_t b = 0; b < images.size(); b++) {
    // 创建输出结构
    std::shared_ptr<ModelBoxInfo> track_result =
        std::make_shared<ModelBoxInfo>();
    track_result->image_width = instance_size_;
    track_result->image_height = instance_size_;

    // 找到最高得分位置
    float max_score = -1;
    int max_i = -1, max_j = -1;

    // 根据数据类型处理score数据
    if (score_info.data_type == TDLDataType::INT8) {
      parse_score_data<int8_t>(score_tensor->getBatchPtr<int8_t>(b),
                               score_size_, score_info.qscale, &max_score,
                               &max_i, &max_j);
    } else if (score_info.data_type == TDLDataType::UINT8) {
      parse_score_data<uint8_t>(score_tensor->getBatchPtr<uint8_t>(b),
                                score_size_, score_info.qscale, &max_score,
                                &max_i, &max_j);
    } else if (score_info.data_type == TDLDataType::FP32) {
      parse_score_data<float>(score_tensor->getBatchPtr<float>(b), score_size_,
                              1.0f, &max_score, &max_i, &max_j);
    } else {
      LOGE("不支持的数据类型:%d\n", static_cast<int>(score_info.data_type));
      return -1;
    }

    if (max_i >= 0 && max_j >= 0) {
      // 解析边界框
      float x1, y1, x2, y2;

      // 根据数据类型处理regression数据
      if (regression_info.data_type == TDLDataType::INT8) {
        parse_regression_data<int8_t>(regression_tensor->getBatchPtr<int8_t>(b),
                                      score_size_, max_i, max_j,
                                      regression_info.qscale, &x1, &y1, &x2,
                                      &y2, grid_x_, grid_y_);
      } else if (regression_info.data_type == TDLDataType::UINT8) {
        parse_regression_data<uint8_t>(
            regression_tensor->getBatchPtr<uint8_t>(b), score_size_, max_i,
            max_j, regression_info.qscale, &x1, &y1, &x2, &y2, grid_x_,
            grid_y_);
      } else if (regression_info.data_type == TDLDataType::FP32) {
        parse_regression_data<float>(regression_tensor->getBatchPtr<float>(b),
                                     score_size_, max_i, max_j, 1.0f, &x1, &y1,
                                     &x2, &y2, grid_x_, grid_y_);
      } else {
        LOGE("不支持的数据类型:%d\n",
             static_cast<int>(regression_info.data_type));
        return -1;
      }
      ObjectBoxInfo bbox;
      bbox.score = max_score;
      bbox.x1 = x1;
      bbox.y1 = y1;
      bbox.x2 = x2;
      bbox.y2 = y2;

      // 添加到结果中
      track_result->bboxes.push_back(bbox);

    }

    out_datas.push_back(track_result);
  }

  return 0;
}
