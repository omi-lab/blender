/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2021-2022 Blender Foundation */

#pragma once
#include "session/display_driver.h"

#include <mutex>
#include <vector>
#include <functional>
#include <thread>
#include <string>

CCL_NAMESPACE_BEGIN

std::string write_buffer_to_image_file(half4 *buffer, int width, int height);

class OMI_render_manager;

/* headless display driver implementation for intermediate render output for "headless" display */
class BlenderDisplayDriverHeadless : public DisplayDriver {
 public:
  BlenderDisplayDriverHeadless(OMI_render_manager *orc_, std::string const& output_path_);
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
  std::string output_path;
  OMI_render_manager* const orc = nullptr;
};

class OMI_render_manager{
public:
  void startTask(std::function<void()> job);
  void run();
  void waitForChanges();
  bool isAllRenderFinished();
  void waitForFinalCleanUp();
  void notifyAboutChanges();
private:
  std::vector<std::thread> workers;
  std::mutex cv_m;
  std::condition_variable cv;
  int changes_counter{0};
  int nThreadsRunning{0};
};

CCL_NAMESPACE_END
