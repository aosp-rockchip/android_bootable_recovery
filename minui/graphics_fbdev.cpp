/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "graphics_fbdev.h"

#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>

#include <android-base/unique_fd.h>

#include "minui/minui.h"

#include <android-base/properties.h>

#define EINK_FB_SIZE		(0x400000) /* 4M */
#define GET_EBC_BUFFER		(0x7000)
#define SET_EBC_SEND_BUFFER	(0x7001)
#define GET_EBC_BUFFER_INFO (0x7002)

#define EPD_NULL            (-1)
#define EPD_AUTO            (0)
#define EPD_OVERLAY         (1)
#define EPD_FULL_GC16       (2)
#define EPD_FULL_GL16       (3)
#define EPD_FULL_GLR16     	(4)
#define EPD_FULL_GLD16      (5)
#define EPD_FULL_GCC16     	(6)
#define EPD_PART_GC16       (7)
#define EPD_PART_GL16       (8)
#define EPD_PART_GLR16      (9)
#define EPD_PART_GLD16		(10)
#define EPD_PART_GCC16     	(11)
#define EPD_A2       		(12)
#define EPD_DU				(13)
#define EPD_RESET        	(14)
#define EPD_SUSPEND        	(15)
#define EPD_RESUME        	(16)
#define EPD_POWER_OFF        (17)
#define EPD_PART_EINK        (18)
#define EPD_FULL_EINK        (19)

/*android use struct*/
struct ebc_buf_info_t{
  int offset;
  int epd_mode;
  int height;
  int width;
  int panel_color;
  int win_x1;
  int win_y1;
  int win_x2;
  int win_y2;
  int width_mm;
  int height_mm;
}__packed;

extern "C" {
    void neon_rgb888_to_gray16ARM_32(uint8_t * dest,uint8_t *  src,int h,int w,int vir_w);
}
extern "C" {
    void neon_rgb888_to_gray16ARM_16(uint8_t * dest,uint8_t *  src,int h,int w,int vir_w);
}

unsigned long ebc_buffer_base = 0;

void neon_rgb888_to_gray16ARM(uint8_t * dest,uint8_t *  src,int h,int w,int vir_w)
{
  if ((vir_w % 32) == 0) {
    neon_rgb888_to_gray16ARM_32(dest,src,h,w,vir_w);
  }
  else {
    neon_rgb888_to_gray16ARM_16(dest,src,h,w,vir_w);
  }
}

std::unique_ptr<GRSurfaceFbdev> GRSurfaceFbdev::Create(size_t width, size_t height,
                                                       size_t row_bytes, size_t pixel_bytes) {
  // Cannot use std::make_unique to access non-public ctor.
  return std::unique_ptr<GRSurfaceFbdev>(new GRSurfaceFbdev(width, height, row_bytes, pixel_bytes));
}

void MinuiBackendFbdev::Blank(bool blank) {
  bool is_eink_fb = android::base::GetBoolProperty("sys.eink.recovery.eink_fb", false);
  if (is_eink_fb) {
    return;
  } else {
    int ret = ioctl(fb_fd, FBIOBLANK, blank ? FB_BLANK_POWERDOWN : FB_BLANK_UNBLANK);
    if (ret < 0) perror("ioctl(): blank");
  }
}

void MinuiBackendFbdev::SetDisplayedFramebuffer(size_t n) {
  bool is_eink_fb = android::base::GetBoolProperty("sys.eink.recovery.eink_fb", false);

  if (is_eink_fb) {
    return;
  } else {
    if (n > 1 || !double_buffered) return;

    vi.yres_virtual = gr_framebuffer[0]->height * 2;
    vi.yoffset = n * gr_framebuffer[0]->height;
    vi.bits_per_pixel = gr_framebuffer[0]->pixel_bytes * 8;
    if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vi) < 0) {
      perror("active fb swap failed");
    }
    displayed_buffer = n;
  }
}

GRSurface* MinuiBackendFbdev::Init() {
  bool is_eink_fb = android::base::GetBoolProperty("sys.eink.recovery.eink_fb", false);
  printf("Fbdev Init is_eink_fb=%d\n", is_eink_fb);
  if (is_eink_fb) {
    struct ebc_buf_info_t ebc_buf_info;
    void* vaddr = NULL;

    android::base::unique_fd ebc_fd(open("/dev/ebc", O_RDWR | O_CLOEXEC));
    if (ebc_fd == -1) {
      perror("cannot open open /dev/ebc\n");
      return nullptr;
    }

    if (ioctl(ebc_fd, GET_EBC_BUFFER_INFO, &ebc_buf_info) != 0) {
      perror("cannot get ebc buffer info\n");
      return nullptr;
    }

    vaddr = mmap(0, EINK_FB_SIZE*4, PROT_READ|PROT_WRITE, MAP_SHARED, ebc_fd, 0);
    if (vaddr == MAP_FAILED) {
      perror("Error mapping the ebc buffer\n");
      return nullptr;
    }
    ebc_buffer_base = intptr_t(vaddr);


    gr_framebuffer[0] =
        GRSurfaceFbdev::Create(ebc_buf_info.width, ebc_buf_info.height, ebc_buf_info.width * 4, 4);
    memory_buffer.resize(gr_framebuffer[0]->height * gr_framebuffer[0]->row_bytes);
    gr_framebuffer[0]->buffer_ = memory_buffer.data();
    memset(gr_framebuffer[0]->buffer_, 0, gr_framebuffer[0]->height * gr_framebuffer[0]->row_bytes);
	gr_draw = gr_framebuffer[0].get();
    memset(gr_draw->buffer_, 0, gr_draw->height * gr_draw->row_bytes);

	fb_fd = std::move(ebc_fd);
    printf("framebuffer: %d (%zu x %zu)\n", fb_fd.get(), gr_draw->width, gr_draw->height);
  } else {
    android::base::unique_fd fd(open("/dev/graphics/fb0", O_RDWR | O_CLOEXEC));
    if (fd == -1) {
      perror("cannot open fb0");
      return nullptr;
    }

    fb_fix_screeninfo fi;
    if (ioctl(fd, FBIOGET_FSCREENINFO, &fi) < 0) {
      perror("failed to get fb0 info");
      return nullptr;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) {
      perror("failed to get fb0 info");
      return nullptr;
    }

    // We print this out for informational purposes only, but
    // throughout we assume that the framebuffer device uses an RGBX
    // pixel format.  This is the case for every development device I
    // have access to.  For some of those devices (eg, hammerhead aka
    // Nexus 5), FBIOGET_VSCREENINFO *reports* that it wants a
    // different format (XBGR) but actually produces the correct
    // results on the display when you write RGBX.
    //
    // If you have a device that actually *needs* another pixel format
    // (ie, BGRX, or 565), patches welcome...

    printf(
        "fb0 reports (possibly inaccurate):\n"
        "  vi.bits_per_pixel = %d\n"
        "  vi.red.offset   = %3d   .length = %3d\n"
        "  vi.green.offset = %3d   .length = %3d\n"
        "  vi.blue.offset  = %3d   .length = %3d\n",
        vi.bits_per_pixel, vi.red.offset, vi.red.length, vi.green.offset, vi.green.length,
        vi.blue.offset, vi.blue.length);

    void* bits = mmap(0, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bits == MAP_FAILED) {
      perror("failed to mmap framebuffer");
      return nullptr;
    }

    memset(bits, 0, fi.smem_len);

    gr_framebuffer[0] =
        GRSurfaceFbdev::Create(vi.xres, vi.yres, fi.line_length, vi.bits_per_pixel / 8);
    gr_framebuffer[0]->buffer_ = static_cast<uint8_t*>(bits);
    memset(gr_framebuffer[0]->buffer_, 0, gr_framebuffer[0]->height * gr_framebuffer[0]->row_bytes);

    gr_framebuffer[1] =
        GRSurfaceFbdev::Create(gr_framebuffer[0]->width, gr_framebuffer[0]->height,
                               gr_framebuffer[0]->row_bytes, gr_framebuffer[0]->pixel_bytes);

    /* check if we can use double buffering */
    if (vi.yres * fi.line_length * 2 <= fi.smem_len) {
      double_buffered = true;

      gr_framebuffer[1]->buffer_ =
          gr_framebuffer[0]->buffer_ + gr_framebuffer[0]->height * gr_framebuffer[0]->row_bytes;
    } else {
      double_buffered = false;

      // Without double-buffering, we allocate RAM for a buffer to draw in, and then "flipping" the
      // buffer consists of a memcpy from the buffer we allocated to the framebuffer.
      memory_buffer.resize(gr_framebuffer[1]->height * gr_framebuffer[1]->row_bytes);
      gr_framebuffer[1]->buffer_ = memory_buffer.data();
    }

    gr_draw = gr_framebuffer[1].get();
    memset(gr_draw->buffer_, 0, gr_draw->height * gr_draw->row_bytes);
    fb_fd = std::move(fd);
    SetDisplayedFramebuffer(0);

    printf("framebuffer: %d (%zu x %zu)\n", fb_fd.get(), gr_draw->width, gr_draw->height);

    Blank(true);
    Blank(false);
  }

  return gr_draw;
}

GRSurface* MinuiBackendFbdev::Flip() {
  bool is_eink_fb = android::base::GetBoolProperty("sys.eink.recovery.eink_fb", false);

  if (is_eink_fb) {
    struct ebc_buf_info_t buf_info;

    if (ioctl(fb_fd, GET_EBC_BUFFER, &buf_info) != 0) {
        perror("GET_EBC_BUFFER failed\n");
    }
    neon_rgb888_to_gray16ARM((uint8_t*)(ebc_buffer_base + buf_info.offset), (uint8_t*)(gr_draw->buffer_), buf_info.height, buf_info.width, buf_info.width);
    buf_info.win_x1= 0;
    buf_info.win_y1= 0;
    buf_info.win_x2= buf_info.width;
    buf_info.win_y2= buf_info.height;
    buf_info.epd_mode = EPD_PART_GC16;

    if (ioctl(fb_fd, SET_EBC_SEND_BUFFER, &buf_info) !=0 ) {
        perror("SET_EBC_SEND_BUFFER failed");
    }
  } else {
    if (double_buffered) {
      // Change gr_draw to point to the buffer currently displayed, then flip the driver so we're
      // displaying the other buffer instead.
      gr_draw = gr_framebuffer[displayed_buffer].get();
      SetDisplayedFramebuffer(1 - displayed_buffer);
    } else {
      // Copy from the in-memory surface to the framebuffer.
      memcpy(gr_framebuffer[0]->buffer_, gr_draw->buffer_, gr_draw->height * gr_draw->row_bytes);
    }
  }
  return gr_draw;
}
