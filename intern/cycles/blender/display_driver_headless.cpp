/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2021-2022 Blender Foundation */

#include "blender/display_driver_headless.h"

// for image output
#include "BKE_image.h"
#include "BKE_image_format.h"
#include "BLI_string.h"
#include "../imbuf/IMB_imbuf.h"
#include "../imbuf/IMB_imbuf_types.h"
#include "DNA_scene_types.h"

#include <OpenImageIO/detail/fmt.h>


CCL_NAMESPACE_BEGIN

/* --------------------------------------------------------------------
 * BlenderDisplayDriverHeadless.
 */

void write_buffer_to_image_file(half4 *buffer, int width, int height)
{
  static int counter = 0;
  std::string filepath = fmt::format("d:/blender_output_test_{}.png", counter++);

  ImageFormatData imf;
  BKE_image_format_init(&imf, false /*render*/);
  // Set the desired color space
  STRNCPY(imf.linear_colorspace_settings.name, "Linear");
  STRNCPY(imf.display_settings.display_device, "sRGB");
  // Set the desired format space
  imf.imtype= BKE_imtype_from_arg("PNG");

  // allocate float buffer
  ImBuf *ibuf = IMB_allocImBuf(width, height, 32, IB_rectfloat);
  size_t imgsize = width * height;

  for (int i = 0; i < imgsize; i++) {
    auto src_i = i << 2;
    ibuf->rect_float[src_i  ] = half_to_float_image(buffer[i].x);
    ibuf->rect_float[src_i|1] = half_to_float_image(buffer[i].y);
    ibuf->rect_float[src_i|2] = half_to_float_image(buffer[i].z);
    ibuf->rect_float[src_i|3] = half_to_float_image(buffer[i].w);
  }
  BKE_imbuf_write(ibuf, filepath.c_str(), &imf);

  IMB_freeImBuf(ibuf);
  BKE_image_format_free(&imf);
}

BlenderDisplayDriverHeadless::BlenderDisplayDriverHeadless() {}
BlenderDisplayDriverHeadless::~BlenderDisplayDriverHeadless() {}

void
BlenderDisplayDriverHeadless::next_tile_begin() {};

bool
BlenderDisplayDriverHeadless::update_begin(const Params &/*params*/, int width, int height) {
  width_ = width;
  height_ = height;
  if(ABLINOV_DEV) // test ouput
    fmt::print(stderr, "ABlinov: update_begin() image dimension: {} {}", width, height);
  return true;
}

void
BlenderDisplayDriverHeadless::update_end() {
  if(ABLINOV_DEV) // test ouput
    fmt::print(stderr, "ABlinov: update_end()");
  write_buffer_to_image_file(buffer.data(), width_, height_);
}

void
BlenderDisplayDriverHeadless::flush() {}

half4 *
BlenderDisplayDriverHeadless::map_texture_buffer() {
  if(buffer.empty()) {
    buffer.resize(width_*height_);
  }

  return buffer.data();
}

void
BlenderDisplayDriverHeadless::unmap_texture_buffer() {}

void
BlenderDisplayDriverHeadless::graphics_interop_activate() {}

void
BlenderDisplayDriverHeadless::graphics_interop_deactivate() {}

void BlenderDisplayDriverHeadless::clear() {}

void BlenderDisplayDriverHeadless::draw(const Params &/*params*/) {}

CCL_NAMESPACE_END
