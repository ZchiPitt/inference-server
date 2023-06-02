// Copyright 2023 Advanced Micro Devices, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file
 * @brief
 */

#include "system_under_test.hpp"

#include <loadgen.h>

#include <cassert>

#include "query_sample_library.hpp"

namespace amdinfer {

const std::string& SystemUnderTest::Name() const { return name_; }

SystemUnderTest::SystemUnderTest(QuerySampleLibrary* qsl, Client* client,
                                 std::string endpoint)
  : qsl_(qsl), client_(client), endpoint_(std::move(endpoint)) {
  waitUntilServerReady(client_);
  waitUntilModelReady(client_, endpoint_);
  std::thread{&SystemUnderTest::FinishQuery, this}.detach();
  // std::thread{&SystemUnderTest::FinishQuery, this}.detach();
}

void SystemUnderTest::IssueQuery(
  const std::vector<mlperf::QuerySample>& samples) {
  for (const auto& sample : samples) {
    auto& request = qsl_->getSample(sample.index);
    request.setID(std::to_string(sample.id));
    auto response = client_->modelInferAsync(endpoint_, request);
    // std::cout << ": " << sample.id << ": " <<
    // std::chrono::high_resolution_clock::now().time_since_epoch().count() <<
    // ": " << std::endl;
    queue_.enqueue(std::move(response));
  }
}

// NOLINTNEXTLINE(readability-identifier-naming)
[[noreturn]] void SystemUnderTest::FinishQuery() {
  const int max = 16;  // arbitrary
  while (true) {
    std::vector<InferenceResponseFuture> futures;
    futures.reserve(max);
    auto returned = queue_.wait_dequeue_bulk(futures.begin(), max);
    for (auto i = 0U; i < returned; ++i) {
      auto& future = futures[i];

      auto response = future.get();
      if (response.isError()) {
        std::cout << "Error encountered in response. App may hang\n";
      } else {
        const auto& outputs = response.getOutputs();
        assert(outputs.size() == 1);
        const auto& output = outputs[0];
        auto data = reinterpret_cast<uintptr_t>(output.getData());
        mlperf::ResponseId id = std::stoul(response.getID());
        mlperf::QuerySampleResponse result{id, data, output.getSize()};
        // std::cout << "::" << id << "::" <<
        // std::chrono::high_resolution_clock::now().time_since_epoch().count()
        // << "::" << std::endl;
        mlperf::QuerySamplesComplete(&result, 1);
      }
    }
  }
}

void SystemUnderTest::FlushQueries() {}

void SystemUnderTest::ReportLatencyResults(
  [[maybe_unused]] const std::vector<mlperf::QuerySampleLatency>&
    latencies_ns) {}

}  // namespace amdinfer
