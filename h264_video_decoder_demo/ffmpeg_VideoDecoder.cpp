#define AV_CODEC_FLAG_GLOBAL_HEADER
#include "H264VideoDecoder.h"
#include "FileReader.h"
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264Golomb.h"
#include "H264VideoDecoder_ffmpeg.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/error.h>


}
CH264VideoDecoder_ffmpeg::CH264VideoDecoder_ffmpeg()
{
    m_filename = "";
    m_output_frame_callback = NULL;
    m_userData = NULL;
}


CH264VideoDecoder_ffmpeg::~CH264VideoDecoder_ffmpeg()
{

}

int CH264VideoDecoder_ffmpeg::init()
{
    return 0;
}

int CH264VideoDecoder_ffmpeg::unInit()
{
    return 0;
}
int CH264VideoDecoder_ffmpeg::set_output_frame_callback_functuin(output_frame_callback output_frame_callback, void* userData)
{
    m_output_frame_callback = output_frame_callback;
    m_userData = userData;
    return 0;
}



int CH264VideoDecoder_ffmpeg::open_ffmpeg(const char* url) {

    CH264PicturesGOP* picturesGop = new CH264PicturesGOP();
    int ret = 0;
    AVFormatContext* formatContext = avformat_alloc_context();
    if (!formatContext) {
        LOG_ERROR("Could not allocate AVFormatContext\n");
        return -1;
    }

    // 打开输入文件
    ret = avformat_open_input(&formatContext, url, NULL, NULL);
    if (ret < 0) {
        LOG_ERROR("Could not open input file: %s\n", ret);
        avformat_close_input(&formatContext);
        return -1;
    }

    ret = avformat_find_stream_info(formatContext, NULL);
    if (ret < 0) {
        LOG_ERROR("Could not find stream information: %s\n", ret);
        return -1;
    }
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        LOG_ERROR("Could not find video stream\n");
        return -1;
    }

    AVCodecParameters* codecParameters = formatContext->streams[videoStreamIndex]->codecpar;

    // 查找解码器
    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        LOG_ERROR("Unsupported codec\n");
        return -1;
    }

    // 分配解码器上下文
    m_codec_ctx = avcodec_alloc_context3(codec);
    if (!m_codec_ctx) {
        LOG_ERROR("Could not allocate video codec context\n");
        return -1;
    }
    ret = avcodec_parameters_to_context(m_codec_ctx, codecParameters);
    if (ret < 0) {
        LOG_ERROR("Could not copy codec parameters: %s\n", ret);
        return -1;
    }

    // 打开解码器
    if (avcodec_open2(m_codec_ctx, codec, NULL) < 0) {
        LOG_ERROR("Failed to open codec\n");
        return -1;
    }
    // 读取数据包并解码
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        LOG_ERROR("Could not allocate AVFrame\n");
        return -1;
    }

    while (av_read_frame(formatContext, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            if((ret = avcodec_send_packet(m_codec_ctx, &packet)) < 0) {
                LOG_ERROR("Error sending packet to decoder, error code: %d\n", ret);
                av_packet_unref(&packet);
                continue;
            }
            else {
                while (avcodec_receive_frame(m_codec_ctx, frame) == 0) {
                    CH264Picture* pictureCurrent = nullptr;
                    ret = picturesGop->getOneEmptyPicture(pictureCurrent);
                    if (ret != 0 || !pictureCurrent) {
                        LOG_ERROR("Failed to get an empty picture\n");
                        break;
                    }
                    ConvertAVFrameToCH264Picture(frame, pictureCurrent);
                    int32_t isNeedFlush = (frame->key_frame == 1) ? 1 : 0;
                    ret = do_callback(pictureCurrent, picturesGop, isNeedFlush);
                    if (ret != 0) {
                        LOG_ERROR("do_callback() failed! ret=%d\n", ret);
                        break;
                    }
                }
            }

        }
        av_packet_unref(&packet); // 释放数据包
    }
    av_frame_free(&frame);
    avcodec_free_context(&m_codec_ctx);
    avformat_close_input(&formatContext);

    return 0;
}

void CH264VideoDecoder_ffmpeg::ConvertAVFrameToCH264Picture(const AVFrame* avFrame, CH264Picture* picture) {
    // 检查输入参数
    if (!avFrame || !picture) {
        printf("Error: avFrame or picture is NULL\n");
        return;
    }

    // 检查 avFrame->data 是否为 NULL
    if (!avFrame->data[0] || !avFrame->data[1] || !avFrame->data[2]) {
        printf("Error: avFrame->data is NULL\n");
        return;
    }

    // 检查 avFrame->width 和 avFrame->height 是否为有效值
    if (avFrame->width <= 0 || avFrame->height <= 0) {
        printf("Error: Invalid frame dimensions (width=%d, height=%d)\n", avFrame->width, avFrame->height);
        return;
    }

    // 检查 avFrame->linesize 是否为有效值
    if (avFrame->linesize[0] <= 0 || avFrame->linesize[1] <= 0 || avFrame->linesize[2] <= 0) {
        printf("Error: Invalid linesize (linesize[0]=%d, linesize[1]=%d, linesize[2]=%d)\n",
            avFrame->linesize[0], avFrame->linesize[1], avFrame->linesize[2]);
        return;
    }


    picture->m_picture_frame.PicWidthInMbs = (avFrame->width + 15) / 16;
    picture->m_picture_frame.PicHeightInMbs = (avFrame->height + 15) / 16;
    picture->m_picture_frame.PicSizeInMbs = picture->m_picture_frame.PicWidthInMbs * picture->m_picture_frame.PicHeightInMbs;

    // 解码后的宽高
    picture->m_picture_frame.PicWidthInSamplesL = picture->m_picture_frame.PicWidthInMbs * 16;
    picture->m_picture_frame.PicWidthInSamplesC = picture->m_picture_frame.PicWidthInMbs * 8;
    picture->m_picture_frame.PicHeightInSamplesL = picture->m_picture_frame.PicHeightInMbs * 16;
    picture->m_picture_frame.PicHeightInSamplesC = picture->m_picture_frame.PicHeightInMbs * 8;

    picture->m_picture_frame.m_pic_coded_width_pixels = picture->m_picture_frame.PicWidthInMbs * 16;
    picture->m_picture_frame.m_pic_coded_height_pixels = picture->m_picture_frame.PicHeightInMbs * 16;

    // 计算 YUV 数据的大小
    int sizeY = picture->m_picture_frame.PicWidthInSamplesL * picture->m_picture_frame.PicHeightInSamplesL;
    int sizeU = picture->m_picture_frame.PicWidthInSamplesC * picture->m_picture_frame.PicHeightInSamplesC;
    int sizeV = picture->m_picture_frame.PicWidthInSamplesC * picture->m_picture_frame.PicHeightInSamplesC;
    int totalSize = sizeY + sizeU + sizeV;

    // 分配内存
    uint8_t* pic_buff = (uint8_t*)my_malloc(sizeof(uint8_t) * totalSize);
    if (!pic_buff) {
        printf("Error: Failed to allocate memory for YUV buffers\n");
        return;
    }
    memset(pic_buff, 0, sizeof(uint8_t) * totalSize);


    picture->m_picture_frame.m_pic_buff_luma = pic_buff;
    picture->m_picture_frame.m_pic_buff_cb = pic_buff + sizeY;
    picture->m_picture_frame.m_pic_buff_cr = pic_buff + sizeY + sizeU;

    // 复制 Y 分量
    for (int y = 0; y < avFrame->height; y++) {
        memcpy(picture->m_picture_frame.m_pic_buff_luma + y * avFrame->width,
            avFrame->data[0] + y * avFrame->linesize[0],
            avFrame->width);
    }

    // 复制 U 分量
    for (int y = 0; y < avFrame->height / 2; y++) {
        memcpy(picture->m_picture_frame.m_pic_buff_cb + y * (avFrame->width / 2),
            avFrame->data[1] + y * avFrame->linesize[1],
            avFrame->width / 2);
    }

    // 复制 V 分量
    for (int y = 0; y < avFrame->height / 2; y++) {
        memcpy(picture->m_picture_frame.m_pic_buff_cr + y * (avFrame->width / 2),
            avFrame->data[2] + y * avFrame->linesize[2],
            avFrame->width / 2);
    }

    //picture->m_picture_frame.m_mbs = avFrame->mb_2;
    //分配宏块内存
    //memcpy(m_mbs, src.m_mbs, sizeof(CH264MacroBlock) * PicSizeInMbs);

    int mb_sum = picture->m_picture_frame.PicSizeInMbs;
    picture->m_picture_frame.m_mbs = (CH264MacroBlock*)malloc(mb_sum * sizeof(CH264MacroBlock));


    picture->m_picture_frame.PicOrderCnt = avFrame->pts; // 假设 PTS 作为 PicOrderCnt
    picture->m_picture_frame.FrameNum = avFrame->pts;    // 短期参考帧

    // 设置当前帧指针
    picture->m_current_picture_ptr = &picture->m_picture_frame;

    // 设置解码完成标志
    picture->m_is_decode_finished = 1;
}
void CH264VideoDecoder_ffmpeg::close()
{
    if (m_sws_ctx) {
        sws_freeContext(m_sws_ctx);
        m_sws_ctx = nullptr;
    }
    if (m_codec_ctx) {
        avcodec_free_context(&m_codec_ctx);
        m_codec_ctx = nullptr;
    }
    if (m_format_ctx) {
        avformat_close_input(&m_format_ctx);
        m_format_ctx = nullptr;
    }
}

int CH264VideoDecoder_ffmpeg::do_callback(CH264Picture* picture_current, CH264PicturesGOP* pictures_gop, int32_t is_need_flush)
{
    int ret = 0;
    CH264Picture* outPicture = NULL;
    int errorCode = 0;

    if (is_need_flush) //说明当前已解码完毕的帧是IDR帧
    {
        while (1)
        {
            ret = pictures_gop->getOneOutPicture(NULL, outPicture); //flush操作
            RETURN_IF_FAILED(ret != 0, ret);

            if (outPicture != NULL)
            {
                if (m_output_frame_callback != NULL)
                {
                    ret = m_output_frame_callback(outPicture, m_userData, errorCode); //当找到可输出的帧时，主动通知外部用户
                    if (ret != 0)
                    {
                        return -1; //直接退出
                    }
                }

                outPicture->m_is_in_use = 0; //标记为闲置状态，以便后续回收重复利用
            }
            else //if (outPicture == NULL) //说明已经flush完毕，DPB缓存中已经没有可输出的帧了
            {
                break;
            }
        }
    }

    //-----------------------------------------------------------------------
    ret = pictures_gop->getOneOutPicture(picture_current, outPicture);
    RETURN_IF_FAILED(ret != 0, ret);

    if (outPicture != NULL)
    {
        if (m_output_frame_callback != NULL)
        {
            ret = m_output_frame_callback(outPicture, m_userData, errorCode); //当找到可输出的帧时，主动通知外部用户
            if (ret != 0)
            {
                return -1; //直接退出
            }
        }

        outPicture->m_is_in_use = 0; //标记为闲置状态，以便后续回收重复利用
    }

    return 0;
}

