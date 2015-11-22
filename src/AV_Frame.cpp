#include "AV_Frame.h"



namespace AV
{
Frame::Frame()
    :frameValidity(false), dataBufferIsInitialized(false)
{

}

Frame::Frame(const AVCodecContext* ctx)
{
    setup(ctx);
}


Frame::Frame(int width, int height, enum AVPixelFormat pix_fmt)
{
}


/*Frame& Frame::operator++ ()     // prefix ++
{
    frm->pts = frm->pts + 1;
    return *this;
}


Frame  Frame::operator++ (int)  // postfix ++
{
    Frame result(*this);   // make a copy for result
    ++(*this);              // Now use the prefix version to do the work
    return result;          // return the copy (the old) value.
}*/



Frame::~Frame()
{
    if (sws_ctx)
    {
        sws_freeContext(sws_ctx);
    }

    if (!frameValidity) return;
    av_frame_free(&frm);
}


bool Frame::setup(const AVCodecContext *ctx)
{
    frm = av_frame_alloc();
    frameValidity = (frm != NULL);
    if (!frameValidity) return false;

    //TODO : Check if width and height are 2 multiples
    frm->format = ctx->pix_fmt;
    frm->width = ctx->width;
    frm->height = ctx->height;


    if (av_image_alloc(frm->data, frm->linesize, frm->width, frm->height, ctx->pix_fmt, 32) <0 )
    {
        ofLogWarning("AV_Frame") << "Could not allocate raw picture buffer";
    }

    dataBufferIsInitialized = (av_frame_get_buffer(frm, 32) >= 0);

    sws_ctx = sws_getContext(ctx->width, ctx->height, AV_PIX_FMT_RGBA,
                             ctx->width, ctx->height, AV_PIX_FMT_RGB24,
                             0, 0, 0, 0);

    return dataBufferIsInitialized;
}

bool Frame::scale(ofPixels& pixels)
{
   if (sws_ctx)
   {
       uint8_t * inData[1] = { pixels.getData() }; // RGBA32 have one plane
       //
       // NOTE: In a more general setting, the rows of your input image may
       //       be padded; that is, the bytes per row may not be 4 * width.
       //       In such cases, inLineSize should be set to that padded width.
       //
       int inLinesize[1] = { pixels.getBytesStride() };
       sws_scale(sws_ctx, inData, inLinesize, 0, pixels.getHeight(), frm->data, frm->linesize);
       frm->pts = frm->pts + 1;
       return true;
   }
    // sws_scale(sws_ctx, inData, inLinesize, 0, ctx->height, frame->data, frame->linesize);
    return false;
}

bool Frame::operator<<(ofPixels& pixels)
{
    return scale(pixels);
}



void Frame::fill(int frame_index)
{
    int x, y, i;

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    if (av_frame_make_writable(frm) < 0) return;

    i = frame_index;

	/*if (picture->format == AV_PIX_FMT_YUV420P) {
		// Y 
		for (y = 0; y < picture->height; y++)
			for (x = 0; x < picture->width; x++)
				picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;

		// Cb and Cr 
		for (y = 0; y < picture->height / 2; y++) {
			for (x = 0; x < picture->width / 2; x++) {
				picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
				picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
			}
		}
	}
	*/
	
    uint8_t *rgba32Data = new uint8_t[4*frm->width*frm->height];
	
	
	
	
    
    SwsContext * ctx = sws_getContext(frm->width, frm->height,
                                      AV_PIX_FMT_RGBA, frm->width, frm->height,
                                      AV_PIX_FMT_YUV420P, 0, 0, 0, 0);
    
    
    /* encode 1 second of video */
        /* prepare a dummy image */
        /* Y */
        //        for (y = 0; y < c->height; y++) {
        //            for (x = 0; x < c->width; x++) {
        //                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
        //            }
        //        }
        //
        //        /* Cb and Cr */
        //        for (y = 0; y < c->height/2; y++) {
        //            for (x = 0; x < c->width/2; x++) {
        //                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
        //                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
        //            }
        //        }
        
	uint8_t *pos = rgba32Data;
    for (y = 0; y < frm->height; y++)
	{
        for (x = 0; x < frm->width; x++)
		{
			pos[0] = i / (float)250 * 255;
            pos[1] = y / (float)(frm->height) * 255;
            pos[2] = x / (float)(frm->width) * 255;
			pos[3] = 255;
			pos += 4;
		}
	}

	uint8_t * inData[1] = { rgba32Data }; // RGBA32 have one plane
	//
	// NOTE: In a more general setting, the rows of your input image may
	//       be padded; that is, the bytes per row may not be 4 * width.
	//       In such cases, inLineSize should be set to that padded width.
	//
    int inLinesize[1] = { 4*frm->width }; // RGBA stride
    sws_scale(ctx, inData, inLinesize, 0, frm->height, frm->data, frm->linesize);
}

void Frame::copy(void *src)
{
    static int i = 0;


    if (src==NULL) return;
    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    if (av_frame_make_writable(frm) < 0) return;

    /*for (y = 0; y < picture->height; y++)
        for (x = 0; x < picture->width; x++)
        {
            picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;
            picture->data[1][y * picture->linesize[1] + x] = x + y + i * 3;
            picture->data[2][y * picture->linesize[2] + x] = x + y + i * 3;
            picture->data[3][y * picture->linesize[3] + x] = x + y + i * 3;
        }*/
    /* Cb and Cr */
    /*for (y = 0; y < picture->height / 2; y++) {
        for (x = 0; x < picture->width / 2; x++) {
            picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
            picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
        }
    }*/

    i++;

    ofLogNotice()<< ".";
    //memcpy(picture->data[0],src,picture->height*picture->width*4);
}

} // Enf of namespace AV


