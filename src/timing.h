#ifndef VGA_TIMING_H
#define VGA_TIMING_H

#include "stdint.h"

namespace Vga
{

/*
 * Describes the horizontal and vertical timing for a display mode, including
 * the outer bounds of active video.
 */
struct Timing
{
    enum struct Polarity
    {
        positive = 0,
        negative = 1,
    };

    double pixel_frequency_mhz; // Pixel frequency in MHz

    /*
    * Horizontal timing, expressed in pixels.
    *
    * The horizontal sync pulse implicitly starts at pixel zero of the line.
    *
    * Some of this information is redundant; it's stored this way to avoid
    * having to rederive it in the driver.
    */
    uint16_t line_pixels;       // Total, including blanking.
    uint16_t sync_pixels;       // Length of pulse.
    uint16_t back_porch_pixels; // Between end of sync and start of video.
    uint16_t video_lead;        // Fudge factor: nudge DMA back in time.
    uint16_t video_pixels;      // Maximum pixels in active video.
    Polarity hsync_polarity;    // Polarity of hsync pulse.

    /*
    * Vertical timing, expressed in lines.
    *
    * Because vertical timing is done in software, it's a little more flexible
    * than horizontal timing.
    */
    uint16_t vsync_start_line; // Top edge of sync pulse.
    uint16_t vsync_end_line;   // Bottom edge of sync pulse.
    uint16_t video_start_line; // Top edge of active video.
    uint16_t video_end_line;   // Bottom edge of active video.
    Polarity vsync_polarity;   // Polarity of vsync pulse.
};

/*
 * Canned timings for common modes.
 */
extern Timing const timing_640x480_60hz;
extern Timing const timing_640x480_60_01hz;
extern Timing const timing_720x400_70hz;

} // namespace Vga

#endif // VGA_TIMING_H
