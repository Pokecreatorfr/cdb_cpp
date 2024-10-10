#include <video_player.hpp>


VideoDecoder::VideoDecoder(const char* filepath, SDL_Renderer* renderer) 
    : formatContext(nullptr), codecContext(nullptr), videoStream(nullptr), codec(nullptr), 
      frame(nullptr), frameYUV(nullptr), swsContext(nullptr), texture(nullptr), 
      videoStreamIndex(-1), renderer(renderer), buffer(nullptr) {

    this->formatContext = avformat_alloc_context();
    // Ouvrir le fichier vidéo
    if (avformat_open_input(&formatContext, filepath, nullptr, nullptr) != 0) {
        std::cerr << "Could not open video file: " << filepath << std::endl;
        throw std::runtime_error("Failed to open video file.");
    }

    // Trouver les informations sur les flux
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "Could not find stream information" << std::endl;
        throw std::runtime_error("Failed to find stream information.");
    }

    // Trouver le flux vidéo
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            codec = avcodec_find_decoder(formatContext->streams[i]->codecpar->codec_id);
            codecContext = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(codecContext, formatContext->streams[i]->codecpar);
            avcodec_open2(codecContext, codec, nullptr);
            videoStream = formatContext->streams[i];
            break;
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "Could not find a video stream" << std::endl;
        throw std::runtime_error("No video stream found.");
    }

    // Préparer les frames et le contexte de conversion
    frame = av_frame_alloc();
    frameYUV = av_frame_alloc();
    buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height, 1));
    av_image_fill_arrays(frameYUV->data, frameYUV->linesize, buffer, AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height, 1);

    // Créer une texture SDL pour afficher les frames décodées
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING,
                                codecContext->width, codecContext->height);

    // Initialiser le contexte de conversion d'image
    swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                                codecContext->width, codecContext->height, AV_PIX_FMT_YUV420P,
                                SWS_BILINEAR, nullptr, nullptr, nullptr);

}

VideoDecoder::~VideoDecoder() {
    av_free(buffer);
    av_frame_free(&frame);
    av_frame_free(&frameYUV);
    avformat_close_input(&formatContext);
    SDL_DestroyTexture(texture);
    sws_freeContext(swsContext);
}

SDL_Texture* VideoDecoder::getTexture() {
    return texture;
}

bool VideoDecoder::decodeFrame() {
    AVPacket packet;
    int response;

    // Lire un paquet (frame) du fichier vidéo
    while (av_read_frame(formatContext, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            response = avcodec_send_packet(codecContext, &packet);
            if (response >= 0) {
                response = avcodec_receive_frame(codecContext, frame);
                if (response == 0) {
                    // Convertir l'image en YUV420P pour SDL
                    sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height,
                              frameYUV->data, frameYUV->linesize);

                    // Mettre à jour la texture SDL avec les données décodées
                    SDL_UpdateYUVTexture(texture, nullptr,
                                         frameYUV->data[0], frameYUV->linesize[0],
                                         frameYUV->data[1], frameYUV->linesize[1],
                                         frameYUV->data[2], frameYUV->linesize[2]);

                    av_packet_unref(&packet);
                    return true;
                }
            }
        }
        av_packet_unref(&packet);
    }

    return false;  // Retourner false si aucune frame n'est disponible
}

AVRational VideoDecoder::getFrameRate() {
    return videoStream->avg_frame_rate;
}

int64_t VideoDecoder::getFrameTimestamp()
{
    return frame->pts;
}

int64_t VideoDecoder::secondToTimestamp(float second)
{
    return second * videoStream->time_base.den / videoStream->time_base.num;
}

void VideoDecoder::seekToTimestamp(int64_t timestamp)
{
    av_seek_frame(formatContext, videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(codecContext);
}
