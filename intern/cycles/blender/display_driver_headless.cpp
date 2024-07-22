/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2021-2022 Blender Foundation */

#include "blender/display_driver_headless.h"

// for image output
#include "BKE_image.h"
#include "BKE_image_format.h"
#include "BLI_string.h"

#include "../imbuf/IMB_imbuf.h"
#include "../imbuf/IMB_imbuf_types.h"

#include <OpenImageIO/detail/fmt.h>
#include <filesystem>


CCL_NAMESPACE_BEGIN

/* --------------------------------------------------------------------
 * BlenderDisplayDriverHeadless.
 */

#define example_with_image_format_off

std::string write_buffer_to_image_file(half4 *buffer, int width, int height)
{
  static std::atomic<int> counter = 0;
  std::string filepath = fmt::format("d:/blender_output_test_{}.exr", counter++);

#ifdef example_with_image_format

  ImageFormatData imf;
  BKE_image_format_init(&imf, false /*render*/);
  imf.color_management = R_IMF_COLOR_MANAGEMENT_FOLLOW_SCENE;
  // Set the desired color space
  //STRNCPY(imf.linear_colorspace_settings.name, "Linear");
  //STRNCPY(imf.display_settings.display_device, "sRGB");
  STRNCPY(imf.linear_colorspace_settings.name, "Linear");
  STRNCPY(imf.display_settings.display_device, "None");
  STRNCPY(imf.view_settings.look, "None");
  STRNCPY(imf.view_settings.view_transform, "Standart");
  imf.planes = R_IMF_PLANES_RGBA;
  // Set the desired format space
  //imf.imtype= BKE_imtype_from_arg("PNG");
  imf.imtype= BKE_imtype_from_arg("EXR");
  imf.depth = R_IMF_CHAN_DEPTH_8;

#endif

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

  ibuf->ftype = IMB_FTYPE_OPENEXR;
  ibuf->foptions.flag |= OPENEXR_COMPRESS | OPENEXR_HALF;

  IMB_saveiff(ibuf, filepath.c_str(), IB_rect);

#ifdef example_with_image_format
  BKE_imbuf_write(ibuf, filepath.c_str(), &imf);
#endif

  IMB_freeImBuf(ibuf);
#ifdef example_with_image_format
  BKE_image_format_free(&imf);
#endif

  return filepath;
}

BlenderDisplayDriverHeadless::BlenderDisplayDriverHeadless(OMI_render_manager* orc_, const std::string &output_path_)
  :output_path{output_path_}
  , orc{orc_}
{
  std::remove(output_path.c_str());
}

BlenderDisplayDriverHeadless::~BlenderDisplayDriverHeadless() {}

void
BlenderDisplayDriverHeadless::next_tile_begin() {};

bool
BlenderDisplayDriverHeadless::update_begin(const Params &/*params*/, int width, int height) {
  width_ = width;
  height_ = height;
#ifdef ABLINOV_DEV_OFF // test output
    fmt::print(stderr, "ABlinov: update_begin() image dimension: {} {}", width, height);
#endif
  return true;
}

void
BlenderDisplayDriverHeadless::update_end() {
#ifdef ABLINOV_DEV_OFF // test output
  fmt::print(stderr, "ABlinov: update_end()");
#endif
  auto file_from = write_buffer_to_image_file(buffer.data(), width_, height_);
  std::error_code err;
  do{
    std::filesystem::rename(file_from, output_path, err);
    if(err.value() != NO_ERROR){
      fmt::print(stderr, "ABlinov: rename error: {}", err.message());
    }
  }while(err.value() != NO_ERROR);
  orc->notifyAboutChanges();
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


// OMI_render_manager
//###################################################################
void OMI_render_manager::startTask(std::function<void()> job)
{
  {
    std::unique_lock<std::mutex> lk(cv_m);
    nThreadsRunning++;
  }

  workers.emplace_back([&,job]()
  {
    job();
    {
      std::unique_lock<std::mutex> lk(cv_m);
      nThreadsRunning--;
    }
    cv.notify_all();
  });
}

void OMI_render_manager::notifyAboutChanges()
{
  std::unique_lock<std::mutex> lk(cv_m);
  changes_counter++;
  cv.notify_all();
}

void OMI_render_manager::waitForChanges()
{
  std::unique_lock<std::mutex> lk(cv_m);
  cv.wait(lk, [this]{
    return changes_counter > 0 || nThreadsRunning == 0;
  });
  changes_counter = 0;
}

bool OMI_render_manager::isAllRenderFinished()
{
  return nThreadsRunning == 0;
}

void OMI_render_manager::waitForFinalCleanUp()
{
  for(auto& thread : workers)
    thread.join();
  workers.clear();
}


CCL_NAMESPACE_END
