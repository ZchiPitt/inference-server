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
 * @brief Implements the Python bindings for the inference_tensor.hpp header
 */

#include "amdinfer/core/inference_tensor.hpp"

#include <pybind11/attr.h>      // for keep_alive
#include <pybind11/cast.h>      // for arg
#include <pybind11/numpy.h>     // for array_t
#include <pybind11/pybind11.h>  // for class_, init
#include <pybind11/stl.h>       // IWYU pragma: keep

#include <array>          // for array
#include <cstring>        // for memcpy
#include <sstream>        // IWYU pragma: keep
#include <unordered_map>  // for unordered_map

#include "amdinfer/bindings/python/helpers/docstrings.hpp"  // for DOCS
#include "amdinfer/bindings/python/helpers/keep_alive.hpp"  // for keep_alive
#include "amdinfer/bindings/python/helpers/print.hpp"       // for toString
#include "amdinfer/core/inference_response.hpp"

namespace py = pybind11;

namespace amdinfer {

void wrapInferenceTensor(py::module_ &m) {
  py::class_<InferenceTensor, Tensor>(m, "InferenceTensor")
    .def(py::init<std::string, std::vector<int64_t>, amdinfer::DataType>(),
         DOCS(InferenceTensor, InferenceTensor), py::arg("name"),
         py::arg("shape"), py::arg("dataType"))
    .def(py::init<Tensor>(), DOCS(InferenceTensor, InferenceTensor, 2),
         py::arg("tensor"))
    .def_property("parameters", &InferenceTensor::getParameters,
                  &InferenceTensor::setParameters)
    .def("__repr__", [](const InferenceTensor &self) {
      (void)self;
      return "InferenceTensor";
    });
}

}  // namespace amdinfer
