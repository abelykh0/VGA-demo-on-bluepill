#include "timing.h"

namespace Vga
{

Timing const timing_640x480_60hz =
    {
        .pixel_frequency_mhz = 25.17,

        .line_pixels = 800,
        .sync_pixels = 96,
        .back_porch_pixels = 48,
        .video_lead = 10,
        .video_pixels = 640,
        .hsync_polarity = Timing::Polarity::positive,

        .vsync_start_line = 10,
        .vsync_end_line = 10 + 2,
        .video_start_line = 10 + 2 + 33,
        .video_end_line = 10 + 2 + 33 + 480,
        .vsync_polarity = Timing::Polarity::positive,
};

Timing const timing_640x480_60_01hz =
    {
        .pixel_frequency_mhz = 24.00,

        .line_pixels = 792,
        .sync_pixels = 88,
        .back_porch_pixels = 32,
        .video_lead = 25,
        .video_pixels = 640,
        .hsync_polarity = Timing::Polarity::negative,

        .vsync_start_line = 10,
        .vsync_end_line = 10 + 5,
        .video_start_line = 10 + 5 + 10,
        .video_end_line = 10 + 5 + 10 + 480,
        .vsync_polarity = Timing::Polarity::negative,
};

Timing const timing_720x400_70hz =
    {
        .pixel_frequency_mhz = 27.00,

        .line_pixels = 864,
        .sync_pixels = 63,
        .back_porch_pixels = 69,
        .video_lead = 25,
        .video_pixels = 720,
        .hsync_polarity = Timing::Polarity::negative,

        .vsync_start_line = 5,
        .vsync_end_line = 5 + 5,
        .video_start_line = 5 + 5 + 39,
        .video_end_line = 5 + 5 + 39 + 576,
        .vsync_polarity = Timing::Polarity::negative,
};

} // namespace Vga