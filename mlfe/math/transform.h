#ifndef __TRANSFORM_HPP__
#define __TRANSFORM_HPP__

namespace mlfe{ namespace math{

template <class DataType, class DeviceContext>
void im2col(const int im_c, const int im_h, const int im_w,
            const int kernel_h, const int kernel_w,
            const int stride, const int padding,
            const DataType *_im, DataType *_col
            );

template <class DataType, class DeviceContext>
void col2im(DataType* data_col,
            int channels, int height, int width,
            int ksize, int stride, int pad,
            DataType* data_im
            );

template <class DataType, class DeviceContext>
void MaxPool(
    const int size,
    const DataType *x, const int c,
    const int h, const int w, const int ph,
    const int pw, const int kernel_h, const int kernel_w,
    const int stride_h, const int stride_w, const int pad_h, const int pad_w,
    DataType *y, int *mask);

template <class DataType, class DeviceContext>
void MaxPoolGradient(
    const int size,
    const float *dy, const int *mask,
    const int c, const int h, const int w,
    const int ph, const int pw, const int kernel_h, const int kernel_w,
    const int stride_h, const int stride_w, const int pad_h, const int pad_w,
    float *dx);

} /* namespace math */
} /* namespace mlfe */
#endif /* __TRANSFORM_HPP__ */
