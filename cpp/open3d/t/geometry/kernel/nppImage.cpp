// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#include "open3d/t/geometry/kernel/nppImage.h"

#include <nppdefs.h>
#include <nppi.h>

#include "open3d/core/Dtype.h"
#include "open3d/core/ShapeUtil.h"
#include "open3d/core/Tensor.h"
#include "open3d/t/geometry/Image.h"
#include "open3d/utility/Console.h"

namespace open3d {
namespace t {
namespace geometry {
namespace npp {

void dilate(const core::Tensor &srcim,
            core::Tensor &dstim,
            int half_kernel_size) {
    // Supported device and datatype checking happens in calling code and will
    // result in an exception if there are errors.

    // create nask
    core::Tensor mask(core::SizeVector{2 * half_kernel_size + 1,
                                       2 * half_kernel_size + 1, 1},
                      core::Dtype::UInt8, srcim.GetDevice());
    mask.Fill(1);
    NppiSize oMaskSize = {2 * half_kernel_size + 1, 2 * half_kernel_size + 1};

    NppiSize oSrcSize = {static_cast<int>(srcim.GetShape(1)),
                         static_cast<int>(srcim.GetShape(0))};
    NppiPoint oSrcOffset = {0, 0};

    // create struct with ROI size
    NppiSize oSizeROI = {static_cast<int>(dstim.GetShape(1)),
                         static_cast<int>(dstim.GetShape(0))};
    NppiPoint oAnchor = {half_kernel_size, half_kernel_size};

    auto dt = srcim.GetDtype();
#define NPP_ARGS                                                        \
    static_cast<const npp_dtype *>(srcim.GetDataPtr()),                 \
            srcim.GetStride(0) * dt.ByteSize(), oSrcSize, oSrcOffset,   \
            static_cast<npp_dtype *>(dstim.GetDataPtr()),               \
            dstim.GetStride(0) * dt.ByteSize(), oSizeROI,               \
            static_cast<const uint8_t *>(mask.GetDataPtr()), oMaskSize, \
            oAnchor, NPP_BORDER_REPLICATE
    if (dt == core::Dtype::UInt8) {
        using npp_dtype = Npp8u;
        if (srcim.GetShape(2) == 1) {
            nppiDilateBorder_8u_C1R(NPP_ARGS);
        } else if (srcim.GetShape(2) == 3) {
            nppiDilateBorder_8u_C3R(NPP_ARGS);
        } else if (srcim.GetShape(2) == 4) {
            nppiDilateBorder_8u_C4R(NPP_ARGS);
        }
    } else if (dt == core::Dtype::UInt16) {
        using npp_dtype = Npp16u;
        if (srcim.GetShape(2) == 1) {
            nppiDilateBorder_16u_C1R(NPP_ARGS);
        } else if (srcim.GetShape(2) == 3) {
            nppiDilateBorder_16u_C3R(NPP_ARGS);
        } else if (srcim.GetShape(2) == 4) {
            nppiDilateBorder_16u_C4R(NPP_ARGS);
        }
    } else if (dt == core::Dtype::Float32) {
        using npp_dtype = Npp32f;
        if (srcim.GetShape(2) == 1) {
            nppiDilateBorder_32f_C1R(NPP_ARGS);
        } else if (srcim.GetShape(2) == 3) {
            nppiDilateBorder_32f_C3R(NPP_ARGS);
        } else if (srcim.GetShape(2) == 4) {
            nppiDilateBorder_32f_C4R(NPP_ARGS);
        }
    } else
        utility::LogError("npp::dilate(): Unspported dtype {}", dt.ToString());
#undef NPP_ARGS
}

}  // namespace npp
}  // namespace geometry
}  // namespace t
}  // namespace open3d