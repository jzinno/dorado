#pragma once

#include "ModelRunner.h"

#include <torch/torch.h>

#include <filesystem>
#include <memory>
#include <vector>

namespace dorado {

class MetalCaller;

std::shared_ptr<MetalCaller> create_metal_caller(const std::filesystem::path& model_path,
                                                 int chunk_size,
                                                 int batch_size);

class MetalModelRunner final : public ModelRunnerBase {
public:
    explicit MetalModelRunner(std::shared_ptr<MetalCaller> caller);
    void accept_chunk(int chunk_idx, const torch::Tensor& chunk) final;
    std::vector<DecodedChunk> call_chunks(int num_chunks) final;
    size_t model_stride() const final;
    size_t chunk_size() const final;
    size_t batch_size() const final;

private:
    std::shared_ptr<MetalCaller> m_caller;
    torch::Tensor m_input;
};

}  // namespace dorado
