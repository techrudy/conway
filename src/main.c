#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

const int CELL_SIZE = 10;
const int WIDTH = 80;
const int HEIGHT = 60;

int getAliveNeighboors(bool grid[WIDTH][HEIGHT], int x, int y)
{
    int xOff;
    int yOff;

    int aliveCount = 0;

    for (xOff = -1; xOff <= 1; xOff++)
        for (yOff = -1; yOff <= 1; yOff++)
        {
            int xI = (x + xOff) % WIDTH;
            int yI = (y + yOff) % HEIGHT;

            if (xI < 0) xI = WIDTH + xI;
            if (yI < 0) yI = HEIGHT + yI;

            if (xOff == 0 && yOff == 0)
                continue;

            aliveCount += grid[xI][yI];
        }

    return aliveCount;
}

void clearGrid(bool grid[WIDTH][HEIGHT])
{
    for (int x = 0; x < WIDTH; x++)
        for (int y = 0; y < HEIGHT; y++)
            grid[x][y] = false;
}

void setCellAtMouse(bool grid[WIDTH][HEIGHT], bool value)
{
    int mouseX = 0;
    int mouseY = 0;

    SDL_GetMouseState(&mouseX, &mouseY);

    int xIndex = mouseX / CELL_SIZE;
    int yIndex = mouseY / CELL_SIZE;

    // Get a pointer to the cell's value
    bool *cell = &grid[xIndex][yIndex];
    // Set the value he's pointing at
    *cell = value;
}

void copyGrid(bool from[WIDTH][HEIGHT], bool to[WIDTH][HEIGHT])
{
    for (int x = 0; x < WIDTH; x++)
        for (int y = 0; y < HEIGHT; y++)
            to[x][y] = from[x][y];
}

int main( int argc, char ** argv )
{
    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;

    bool cells[WIDTH][HEIGHT];
    bool tmpCells[WIDTH][HEIGHT];
    bool checkpoint[WIDTH][HEIGHT];

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Impossible d'initialiser la SDL\n");
        return -1;
    }

    window = SDL_CreateWindow(
        "Game Of Life - Drawing...",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH * CELL_SIZE,
        HEIGHT * CELL_SIZE,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL)
    {
        printf("Could not create window: %s\n", SDL_GetError());
        return -1;
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (window == NULL)
    {
        printf("Could not create renderer: %s\n", SDL_GetError());
        return -1;
    }

    // Initialize cells values (false)
    clearGrid(cells);

    SDL_Rect rect;
    rect.h = CELL_SIZE;
    rect.w = CELL_SIZE;
    rect.x = 0;
    rect.y = 0;

    bool running = true;
    bool playing = false;
    bool drawing = false;
    bool shouldRemove = false;

    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;

    double deltaTime = 0;
    double timer = 0;

    while (running)
    {
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();

        deltaTime = (double)((NOW - LAST) / (double)SDL_GetPerformanceFrequency());

        if (SDL_PollEvent(&event))
            switch (event.type)
            {
            case SDL_QUIT: running = false; break;
            case SDL_MOUSEBUTTONUP: drawing = false; break;
            case SDL_MOUSEBUTTONDOWN:
                drawing = true;
                setCellAtMouse(cells, !shouldRemove);
                break;
            case SDL_MOUSEMOTION:
                if (!drawing || playing) break;
                setCellAtMouse(cells, !shouldRemove);
                break;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_LALT)
                    shouldRemove = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_SPACE:
                    playing = !playing;
                    copyGrid(cells, checkpoint);
                    break;
                case SDLK_LALT: shouldRemove = true; break;
                case SDLK_r:
                    clearGrid(cells);
                    playing = false;
                    break;
                case SDLK_p:
                    copyGrid(checkpoint, cells);
                    playing = false;
                    break;
                
                default: break;
                }
                SDL_SetWindowTitle(window, playing
                    ? "Game Of Life - Playing!"
                    : "Game Of Life - Drawing."
                );
                break;
            default: break;
            }

        timer += deltaTime;
        // Logic
        if (playing && timer >= .05)
        {
            timer = 0;
            copyGrid(cells, tmpCells);
            
            for (int x = 0; x < WIDTH; x++)
                for (int y = 0; y < HEIGHT; y++)
                {
                    int aliveCount = getAliveNeighboors(tmpCells, x, y);
                    bool *cell = &cells[x][y];

                    if (aliveCount == 3)
                        *cell = true;
                    else if (aliveCount < 2 || aliveCount > 3)
                        *cell = false;
                }
        }

        // Display
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        for (int x = 0; x < WIDTH; x++)
            for (int y = 0; y < HEIGHT; y++)
            {
                int color = cells[x][y] ? 0 : 255;

                rect.x = x * rect.w;
                rect.y = y * rect.h;

                SDL_SetRenderDrawColor(renderer, color, color, color, 0);
                SDL_RenderFillRect(renderer, &rect);
            }
        
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}