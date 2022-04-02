
#ifdef __cplusplus
extern "C"
{
#endif

#include "libavcodec/avcodec.h"

#include "libavutil/frame.h"
#include "libavutil/imgutils.h"

#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

#ifdef __cplusplus
}
#endif

namespace avpp{
    class Frame {
    public:
        Frame();
        Frame(enum AVPixelFormat pix_fmt, int width, int height);

        int width() const;
        int height() const;
        int* linesize() const;
        void pts( int64_t timestamp);
        uint8_t** dataPtr();
        AVFrame* native();

        AVFrame *frame;
    };

}