extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <iostream>

class VideoDecoder {
public:
    VideoDecoder(const char* filepath, SDL_Renderer* renderer);
    ~VideoDecoder();

    SDL_Texture* getTexture();
    bool decodeFrame();
    AVRational getFrameRate();

    // Get the timestamp of the current frame
    int64_t getFrameTimestamp();

    int64_t secondToTimestamp(float second);

    void seekToTimestamp(int64_t timestamp);


private:
    // Vidéo
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    AVStream* videoStream;
    const AVCodec* codec;
    AVFrame* frame;
    AVFrame* frameYUV;
    SwsContext* swsContext;
    SDL_Texture* texture;
    int videoStreamIndex;
    SDL_Renderer* renderer;
    uint8_t* buffer;
};