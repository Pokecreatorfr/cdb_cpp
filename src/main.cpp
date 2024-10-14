#include <video_player.hpp>
#include <time.h>

int main(int argc, char* argv[]) {
    // Vérifiez que le fichier vidéo est fourni
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <video file>" << std::endl;
        return -1;
    }

    const char* filepath = argv[1];


    // Initialisation de SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Could not initialize SDL - " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Exemple Video Player Software CPP",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "Could not create SDL window - " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);

    const char* driver = info.name;

    std::cout << "Using driver: " << driver << std::endl;


    VideoDecoder videoDecoder(filepath, renderer);

    SDL_Texture* texture = videoDecoder.getTexture();

    AVRational frameRate = videoDecoder.getFrameRate();

    int textureWidth, textureHeight;
    SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);

    SDL_Texture* whiteTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 2, 2);
    SDL_SetRenderTarget(renderer, whiteTexture);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


    // create timer
    clock_t timer;
    timer = clock();
    int frameCount = 0;

    // Boucle de décodage et d'affichage
    while (true) {

        clock_t currentTime = clock();
        float elapsedTime = float(currentTime - timer) / CLOCKS_PER_SEC;
        if (elapsedTime > 1.0f) {
            float fps = frameCount / elapsedTime;
            std::cout << "FPS: " << fps << std::endl;

            // Réinitialisation du compteur
            frameCount = 0;
            timer = currentTime;
        }
        videoDecoder.decodeFrame();



        int screenWidth, screenHeight;
        SDL_GetWindowSize(window, &screenWidth, &screenHeight);

        SDL_Rect drawRect = { 
            (screenWidth/ 2 - screenWidth/ 4),
            0 , 
            screenWidth / 2, 
            screenHeight / 2};

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, &drawRect);

        SDL_Rect whiteRect = {0 , screenHeight / 2, screenWidth, screenHeight/2};
        SDL_RenderCopy(renderer, whiteTexture, nullptr, &whiteRect);


        SDL_RenderPresent(renderer);
        frameCount++;

        SDL_Event event;
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }
        if(event.type == SDL_KEYDOWN){
            if(event.key.keysym.sym == SDLK_LEFT){
                videoDecoder.seekToTimestamp(videoDecoder.getFrameTimestamp() - videoDecoder.secondToTimestamp(5));
            }
            if(event.key.keysym.sym == SDLK_RIGHT){
                videoDecoder.seekToTimestamp(videoDecoder.getFrameTimestamp() + videoDecoder.secondToTimestamp(5));
            }
        }
        SDL_Delay(1000 / frameRate.num);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
