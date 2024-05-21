/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2021-2022 Blender Foundation */

#pragma once
#include "session/display_driver.h"

#include <vector>

CCL_NAMESPACE_BEGIN

void write_buffer_to_image_file(half4 *buffer, int width, int height);

/* headless display driver implementation for intermediate render output for "headless" display */
class BlenderDisplayDriverHeadless : public DisplayDriver {
 public:
  BlenderDisplayDriverHeadless();
  ~BlenderDisplayDriverHeadless() override;

  void next_tile_begin() override;

  bool update_begin(const Params &params, int width, int height) override;
  void update_end() override;

  void flush() override;

  half4 *map_texture_buffer() override;
  void unmap_texture_buffer() override;

  void graphics_interop_activate() override;
  void graphics_interop_deactivate() override;
  void clear() override;

  void draw(const Params &params) override;
private:
  std::vector<half4> buffer;
  size_t width_, height_;
};

CCL_NAMESPACE_END
