//
// H264VideoDecoder_ffmpeg.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_VIDEO_DECODER_FFMPEHG__
#define __H264_VIDEO_DECODER_FFMPEHG__
#define AV_CODEC_FLAG_GLOBAL_HEADER
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include "H264PicturesGOP.h"
#include "H264Picture.h"

#include "H264VideoDecoder.h"
#include "FileReader.h"
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264Golomb.h"


//H264��������������һ֡��Ļص�����
typedef int (*output_frame_callback)(CH264Picture* outPicture, void* userData, int errorCode);


class CH264VideoDecoder_ffmpeg
{
public:
    std::string              m_filename;
    output_frame_callback    m_output_frame_callback; //������ϵ�֡�Ļص���������Ҫ�û���������
    void* m_userData;

private:
    AVFormatContext* m_format_ctx;   // �����ļ�������
    AVCodecContext* m_codec_ctx;     // ������������
    int m_video_stream_index;        // ��Ƶ������
    SwsContext* m_sws_ctx;           // ͼ�����������ģ����ڸ�ʽת����
public:
    CH264VideoDecoder_ffmpeg();
    ~CH264VideoDecoder_ffmpeg();

    int init();
    int unInit();

    int set_output_frame_callback_functuin(output_frame_callback output_frame_callback, void* userData);

    int open(const char* url);
    int open_ffmpeg(const char* url);
    int do_callback(CH264Picture* picture_current, CH264PicturesGOP* pictures_gop, int32_t is_need_flush);

    void CH264VideoDecoder_ffmpeg::ConvertAVFrameToCH264Picture(const AVFrame* avFrame, CH264Picture* picture);

    void close();

};

#endif //__H264_VIDEO_DECODER_FFMPEHG__
#pragma once
