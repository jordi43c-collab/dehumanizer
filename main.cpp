#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>
#include <stdlib.h>

// Custom Textured 3D Quad rendering helpers for pixel-perfect alignment
void DrawFloorTile(Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint) {
    float x = position.x;
    float y = position.y;
    float z = position.z;
    float hw = size.x / 2.0f;
    float hd = size.y / 2.0f;
    
    float u0 = source.x / (float)texture.width;
    float v0 = source.y / (float)texture.height;
    float u1 = (source.x + source.width) / (float)texture.width;
    float v1 = (source.y + source.height) / (float)texture.height;
    
    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(u0, v0); rlVertex3f(x - hw, y, z - hd);
        rlTexCoord2f(u0, v1); rlVertex3f(x - hw, y, z + hd);
        rlTexCoord2f(u1, v1); rlVertex3f(x + hw, y, z + hd);
        rlTexCoord2f(u1, v0); rlVertex3f(x + hw, y, z - hd);
    rlEnd();
    rlSetTexture(0);
}

void DrawWallBlock(Texture2D texture, Rectangle source, Vector3 position, Vector3 size, Color tint) {
    float x = position.x;
    float y = position.y;
    float z = position.z;
    float hw = size.x / 2.0f;
    float hh = size.y / 2.0f;
    float hd = size.z / 2.0f;
    
    float u0 = source.x / (float)texture.width;
    float v0 = source.y / (float)texture.height;
    float u1 = (source.x + source.width) / (float)texture.width;
    float v1 = (source.y + source.height) / (float)texture.height;
    
    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        
        // Front Face (Facing South: +z)
        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(u0, v0); rlVertex3f(x - hw, y - hh, z + hd);
        rlTexCoord2f(u1, v0); rlVertex3f(x + hw, y - hh, z + hd);
        rlTexCoord2f(u1, v1); rlVertex3f(x + hw, y + hh, z + hd);
        rlTexCoord2f(u0, v1); rlVertex3f(x - hw, y + hh, z + hd);
        
        // Back Face (Facing North: -z)
        rlNormal3f(0.0f, 0.0f, -1.0f);
        rlTexCoord2f(u1, v0); rlVertex3f(x - hw, y - hh, z - hd);
        rlTexCoord2f(u1, v1); rlVertex3f(x - hw, y + hh, z - hd);
        rlTexCoord2f(u0, v1); rlVertex3f(x + hw, y + hh, z - hd);
        rlTexCoord2f(u0, v0); rlVertex3f(x + hw, y - hh, z - hd);
        
        // Left Face (Facing West: -x)
        rlNormal3f(-1.0f, 0.0f, 0.0f);
        rlTexCoord2f(u0, v0); rlVertex3f(x - hw, y - hh, z - hd);
        rlTexCoord2f(u1, v0); rlVertex3f(x - hw, y - hh, z + hd);
        rlTexCoord2f(u1, v1); rlVertex3f(x - hw, y + hh, z + hd);
        rlTexCoord2f(u0, v1); rlVertex3f(x - hw, y + hh, z - hd);
        
        // Right Face (Facing East: +x)
        rlNormal3f(1.0f, 0.0f, 0.0f);
        rlTexCoord2f(u1, v0); rlVertex3f(x + hw, y - hh, z - hd);
        rlTexCoord2f(u1, v1); rlVertex3f(x + hw, y + hh, z - hd);
        rlTexCoord2f(u0, v1); rlVertex3f(x + hw, y + hh, z + hd);
        rlTexCoord2f(u0, v0); rlVertex3f(x + hw, y - hh, z + hd);
        
        // Top Face (Facing Up: +y)
        rlNormal3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(u0, v0); rlVertex3f(x - hw, y + hh, z - hd);
        rlTexCoord2f(u0, v1); rlVertex3f(x - hw, y + hh, z + hd);
        rlTexCoord2f(u1, v1); rlVertex3f(x + hw, y + hh, z + hd);
        rlTexCoord2f(u1, v0); rlVertex3f(x + hw, y + hh, z - hd);
        
        // Bottom Face (Facing Down: -y)
        rlNormal3f(0.0f, -1.0f, 0.0f);
        rlTexCoord2f(u1, v0); rlVertex3f(x - hw, y - hh, z - hd);
        rlTexCoord2f(u0, v0); rlVertex3f(x + hw, y - hh, z - hd);
        rlTexCoord2f(u0, v1); rlVertex3f(x + hw, y - hh, z + hd);
        rlTexCoord2f(u1, v1); rlVertex3f(x - hw, y - hh, z + hd);
    rlEnd();
    rlSetTexture(0);
}

// Enums & Structs
Vector3 GetMouseGroundIntersection(Camera3D camera);

const Color CYAN = (Color){ 0, 240, 240, 255 };
enum State { STATE_IDLE = 0, STATE_RUN, STATE_ATTACK, STATE_HURT, STATE_DEAD };
enum GameScreen { SCREEN_TITLE, SCREEN_GAMEPLAY, SCREEN_GAMEOVER, SCREEN_VICTORY, SCREEN_ROOM_TRANSITION };
enum Difficulty { DIFF_EASY, DIFF_NORMAL, DIFF_HARD };

enum HeadState {
    HEAD_LOOK_DOWN = 0,
    HEAD_LOOK_UP = 1,
    HEAD_LOOK_LEFT = 2,
    HEAD_SHOOT_DOWN = 3,
    HEAD_SHOOT_UP = 4,
    HEAD_SHOOT_LEFT = 5
};

enum BodyState {
    BODY_WALK_DOWN = 0,
    BODY_WALK_UP = 1
};

enum EnemyType {
    ENEMY_KAMIKAZE = 0, // Fast melee chaser, explodes
    ENEMY_SENTRY = 1,   // Static ranged laser spore spitter
    ENEMY_SPREADER = 2  // Heavy AoE purple gas cloud dispenser
};

enum ItemType {
    ITEM_HEAL = 0,
    ITEM_CYBER_EYE = 1,
    ITEM_BOOTS = 2,
    ITEM_ACID = 3
};

struct Entity {
    Vector3 position;
    float radius;
    float health;
    float maxHealth;
    float speed;
    State state;
    float stateTimer;
    Vector3 direction;
    Color color;
    
    // Animation details
    float animTimer;
    int animFrame;
    bool hasFiredAttack;
    bool isBoss;
    
    // Spaceship upgrades / Enemy type variables
    EnemyType enemyType;
    float gasTimer; // AoE Gas trail timer
};

struct Projectile {
    Vector3 position;
    Vector3 direction;
    float speed;
    float radius;
    bool active;
    bool isEnemy;
    bool isAcid; // Acid glands poison bubble
};

struct Particle {
    Vector3 position;
    Vector3 velocity;
    Color color;
    float life;
    float maxLife;
    bool active;
    bool isGas; // Toxic gas cloud particle
};

struct ImpactEffect {
    Vector3 position;
    int frame;
    float timer;
    bool active;
};

struct GroundItem {
    Vector3 position;
    ItemType type;
    bool active;
    float animTimer;
};

struct Star {
    Vector3 position;
    float size;
    Color color;
    float parallaxFactor;
};

// --- DUNGEON & ROOMS ---
enum RoomType { ROOM_START, ROOM_ENEMY, ROOM_TREASURE, ROOM_BOSS };

#define ROOM_GRID_SIZE 21
#define DUNGEON_SIZE 5
#define MAX_ROOM_ENEMIES 8
#define MAX_ROOM_PILLARS 6
#define MAX_GROUND_ITEMS 8
#define MAX_STARS 150
#define MAX_ROOM_DECORATIONS 50

struct Decoration {
    Vector3 position;
    int type;          // 0: Moss1, 1: Moss2, 2: Grass, 3: CyanFlower, 4: GreenFlower, 5: RedMushroom, 6: PurpleMushroom, 7: Stalagmite
    int animFrame;
    float animTimer;
    Vector3 velocity;
    float wanderTimer;
    float speed;
    bool active;
    bool isInsect;     // true for beetle/firefly
    bool isFly;        // true for firefly, false for beetle
    float fleeTimer;
    float alpha;
    float bobOffset;
    float floatHeight;
};

struct Room {
    bool active;
    RoomType type;
    bool cleared;
    bool doors[4];
    int gridX;
    int gridY;
    
    int floorTileVariants[ROOM_GRID_SIZE][ROOM_GRID_SIZE];
    int wallTileVariants[ROOM_GRID_SIZE][ROOM_GRID_SIZE];
    
    int numPillars;
    Vector3 pillars[MAX_ROOM_PILLARS];
    
    int numEnemies;
    Entity enemies[MAX_ROOM_ENEMIES];
    
    int numItems;
    GroundItem items[MAX_GROUND_ITEMS];
    
    int numDecorations;
    Decoration decorations[MAX_ROOM_DECORATIONS];
};

// Global Constants
#define MAX_PROJECTILES 128
#define MAX_PARTICLES 800
#define MAX_IMPACTS 64
const float ROOM_BOUNDS = 9.5f;

// Globals
Particle particles[MAX_PARTICLES] = { 0 };
ImpactEffect impacts[MAX_IMPACTS] = { 0 };
Room dungeon[DUNGEON_SIZE][DUNGEON_SIZE] = { 0 };
Star spaceStars[MAX_STARS] = { 0 };

int currentRoomX = 2;
int currentRoomY = 2;
Difficulty selectedDifficulty = DIFF_NORMAL;

// Upgrade states
bool hasCyberEye = false;
bool hasThrusterBoots = false;
bool hasAcidGlands = false;

// Heart health definitions
int playerHearts = 3;
int playerMaxHearts = 3;
int playerHalfHeartsHealth = 6; // each full heart represents 2 half-hearts

// Transition states
int nextRoomX = 2;
int nextRoomY = 2;
float transitionTimer = 0.0f;
Vector3 transitionPlayerStart;
Vector3 transitionPlayerEnd;

void SpawnParticles(Vector3 pos, Color color, int count, bool isGas = false) {
    for (int k = 0; k < count; k++) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) {
                particles[i].position = pos;
                if (isGas) {
                    // Gas spreads flat on the ground
                    particles[i].velocity = (Vector3){
                        (float)GetRandomValue(-100, 100) * 0.015f,
                        0.0f,
                        (float)GetRandomValue(-100, 100) * 0.015f
                    };
                    particles[i].life = (float)GetRandomValue(350, 600) * 0.01f; // lasts 3.5 - 6 seconds!
                } else {
                    particles[i].velocity = (Vector3){
                        (float)GetRandomValue(-100, 100) * 0.04f,
                        (float)GetRandomValue(40, 120) * 0.04f,
                        (float)GetRandomValue(-100, 100) * 0.04f
                    };
                    particles[i].life = (float)GetRandomValue(25, 60) * 0.01f;
                }
                particles[i].color = color;
                particles[i].maxLife = particles[i].life;
                particles[i].isGas = isGas;
                particles[i].active = true;
                break;
            }
        }
    }
}

void SpawnImpact(Vector3 pos) {
    for (int i = 0; i < MAX_IMPACTS; i++) {
        if (!impacts[i].active) {
            impacts[i].position = pos;
            impacts[i].frame = 0;
            impacts[i].timer = 0.07f;
            impacts[i].active = true;
            break;
        }
    }
}

// Procedural Spaceship Parallax Stars
void InitStarfield() {
    for (int i = 0; i < MAX_STARS; i++) {
        spaceStars[i].position = (Vector3){
            (float)GetRandomValue(-120, 120),
            (float)GetRandomValue(-40, -10), // drawn deep below or high above far away
            (float)GetRandomValue(-120, 120)
        };
        spaceStars[i].size = (float)GetRandomValue(10, 40) * 0.01f;
        spaceStars[i].parallaxFactor = (float)GetRandomValue(40, 85) * 0.01f;
        
        int rVal = GetRandomValue(0, 3);
        if (rVal == 0) spaceStars[i].color = Fade(CYAN, (float)GetRandomValue(40, 90) * 0.01f);
        else if (rVal == 1) spaceStars[i].color = Fade(SKYBLUE, (float)GetRandomValue(40, 90) * 0.01f);
        else spaceStars[i].color = Fade(RAYWHITE, (float)GetRandomValue(40, 95) * 0.01f);
    }
}

// Chroma Key helper to clean background solid colors (e.g. pure black or magenta) and make them fully transparent
void CleanImageBackground(Image *image, Color backgroundColor) {
    if (image->data == NULL) return;
    // Ensure the image format is uncompressed RGBA for direct Color casting
    ImageFormat(image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    
    Color *pixels = (Color*)image->data;
    int numPixels = image->width * image->height;
    for (int i = 0; i < numPixels; i++) {
        // Tolerance match for pure black or very dark colors (almost black, e.g. R, G, B all less than 15)
        bool isBlackKey = (pixels[i].r < 15 && pixels[i].g < 15 && pixels[i].b < 15);
        // Also support classic magenta chroma key
        bool isMagentaKey = (pixels[i].r > 240 && pixels[i].g < 15 && pixels[i].b > 240);
        
        if (isBlackKey || isMagentaKey) {
            // Convert to fully transparent black
            pixels[i].a = 0;
            pixels[i].r = 0;
            pixels[i].g = 0;
            pixels[i].b = 0;
        }
    }
}

// Resolves a resource file path by checking both the local folder and the parent folder (e.g. running from build/)
const char* FindResourcePath(const char *fileName) {
    // Try parent directory first (higher priority when running from build/ subfolder)
    const char *parentPath = TextFormat("../%s", fileName);
    if (FileExists(parentPath)) {
        return parentPath;
    }
    if (FileExists(fileName)) {
        return fileName;
    }
    return NULL;
}

// Loads a texture from file, applying real-time Chroma Keying to remove backgrounds
Texture2D LoadTextureWithChromaKey(const char *fileName, Color keyColor) {
    const char *resolvedPath = FindResourcePath(fileName);
    if (resolvedPath == NULL) return (Texture2D){ 0 };
    
    Image img = LoadImage(resolvedPath);
    if (img.data != NULL) {
        CleanImageBackground(&img, keyColor);
        Texture2D tex = LoadTextureFromImage(img);
        UnloadImage(img);
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);
        return tex;
    }
    return (Texture2D){ 0 };
}

// Procedural Character Spritesheet with Upgrades and Enemy Types (Width=256, Height=256)
Texture2D GenerateProceduralSpritesheet() {
    Image img = GenImageColor(256, 256, BLANK);
    
    // --- ROW 0: Player Heads ---
    ImageDrawCircle(&img, 16, 16, 12, LIME);
    ImageDrawCircle(&img, 10, 14, 3, RAYWHITE);
    ImageDrawCircle(&img, 10, 14, 1, BLACK);
    ImageDrawCircle(&img, 22, 14, 3, RAYWHITE);
    ImageDrawCircle(&img, 22, 14, 1, BLACK);
    ImageDrawRectangle(&img, 14, 21, 4, 2, BLACK);
    
    ImageDrawCircle(&img, 32 + 16, 16, 12, LIME);
    ImageDrawCircle(&img, 32 + 11, 10, 2, RAYWHITE);
    ImageDrawCircle(&img, 32 + 21, 10, 2, RAYWHITE);
    
    ImageDrawCircle(&img, 64 + 16, 16, 12, LIME);
    ImageDrawCircle(&img, 64 + 9, 14, 3, RAYWHITE);
    ImageDrawCircle(&img, 64 + 9, 14, 1, BLACK);
    ImageDrawRectangle(&img, 64 + 5, 21, 3, 2, BLACK);
    
    ImageDrawCircle(&img, 96 + 16, 16, 12, LIME);
    ImageDrawCircle(&img, 96 + 10, 14, 3, RAYWHITE);
    ImageDrawCircle(&img, 96 + 10, 14, 1, BLACK);
    ImageDrawCircle(&img, 96 + 22, 14, 3, RAYWHITE);
    ImageDrawCircle(&img, 96 + 22, 14, 1, BLACK);
    ImageDrawCircle(&img, 96 + 16, 22, 4, BLACK);
    ImageDrawCircle(&img, 96 + 16, 22, 2, SKYBLUE);
    
    ImageDrawCircle(&img, 128 + 16, 16, 12, LIME);
    ImageDrawCircle(&img, 128 + 11, 11, 2, RAYWHITE);
    ImageDrawCircle(&img, 128 + 21, 11, 2, RAYWHITE);
    ImageDrawCircle(&img, 128 + 16, 6, 3, BLACK);
    
    ImageDrawCircle(&img, 160 + 16, 16, 12, LIME);
    ImageDrawCircle(&img, 160 + 9, 14, 3, RAYWHITE);
    ImageDrawCircle(&img, 160 + 9, 14, 1, BLACK);
    ImageDrawCircle(&img, 160 + 4, 18, 3, BLACK);
    ImageDrawCircle(&img, 160 + 4, 18, 1, SKYBLUE);
    
    // --- ROW 1: Player Legs Walk Down ---
    for (int f = 0; f < 4; f++) {
        int ox = f * 32;
        int oy = 32;
        ImageDrawRectangle(&img, ox + 11, oy + 12, 10, 10, GREEN);
        if (f == 0 || f == 2) {
            ImageDrawRectangle(&img, ox + 11, oy + 22, 3, 7, LIME);
            ImageDrawRectangle(&img, ox + 18, oy + 22, 3, 7, LIME);
        } else if (f == 1) {
            ImageDrawRectangle(&img, ox + 9, oy + 20, 4, 9, LIME);
            ImageDrawRectangle(&img, ox + 19, oy + 22, 3, 6, LIME);
        } else if (f == 3) {
            ImageDrawRectangle(&img, ox + 10, oy + 22, 3, 6, LIME);
            ImageDrawRectangle(&img, ox + 19, oy + 20, 4, 9, LIME);
        }
    }
    
    // --- ROW 2: Player Legs Walk Up ---
    for (int f = 0; f < 4; f++) {
        int ox = f * 32;
        int oy = 64;
        ImageDrawRectangle(&img, ox + 11, oy + 12, 10, 10, GREEN);
        if (f == 0 || f == 2) {
            ImageDrawRectangle(&img, ox + 11, oy + 22, 3, 7, LIME);
            ImageDrawRectangle(&img, ox + 18, oy + 22, 3, 7, LIME);
        } else if (f == 1) {
            ImageDrawRectangle(&img, ox + 10, oy + 21, 3, 8, LIME);
            ImageDrawRectangle(&img, ox + 19, oy + 22, 3, 7, LIME);
        } else if (f == 3) {
            ImageDrawRectangle(&img, ox + 10, oy + 22, 3, 7, LIME);
            ImageDrawRectangle(&img, ox + 19, oy + 21, 3, 8, LIME);
        }
    }
    
    // --- ROW 3: Kamikaze drone (Melee Alien) ---
    for (int f = 0; f < 4; f++) {
        int ox = f * 32;
        int oy = 96;
        ImageDrawCircle(&img, ox + 16, oy + 16, 9, ORANGE);
        ImageDrawRectangle(&img, ox + 6, oy + 14, 20, 4, DARKGRAY); // steel wings
        ImageDrawCircle(&img, ox + 16, oy + 16, 4, RED); // glowing core
        if (f == 1 || f == 3) {
            ImageDrawCircle(&img, ox + 16, oy + 16, 6, GOLD); // pulse core
        }
    }
    
    // --- ROW 4: Spore Sentry (Ranged Plant Sentry) ---
    for (int f = 0; f < 4; f++) {
        int ox = f * 32;
        int oy = 128;
        ImageDrawRectangle(&img, ox + 12, oy + 18, 8, 14, BROWN); // stalk
        ImageDrawCircle(&img, ox + 16, oy + 12, 9, PURPLE);       // spore bulb
        ImageDrawCircle(&img, ox + 16, oy + 12, 3, BLACK);        // eye spitter
        if (f == 1) {
            ImageDrawCircle(&img, ox + 16, oy + 12, 10, CYAN); // charging indicator
        } else if (f == 2) {
            ImageDrawCircle(&img, ox + 16, oy + 12, 11, LIME); // firing espora frame
        }
    }
    
    // --- ROW 5: Gas Spreader (Heavy AoE Alien) ---
    for (int f = 0; f < 4; f++) {
        int ox = f * 32;
        int oy = 160;
        ImageDrawCircle(&img, ox + 16, oy + 18, 12, (Color){ 70, 40, 110, 255 }); // dark purple slug
        ImageDrawRectangle(&img, ox + 4, oy + 22, 24, 10, (Color){ 70, 40, 110, 255 });
        ImageDrawCircle(&img, ox + 11, oy + 14, 2, LIME); // toxic eyes
        ImageDrawCircle(&img, ox + 21, oy + 14, 2, LIME);
        if (f == 1 || f == 3) { // steaming
            ImageDrawCircle(&img, ox + 16, oy + 8, 4, Fade(PURPLE, 0.6f));
        }
    }
    
    // --- ROW 6: Effects & Projectiles ---
    ImageDrawCircle(&img, 16, 192 + 16, 5, SKYBLUE); // Player tear
    ImageDrawCircle(&img, 14, 192 + 14, 1, WHITE);
    ImageDrawCircle(&img, 32 + 16, 192 + 16, 6, LIME); // Enemy espora
    ImageDrawCircle(&img, 32 + 16, 192 + 16, 2, WHITE);
    ImageDrawCircle(&img, 64 + 16, 192 + 16, 10, (Color){ 50, 205, 50, 255 }); // Acid green giant bubble
    ImageDrawCircle(&img, 64 + 12, 192 + 12, 3, LIME);
    ImageDrawCircle(&img, 96 + 16, 192 + 16, 6, SKYBLUE); // Splash 0
    ImageDrawCircle(&img, 96 + 16, 192 + 16, 4, BLANK);
    ImageDrawCircle(&img, 128 + 10, 192 + 10, 2, SKYBLUE); // Splash 1
    ImageDrawCircle(&img, 128 + 22, 192 + 22, 2, SKYBLUE);
    
    // --- ROW 7: Ground Items & Loot ---
    // Cell 0: Cyber Eye (glowing neon green cybernetic circle)
    ImageDrawCircle(&img, 16, 224 + 16, 8, DARKGRAY);
    ImageDrawCircle(&img, 16, 224 + 16, 4, GREEN);
    ImageDrawCircle(&img, 16, 224 + 16, 1, LIME);
    // Cell 1: Thruster Boots (cyan boots with small spark fire)
    ImageDrawRectangle(&img, 32 + 8, 224 + 14, 6, 12, CYAN);
    ImageDrawRectangle(&img, 32 + 18, 224 + 14, 6, 12, CYAN);
    ImageDrawRectangle(&img, 32 + 8, 224 + 26, 16, 3, GOLD); // thruster plate
    // Cell 2: Acid Glands (pulsating neon capsule)
    ImageDrawRectangle(&img, 64 + 11, 224 + 8, 10, 16, LIME);
    ImageDrawCircle(&img, 64 + 16, 224 + 16, 6, (Color){ 0, 180, 0, 255 });
    // Cell 3: Spatial Health Battery
    ImageDrawRectangle(&img, 96 + 12, 224 + 8, 8, 16, RED);
    ImageDrawRectangle(&img, 96 + 12, 224 + 8, 8, 6, RAYWHITE);
    ImageDrawRectangle(&img, 96 + 14, 224 + 18, 4, 4, RAYWHITE); // positive cross
    
    CleanImageBackground(&img, BLACK);
    ExportImage(img, "spritesheet.png");
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    return tex;
}

// Procedural Auto-Tiled Cave Environment Spritesheet (Width=256, Height=256)
Texture2D GenerateEnvironmentTileSheet() {
    Image img = GenImageColor(256, 256, BLANK);
    
    // Palettes
    Color rockBase = (Color){ 62, 54, 46, 255 };      // standard grayish brown rock
    Color rockDark = (Color){ 36, 30, 26, 255 };      // cracks and shadows
    Color rockLight = (Color){ 88, 78, 68, 255 };     // highlights
    
    Color crystalBase = (Color){ 45, 45, 60, 255 };   // mystical dark purple-gray rock
    Color crystalDark = (Color){ 22, 22, 34, 255 };
    Color crystalGlow = (Color){ 0, 210, 225, 255 };  // cyan bioluminescent crystals
    Color crystalVibe = (Color){ 160, 80, 240, 255 }; // purple vibe
    
    Color lavaBase = (Color){ 30, 25, 25, 255 };      // black/basalt volcanic rock
    Color lavaDark = (Color){ 12, 10, 10, 255 };
    Color lavaGlow = (Color){ 245, 95, 30, 255 };     // orange-red lava
    Color lavaCore = (Color){ 255, 210, 60, 255 };    // yellow magma core
    
    for (int ty = 0; ty < 8; ty++) {
        for (int tx = 0; tx < 8; tx++) {
            int ox = tx * 32;
            int oy = ty * 32;
            
            // --- 1. SECTOR 0: Standard Brown-Gray Cave (Cols 0-2) ---
            if (tx < 3) {
                if (ty < 2) { // Rocky Walls
                    ImageDrawRectangle(&img, ox, oy, 32, 32, rockBase);
                    // Draw escarpated stone crack patterns
                    for (int i = 0; i < 5; i++) {
                        int rx1 = GetRandomValue(0, 31);
                        int ry1 = GetRandomValue(0, 31);
                        int rx2 = GetRandomValue(0, 31);
                        int ry2 = GetRandomValue(0, 31);
                        ImageDrawLine(&img, ox + rx1, oy + ry1, ox + rx2, oy + ry2, rockDark);
                    }
                    // Highlight stone edges
                    for (int i = 0; i < 3; i++) {
                        int rx = GetRandomValue(2, 28);
                        int ry = GetRandomValue(2, 28);
                        ImageDrawRectangle(&img, ox + rx, oy + ry, GetRandomValue(3, 8), GetRandomValue(2, 4), rockLight);
                    }
                    // Auto-tiling borders (shadow at edges)
                    ImageDrawRectangle(&img, ox, oy, 32, 2, rockDark);
                    ImageDrawRectangle(&img, ox, oy, 2, 32, rockDark);
                    ImageDrawRectangle(&img, ox + 30, oy, 2, 32, rockDark);
                    ImageDrawRectangle(&img, ox, oy + 30, 32, 2, rockDark);
                }
                else if (ty < 4) { // Natural Stone / Earth transit floors
                    ImageDrawRectangle(&img, ox, oy, 32, 32, (Color){ 54, 45, 38, 255 }); // earth base
                    // Draw irregular flagstone textures
                    for (int f = 0; f < 3; f++) {
                        int px = GetRandomValue(2, 20);
                        int py = GetRandomValue(2, 20);
                        int w = GetRandomValue(6, 12);
                        int h = GetRandomValue(6, 12);
                        ImageDrawRectangle(&img, ox + px, oy + py, w, h, (Color){ 68, 58, 48, 255 });
                        ImageDrawRectangleLines(&img, (Rectangle){ (float)(ox + px), (float)(oy + py), (float)w, (float)h }, 1, rockDark);
                    }
                    // Faint gravel dots
                    for (int d = 0; d < 10; d++) {
                        ImageDrawPixel(&img, ox + GetRandomValue(1, 30), oy + GetRandomValue(1, 30), rockLight);
                    }
                }
                else { // Background deep cave background
                    ImageDrawRectangle(&img, ox, oy, 32, 32, (Color){ 16, 14, 12, 255 });
                    // Faint stony cracks in dark void
                    for (int c = 0; c < 2; c++) {
                        ImageDrawLine(&img, ox + GetRandomValue(0, 31), oy, ox + GetRandomValue(0, 31), oy + 31, (Color){ 24, 20, 18, 255 });
                    }
                }
            }
            // --- 2. SECTOR 1: Bioluminescent Crystal Cave (Cols 3-5) ---
            else if (tx < 6) {
                if (ty < 2) { // Walls with glowing crystals
                    ImageDrawRectangle(&img, ox, oy, 32, 32, crystalBase);
                    // Draw dark rocky crack lines
                    for (int i = 0; i < 4; i++) {
                        ImageDrawLine(&img, ox + GetRandomValue(0, 31), oy + GetRandomValue(0, 31), ox + GetRandomValue(0, 31), oy + GetRandomValue(0, 31), crystalDark);
                    }
                    // Draw bioluminescent crystals (cyan spots)
                    for (int c = 0; c < 3; c++) {
                        int cx = GetRandomValue(4, 26);
                        int cy = GetRandomValue(4, 26);
                        ImageDrawCircle(&img, ox + cx, oy + cy, GetRandomValue(2, 4), crystalGlow);
                        ImageDrawCircle(&img, ox + cx, oy + cy, 1, WHITE); // glowing tip
                    }
                    // Auto-tiling borders
                    ImageDrawRectangle(&img, ox, oy, 32, 2, crystalDark);
                    ImageDrawRectangle(&img, ox, oy, 2, 32, crystalDark);
                    ImageDrawRectangle(&img, ox + 30, oy, 2, 32, crystalDark);
                    ImageDrawRectangle(&img, ox, oy + 30, 32, 2, crystalDark);
                }
                else if (ty < 4) { // Floor
                    ImageDrawRectangle(&img, ox, oy, 32, 32, (Color){ 30, 30, 42, 255 }); // dark floor
                    // Draw flat moss stones
                    for (int f = 0; f < 3; f++) {
                        int px = GetRandomValue(2, 20);
                        int py = GetRandomValue(2, 20);
                        int w = GetRandomValue(6, 12);
                        int h = GetRandomValue(6, 12);
                        ImageDrawRectangle(&img, ox + px, oy + py, w, h, (Color){ 45, 45, 60, 255 });
                        // Moss fringe on floor flagstones
                        ImageDrawRectangle(&img, ox + px, oy + py, w, 2, (Color){ 10, 110, 80, 255 });
                    }
                    // Glowing spores
                    for (int sp = 0; sp < 4; sp++) {
                        ImageDrawPixel(&img, ox + GetRandomValue(2, 29), oy + GetRandomValue(2, 29), crystalGlow);
                    }
                }
                else { // Background deep dark
                    ImageDrawRectangle(&img, ox, oy, 32, 32, (Color){ 12, 12, 20, 255 });
                    // Crystal cluster in background
                    ImageDrawCircle(&img, ox + 16, oy + 16, 2, crystalVibe);
                    ImageDrawPixel(&img, ox + 16, oy + 16, WHITE);
                }
            }
            // --- 3. SECTOR 2: Volcanic Lava Cave (Cols 6-7) ---
            else {
                if (ty < 2) { // Basalt walls with glowing lava veins
                    ImageDrawRectangle(&img, ox, oy, 32, 32, lavaBase);
                    // Draw magma cracks
                    ImageDrawLine(&img, ox + 6, oy, ox + 12, oy + 31, lavaGlow);
                    ImageDrawLine(&img, ox + 22, oy, ox + 18, oy + 31, lavaGlow);
                    ImageDrawLine(&img, ox + 2, oy + 16, ox + 28, oy + 18, lavaGlow);
                    // Magma core lines inside cracks
                    ImageDrawLine(&img, ox + 6, oy, ox + 12, oy + 31, lavaCore);
                    
                    // Dark cracks
                    for (int c = 0; c < 3; c++) {
                        ImageDrawLine(&img, ox + GetRandomValue(0, 31), oy + GetRandomValue(0, 31), ox + GetRandomValue(0, 31), oy + GetRandomValue(0, 31), lavaDark);
                    }
                    // Auto-tiling borders
                    ImageDrawRectangle(&img, ox, oy, 32, 2, lavaDark);
                    ImageDrawRectangle(&img, ox, oy, 2, 32, lavaDark);
                    ImageDrawRectangle(&img, ox + 30, oy, 2, 32, lavaDark);
                    ImageDrawRectangle(&img, ox, oy + 30, 32, 2, lavaDark);
                }
                else if (ty < 4) { // Floor
                    ImageDrawRectangle(&img, ox, oy, 32, 32, lavaDark);
                    // Lava pools / cracks in floor
                    ImageDrawRectangle(&img, ox + 4, oy + 12, 24, 8, lavaGlow);
                    ImageDrawRectangle(&img, ox + 8, oy + 14, 16, 4, lavaCore);
                    // Basalt cobblestone details
                    ImageDrawRectangleLines(&img, (Rectangle){ (float)ox, (float)oy, 32, 32 }, 1, (Color){ 40, 35, 35, 255 });
                }
                else { // Background deep dark
                    ImageDrawRectangle(&img, ox, oy, 32, 32, BLACK);
                    // Magma drip vertical line
                    ImageDrawRectangle(&img, ox + 15, oy, 2, 32, lavaGlow);
                }
            }
        }
    }
    
    // --- 4. CAVE FLORA DECORATIONS (Row 4: ty = 4) ---
    // Column 0: Moss Patch 1 (Dense forest green)
    int mox0 = 0 * 32, moy4 = 4 * 32;
    ImageDrawCircle(&img, mox0 + 16, moy4 + 16, 11, (Color){ 20, 80, 45, 255 });
    ImageDrawCircle(&img, mox0 + 11, moy4 + 14, 6, (Color){ 30, 110, 55, 255 });
    ImageDrawCircle(&img, mox0 + 20, moy4 + 18, 7, (Color){ 45, 140, 70, 255 });
    ImageDrawCircle(&img, mox0 + 16, moy4 + 16, 3, (Color){ 80, 185, 95, 255 }); // light highlight
    
    // Column 1: Moss Patch 2 (Vibrant golden/lime moss)
    int mox1 = 1 * 32;
    ImageDrawCircle(&img, mox1 + 16, moy4 + 16, 10, (Color){ 40, 70, 15, 255 });
    ImageDrawCircle(&img, mox1 + 12, moy4 + 17, 7, (Color){ 75, 115, 25, 255 });
    ImageDrawCircle(&img, mox1 + 20, moy4 + 13, 6, (Color){ 110, 155, 35, 255 });
    ImageDrawCircle(&img, mox1 + 16, moy4 + 16, 4, (Color){ 165, 215, 45, 255 });
    
    // Column 2: Small underground grass patches (lime grass blades)
    int mox2 = 2 * 32;
    // Draw individual grass blades
    ImageDrawLine(&img, mox2 + 10, moy4 + 28, mox2 + 7, moy4 + 10, LIME);
    ImageDrawLine(&img, mox2 + 10, moy4 + 28, mox2 + 12, moy4 + 8, GREEN);
    ImageDrawLine(&img, mox2 + 16, moy4 + 28, mox2 + 16, moy4 + 6, LIME);
    ImageDrawLine(&img, mox2 + 20, moy4 + 28, mox2 + 20, moy4 + 12, GREEN);
    ImageDrawLine(&img, mox2 + 20, moy4 + 28, mox2 + 25, moy4 + 9, LIME);
    
    // Column 3: Bioluminescent Cyan Flower
    int mox3 = 3 * 32;
    ImageDrawRectangle(&img, mox3 + 15, moy4 + 18, 2, 12, (Color){ 20, 80, 65, 255 }); // stem
    // Petals
    ImageDrawCircle(&img, mox3 + 10, moy4 + 14, 5, (Color){ 0, 140, 200, 255 });
    ImageDrawCircle(&img, mox3 + 22, moy4 + 14, 5, (Color){ 0, 140, 200, 255 });
    ImageDrawCircle(&img, mox3 + 16, moy4 + 9, 5, (Color){ 0, 160, 230, 255 });
    ImageDrawCircle(&img, mox3 + 16, moy4 + 19, 5, (Color){ 0, 120, 170, 255 });
    // Glowing center
    ImageDrawCircle(&img, mox3 + 16, moy4 + 14, 3, CYAN);
    ImageDrawCircle(&img, mox3 + 16, moy4 + 14, 1, WHITE);
    
    // Column 4: Bioluminescent Green Flower
    int mox4 = 4 * 32;
    ImageDrawRectangle(&img, mox4 + 15, moy4 + 18, 2, 12, (Color){ 10, 60, 15, 255 }); // stem
    // Petals
    ImageDrawCircle(&img, mox4 + 11, moy4 + 13, 5, (Color){ 0, 120, 40, 255 });
    ImageDrawCircle(&img, mox4 + 21, moy4 + 13, 5, (Color){ 0, 120, 40, 255 });
    ImageDrawCircle(&img, mox4 + 16, moy4 + 8, 5, (Color){ 10, 170, 60, 255 });
    ImageDrawCircle(&img, mox4 + 16, moy4 + 18, 5, (Color){ 5, 100, 30, 255 });
    // Glowing center
    ImageDrawCircle(&img, mox4 + 16, moy4 + 13, 3, LIME);
    ImageDrawCircle(&img, mox4 + 16, moy4 + 13, 1, WHITE);
    
    // Column 5: Red Mushroom (Setas rojas)
    int mox5 = 5 * 32;
    ImageDrawRectangle(&img, mox5 + 14, moy4 + 16, 4, 13, (Color){ 200, 190, 180, 255 }); // stalk
    ImageDrawCircle(&img, mox5 + 16, moy4 + 12, 9, RED); // cap
    ImageDrawRectangle(&img, mox5 + 7, moy4 + 12, 19, 3, RED); // base of cap
    // White spots
    ImageDrawPixel(&img, mox5 + 12, moy4 + 8, WHITE);
    ImageDrawPixel(&img, mox5 + 20, moy4 + 10, WHITE);
    ImageDrawPixel(&img, mox5 + 16, moy4 + 12, WHITE);
    ImageDrawPixel(&img, mox5 + 15, moy4 + 7, WHITE);
    
    // Column 6: Purple Bioluminescent Mushroom (Seta bioluminiscente)
    int mox6 = 6 * 32;
    ImageDrawRectangle(&img, mox6 + 14, moy4 + 16, 4, 13, (Color){ 120, 110, 160, 255 }); // stalk
    ImageDrawCircle(&img, mox6 + 16, moy4 + 11, 8, VIOLET); // cap
    ImageDrawRectangle(&img, mox6 + 8, moy4 + 11, 17, 3, VIOLET);
    // Cyan bioluminescent spots
    ImageDrawPixel(&img, mox6 + 12, moy4 + 7, CYAN);
    ImageDrawPixel(&img, mox6 + 19, moy4 + 9, CYAN);
    ImageDrawPixel(&img, mox6 + 15, moy4 + 10, CYAN);
    
    // Column 7: Small Stalagmite / Stone Pile
    int mox7 = 7 * 32;
    // Stalagmite spike shape (using stacked horizontal lines of decreasing width to draw a gorgeous pixel stalagmite!)
    for (int y = 4; y < 30; y++) {
        int width = (30 - y) * 20 / 26;
        int lx = 16 - width / 2;
        int rx = 16 + width / 2;
        for (int px = lx; px <= rx; px++) {
            Color c = (px == lx || px == rx) ? rockDark : (px == lx + 1 || px == lx + 2) ? rockLight : rockBase;
            ImageDrawPixel(&img, mox7 + px, moy4 + y, c);
        }
    }
    
    // --- 5. CAVE FAUNA ANIMATED BEETLES (Row 5: ty = 5) ---
    int moy5 = 5 * 32;
    // Frame 0: Beetle legs out
    int bx0 = 0 * 32;
    ImageDrawRectangle(&img, bx0 + 13, moy5 + 10, 6, 12, (Color){ 45, 32, 22, 255 }); // body shell
    ImageDrawCircle(&img, bx0 + 16, moy5 + 8, 4, (Color){ 30, 20, 15, 255 }); // head
    // Legs frame 0
    ImageDrawLine(&img, bx0 + 10, moy5 + 11, bx0 + 22, moy5 + 11, BLACK); // front legs
    ImageDrawLine(&img, bx0 + 9, moy5 + 15, bx0 + 23, moy5 + 15, BLACK);  // mid legs
    ImageDrawLine(&img, bx0 + 10, moy5 + 19, bx0 + 22, moy5 + 19, BLACK); // back legs
    // Antennas
    ImageDrawLine(&img, bx0 + 14, moy5 + 5, bx0 + 16, moy5 + 8, ORANGE);
    ImageDrawLine(&img, bx0 + 18, moy5 + 5, bx0 + 16, moy5 + 8, ORANGE);
    
    // Frame 1: Beetle legs in/bent
    int bx1 = 1 * 32;
    ImageDrawRectangle(&img, bx1 + 13, moy5 + 10, 6, 12, (Color){ 45, 32, 22, 255 }); // body
    ImageDrawCircle(&img, bx1 + 16, moy5 + 8, 4, (Color){ 30, 20, 15, 255 });
    // Legs frame 1 (bent)
    ImageDrawLine(&img, bx1 + 11, moy5 + 12, bx1 + 21, moy5 + 12, BLACK);
    ImageDrawLine(&img, bx1 + 12, moy5 + 16, bx1 + 20, moy5 + 16, BLACK);
    ImageDrawLine(&img, bx1 + 11, moy5 + 18, bx1 + 21, moy5 + 18, BLACK);
    // Antennas
    ImageDrawLine(&img, bx1 + 13, moy5 + 6, bx1 + 16, moy5 + 8, ORANGE);
    ImageDrawLine(&img, bx1 + 19, moy5 + 6, bx1 + 16, moy5 + 8, ORANGE);
    
    // Frame 2: Beetle legs toggled/walking asymmetry
    int bx2 = 2 * 32;
    ImageDrawRectangle(&img, bx2 + 13, moy5 + 10, 6, 12, (Color){ 45, 32, 22, 255 });
    ImageDrawCircle(&img, bx2 + 16, moy5 + 8, 4, (Color){ 30, 20, 15, 255 });
    // Legs asymmetrical
    ImageDrawLine(&img, bx2 + 9, moy5 + 10, bx2 + 21, moy5 + 12, BLACK);
    ImageDrawLine(&img, bx2 + 12, moy5 + 14, bx2 + 23, moy5 + 16, BLACK);
    ImageDrawLine(&img, bx2 + 9, moy5 + 19, bx2 + 21, moy5 + 17, BLACK);
    // Antennas
    ImageDrawLine(&img, bx2 + 14, moy5 + 5, bx2 + 16, moy5 + 8, ORANGE);
    ImageDrawLine(&img, bx2 + 18, moy5 + 5, bx2 + 16, moy5 + 8, ORANGE);
    
    // --- 6. CAVE FAUNA ANIMATED FIREFLIES (Row 6: ty = 6) ---
    int moy6 = 6 * 32;
    // Frame 0: Small glow
    int fx0 = 0 * 32;
    ImageDrawCircle(&img, fx0 + 16, moy6 + 16, 5, Fade(YELLOW, 0.4f)); // soft outer glow
    ImageDrawCircle(&img, fx0 + 16, moy6 + 16, 2, (Color){ 255, 255, 100, 255 }); // yellow core
    ImageDrawPixel(&img, fx0 + 16, moy6 + 16, WHITE); // white hot core
    
    // Frame 1: Big glow with destello (cross flare)
    int fx1 = 1 * 32;
    ImageDrawCircle(&img, fx1 + 16, moy6 + 16, 9, Fade(GOLD, 0.35f));
    ImageDrawCircle(&img, fx1 + 16, moy6 + 16, 4, Fade(YELLOW, 0.6f));
    ImageDrawCircle(&img, fx1 + 16, moy6 + 16, 2, WHITE);
    // Flare cross lines
    ImageDrawLine(&img, fx1 + 11, moy6 + 16, fx1 + 21, moy6 + 16, Fade(WHITE, 0.8f));
    ImageDrawLine(&img, fx1 + 16, moy6 + 11, fx1 + 16, moy6 + 21, Fade(WHITE, 0.8f));
    
    // Frame 2: Soft pulsing medium glow
    int fx2 = 2 * 32;
    ImageDrawCircle(&img, fx2 + 16, moy6 + 16, 7, Fade(LIME, 0.3f));
    ImageDrawCircle(&img, fx2 + 16, moy6 + 16, 3, Fade(YELLOW, 0.5f));
    ImageDrawCircle(&img, fx2 + 16, moy6 + 16, 1, WHITE);
    
    CleanImageBackground(&img, BLACK);
    ExportImage(img, "tile_spritesheet.png");
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    return tex;
}

// Procedural Spaceship Dungeon Generator
void GenerateProceduralDungeon() {
    for (int y = 0; y < DUNGEON_SIZE; y++) {
        for (int x = 0; x < DUNGEON_SIZE; x++) {
            dungeon[y][x].active = false;
            dungeon[y][x].cleared = false;
            dungeon[y][x].numPillars = 0;
            dungeon[y][x].numEnemies = 0;
            dungeon[y][x].numItems = 0;
            dungeon[y][x].gridX = x;
            dungeon[y][x].gridY = y;
            for (int d = 0; d < 4; d++) dungeon[y][x].doors[d] = false;
            
            for (int ty = 0; ty < ROOM_GRID_SIZE; ty++) {
                for (int tx = 0; tx < ROOM_GRID_SIZE; tx++) {
                    dungeon[y][x].floorTileVariants[ty][tx] = GetRandomValue(0, 1);
                    dungeon[y][x].wallTileVariants[ty][tx] = GetRandomValue(0, 1);
                }
            }
        }
    }
    
    int startX = 2;
    int startY = 2;
    dungeon[startY][startX].active = true;
    dungeon[startY][startX].type = ROOM_START;
    dungeon[startY][startX].cleared = true;
    
    int totalRooms = 1;
    int targetRooms = 7;
    
    struct Coord { int x; int y; };
    Coord queue[32];
    int qHead = 0, qTail = 0;
    queue[qTail++] = { startX, startY };
    
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };
    
    Coord spawnedCoords[32];
    int spawnedCount = 0;
    spawnedCoords[spawnedCount++] = { startX, startY };
    
    while (qHead < qTail && totalRooms < targetRooms) {
        Coord curr = queue[qHead++];
        
        for (int d = 0; d < 4; d++) {
            int nx = curr.x + dx[d];
            int ny = curr.y + dy[d];
            
            if (nx >= 0 && nx < DUNGEON_SIZE && ny >= 0 && ny < DUNGEON_SIZE) {
                if (!dungeon[ny][nx].active && GetRandomValue(0, 100) < 65) {
                    dungeon[ny][nx].active = true;
                    dungeon[ny][nx].type = ROOM_ENEMY;
                    dungeon[ny][nx].cleared = false;
                    
                    spawnedCoords[spawnedCount++] = { nx, ny };
                    queue[qTail++] = { nx, ny };
                    totalRooms++;
                    if (totalRooms >= targetRooms) break;
                }
            }
        }
    }
    
    for (int y = 0; y < DUNGEON_SIZE; y++) {
        for (int x = 0; x < DUNGEON_SIZE; x++) {
            if (!dungeon[y][x].active) continue;
            for (int d = 0; d < 4; d++) {
                int nx = x + dx[d];
                int ny = y + dy[d];
                if (nx >= 0 && nx < DUNGEON_SIZE && ny >= 0 && ny < DUNGEON_SIZE) {
                    if (dungeon[ny][nx].active) {
                        dungeon[y][x].doors[d] = true;
                    }
                }
            }
        }
    }
    
    // Boss Room
    int furthestIdx = 0;
    float maxDist = 0.0f;
    for (int i = 1; i < spawnedCount; i++) {
        float dist = abs(spawnedCoords[i].x - startX) + abs(spawnedCoords[i].y - startY);
        if (dist > maxDist) {
            maxDist = dist;
            furthestIdx = i;
        }
    }
    dungeon[spawnedCoords[furthestIdx].y][spawnedCoords[furthestIdx].x].type = ROOM_BOSS;
    
    // Treasure Room
    for (int i = 1; i < spawnedCount; i++) {
        if (i == furthestIdx) continue;
        int tx = spawnedCoords[i].x;
        int ty = spawnedCoords[i].y;
        int doorsNum = 0;
        for (int d = 0; d < 4; d++) if (dungeon[ty][tx].doors[d]) doorsNum++;
        if (doorsNum == 1) {
            dungeon[ty][tx].type = ROOM_TREASURE;
            dungeon[ty][tx].cleared = true;
            
            // Spawn unique Power-up item inside Treasure room center
            dungeon[ty][tx].numItems = 1;
            dungeon[ty][tx].items[0].position = (Vector3){ 0.0f, 1.0f, 0.0f };
            dungeon[ty][tx].items[0].type = (ItemType)GetRandomValue(1, 3); // Cyber Eye, Boots, or Acid Glands!
            dungeon[ty][tx].items[0].active = true;
            dungeon[ty][tx].items[0].animTimer = 0.0f;
            break;
        }
    }
    
    // Populate obstacles & diverse enemies
    for (int i = 0; i < spawnedCount; i++) {
        int rx = spawnedCoords[i].x;
        int ry = spawnedCoords[i].y;
        Room &r = dungeon[ry][rx];
        
        if (rx == startX && ry == startY) continue;
        
        r.numPillars = GetRandomValue(1, 3);
        for (int p = 0; p < r.numPillars; p++) {
            r.pillars[p] = (Vector3){
                (float)GetRandomValue(-3, 3) * 2.0f,
                2.0f,
                (float)GetRandomValue(-3, 3) * 2.0f
            };
        }
        
        if (r.type == ROOM_ENEMY) {
            // Difficulty balances:
            int enemyCount = GetRandomValue(2, 4);
            if (selectedDifficulty == DIFF_EASY) enemyCount = GetRandomValue(1, 3);
            else if (selectedDifficulty == DIFF_HARD) enemyCount = GetRandomValue(3, 5);
            
            r.numEnemies = enemyCount;
            for (int e = 0; e < r.numEnemies; e++) {
                r.enemies[e].position = (Vector3){
                    (float)GetRandomValue(-5, 5) * 1.2f,
                    1.0f,
                    (float)GetRandomValue(-5, 5) * 1.2f
                };
                r.enemies[e].radius = 0.55f;
                r.enemies[e].state = STATE_RUN;
                r.enemies[e].stateTimer = 0.0f;
                r.enemies[e].animTimer = (float)GetRandomValue(0, 100) * 0.01f;
                r.enemies[e].animFrame = 0;
                r.enemies[e].hasFiredAttack = false;
                r.enemies[e].isBoss = false;
                r.enemies[e].gasTimer = 0.0f;
                
                // Select arquetype randomly
                int typeVal = GetRandomValue(0, 2);
                r.enemies[e].enemyType = (EnemyType)typeVal;
                
                // Difficulty health scaling
                float baseHp = 30.0f;
                float baseSpd = 2.4f;
                
                if (typeVal == ENEMY_KAMIKAZE) { // Fast melee drone
                    baseHp = 22.0f;
                    baseSpd = 4.2f;
                    r.enemies[e].radius = 0.5f;
                } else if (typeVal == ENEMY_SENTRY) { // Static spore spitter
                    baseHp = 45.0f;
                    baseSpd = 0.0f; // completely stationary Sentry!
                } else { // Heavy poison spreader
                    baseHp = 50.0f;
                    baseSpd = 1.8f;
                }
                
                if (selectedDifficulty == DIFF_EASY) {
                    baseHp *= 0.7f;
                    baseSpd *= 0.8f;
                } else if (selectedDifficulty == DIFF_HARD) {
                    baseHp *= 1.3f;
                    baseSpd *= 1.25f;
                }
                
                r.enemies[e].health = baseHp;
                r.enemies[e].maxHealth = baseHp;
                r.enemies[e].speed = baseSpd;
            }
        }
        else if (r.type == ROOM_BOSS) {
            r.numEnemies = 1;
            r.enemies[0].position = (Vector3){ 0.0f, 1.3f, 0.0f };
            r.enemies[0].radius = 1.3f;
            r.enemies[0].isBoss = true;
            r.enemies[0].enemyType = ENEMY_SPREADER; // Boss is a giant AoE Poison Spreader!
            r.enemies[0].gasTimer = 0.0f;
            
            float bHp = 180.0f;
            float bSpd = 2.4f;
            if (selectedDifficulty == DIFF_EASY) bHp = 120.0f;
            else if (selectedDifficulty == DIFF_HARD) { bHp = 240.0f; bSpd = 3.1f; }
            
            r.enemies[0].health = bHp;
            r.enemies[0].maxHealth = bHp;
            r.enemies[0].speed = bSpd;
            r.enemies[0].state = STATE_RUN;
            r.enemies[0].stateTimer = 0.0f;
            r.enemies[0].animTimer = 0.0f;
            r.enemies[0].animFrame = 0;
            r.enemies[0].hasFiredAttack = false;
        }
        
        // --- SEED-BASED DECORATION CLUSTERING ---
        r.numDecorations = 0;
        
        // Let's create 3 or 4 seeds in the room
        int numSeeds = GetRandomValue(3, 5);
        Vector3 seeds[5];
        
        // Define seed points: some near corners, some near pillars, some completely random
        for (int s = 0; s < numSeeds; s++) {
            if (s < r.numPillars) {
                // Seed near a pillar (beautiful accumulation!)
                float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
                seeds[s] = (Vector3){
                    r.pillars[s].x + cosf(angle) * 1.0f,
                    0.0f,
                    r.pillars[s].z + sinf(angle) * 1.0f
                };
            } else {
                // Seed in random walkable spots
                seeds[s] = (Vector3){
                    (float)GetRandomValue(-7, 7) * 1.0f,
                    0.0f,
                    (float)GetRandomValue(-7, 7) * 1.0f
                };
            }
        }
        
        // From each seed, grow a cluster of plants/insects
        for (int s = 0; s < numSeeds; s++) {
            int clusterSize = GetRandomValue(5, 10);
            for (int c = 0; c < clusterSize; c++) {
                if (r.numDecorations >= MAX_ROOM_DECORATIONS) break;
                
                // Disperse coordinates radially from the seed
                float angle = (float)GetRandomValue(0, 359) * DEG2RAD;
                float radius = (float)GetRandomValue(10, 220) * 0.01f; // up to 2.2 units out
                
                float dx = cosf(angle) * radius;
                float dz = sinf(angle) * radius;
                
                Vector3 decPos = (Vector3){ seeds[s].x + dx, 0.0f, seeds[s].z + dz };
                
                // Walkability checks
                if (decPos.x < -8.5f || decPos.x > 8.5f || decPos.z < -8.5f || decPos.z > 8.5f) continue;
                
                // Avoid doorways (the cross corridors of width 2.5)
                if (fabsf(decPos.x) < 1.8f && fabsf(decPos.z) > 6.0f) continue; // North/South doors
                if (fabsf(decPos.z) < 1.8f && fabsf(decPos.x) > 6.0f) continue; // East/West doors
                
                // Avoid too close to start room spawn center (if start room)
                if (rx == startX && ry == startY && decPos.x * decPos.x + decPos.z * decPos.z < 9.0f) continue;
                
                // Avoid too close to boss spawn in boss room
                if (r.type == ROOM_BOSS && decPos.x * decPos.x + decPos.z * decPos.z < 4.0f) continue;
                
                // Avoid pillar overlap
                bool overlapsPillar = false;
                for (int p = 0; p < r.numPillars; p++) {
                    float pdx = decPos.x - r.pillars[p].x;
                    float pdz = decPos.z - r.pillars[p].z;
                    if (pdx * pdx + pdz * pdz < 1.3f) {
                        overlapsPillar = true;
                        break;
                    }
                }
                if (overlapsPillar) continue;
                
                // Weighted Probability Selector:
                // 60% Grass/Moss, 30% Flower/Mushroom, 10% Insect
                int roll = GetRandomValue(0, 99);
                Decoration dec = { 0 };
                dec.position = decPos;
                dec.active = true;
                dec.alpha = 1.0f;
                dec.fleeTimer = 0.0f;
                dec.animTimer = (float)GetRandomValue(0, 100) * 0.01f;
                dec.animFrame = 0;
                
                if (roll < 60) {
                    // Moss or Grass
                    int subRoll = GetRandomValue(0, 2);
                    if (subRoll == 0) dec.type = 0;      // Moss1 (dark green)
                    else if (subRoll == 1) dec.type = 1; // Moss2 (lime moss)
                    else dec.type = 2;                  // Grass
                    dec.isInsect = false;
                }
                else if (roll < 90) {
                    // Flower or Mushroom
                    int subRoll = GetRandomValue(0, 3);
                    if (subRoll == 0) dec.type = 3;      // Cyan Flower
                    else if (subRoll == 1) dec.type = 4; // Green Flower
                    else if (subRoll == 2) dec.type = 5; // Red Mushroom
                    else dec.type = 6;                  // Purple Mushroom
                    dec.isInsect = false;
                }
                else {
                    // Insect
                    dec.isInsect = true;
                    int subRoll = GetRandomValue(0, 1);
                    if (subRoll == 0) {
                        // Beetle
                        dec.type = 8; // Beetle (sprites drawn at row 5)
                        dec.isFly = false;
                        dec.speed = (float)GetRandomValue(6, 12) * 0.1f; // 0.6 - 1.2
                    } else {
                        // Firefly
                        dec.type = 9; // Firefly (sprites drawn at row 6)
                        dec.isFly = true;
                        dec.speed = (float)GetRandomValue(12, 22) * 0.1f; // 1.2 - 2.2
                        dec.position.y = (float)GetRandomValue(5, 12) * 0.1f; // starts suspended
                        dec.floatHeight = dec.position.y;
                        dec.bobOffset = (float)GetRandomValue(0, 100) * 0.1f;
                    }
                    
                    // Wander setup
                    float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
                    dec.velocity = (Vector3){ cosf(angle), 0.0f, sinf(angle) };
                    dec.wanderTimer = (float)GetRandomValue(10, 30) * 0.1f; // 1 to 3 secs
                }
                
                r.decorations[r.numDecorations++] = dec;
            }
        }
    }
    
    currentRoomX = startX;
    currentRoomY = startY;
}

Vector3 GetMouseGroundIntersection(Camera3D camera) {
    Ray ray = GetMouseRay(GetMousePosition(), camera);
    Vector3 intersection = { 0.0f, 0.0f, 0.0f };
    if (ray.direction.y != 0.0f) {
        float t = -ray.position.y / ray.direction.y;
        if (t >= 0.0f) {
            intersection.x = ray.position.x + t * ray.direction.x;
            intersection.y = 0.0f;
            intersection.z = ray.position.z + t * ray.direction.z;
        }
    }
    return intersection;
}

// Layered Rendering Dynamic Z-sorting setup
struct RenderBillboard {
    Vector3 position;
    Texture2D texture;
    Rectangle source;
    Vector2 size;
    Color tint;
    float depth;
    int layer; // 0 = environment/decorations, 1 = characters/enemies/projectiles/items
};

#define MAX_RENDER_BILLBOARDS 512
RenderBillboard billBuffer[MAX_RENDER_BILLBOARDS];
int billCount = 0;

void AddBillboardToRender(Vector3 pos, Texture2D tex, Rectangle src, Vector2 sz, Color col, int layer, Camera3D camera) {
    if (billCount >= MAX_RENDER_BILLBOARDS) return;
    
    Vector3 camToPos = Vector3Subtract(pos, camera.position);
    float depth = Vector3Length(camToPos);
    
    billBuffer[billCount++] = { pos, tex, src, sz, col, depth, layer };
}

void SortRenderBillboards() {
    for (int i = 0; i < billCount - 1; i++) {
        for (int j = i + 1; j < billCount; j++) {
            bool swap = false;
            // Sort by layer first (lower layer numbers are drawn first)
            if (billBuffer[j].layer < billBuffer[i].layer) {
                swap = true;
            } else if (billBuffer[j].layer == billBuffer[i].layer) {
                // If on the same layer, sort by depth (furthest drawn first)
                if (billBuffer[j].depth > billBuffer[i].depth) {
                    swap = true;
                }
            }
            
            if (swap) {
                RenderBillboard temp = billBuffer[i];
                billBuffer[i] = billBuffer[j];
                billBuffer[j] = temp;
            }
        }
    }
}

// Custom UI container renderer for Heart energy cores
void DrawHeartUI(int x, int y, int health, int maxHearts) {
    for (int i = 0; i < maxHearts; i++) {
        int heartX = x + i * 38;
        
        // Dark outer circle border
        DrawCircle(heartX + 16, y + 16, 17, (Color){ 20, 20, 24, 200 });
        DrawCircle(heartX + 16, y + 16, 14, BLACK);
        
        int heartState = health - (i * 2);
        
        if (heartState >= 2) {
            // Full heart core (Neon glowing red battery cell shape)
            DrawRectangle(heartX + 10, y + 8, 12, 16, RED);
            DrawRectangle(heartX + 8, y + 10, 16, 12, RED);
            DrawCircle(heartX + 16, y + 16, 3, WHITE); // glowing core center
        } else if (heartState == 1) {
            // Half heart core
            DrawRectangle(heartX + 10, y + 8, 6, 16, RED);
            DrawRectangle(heartX + 8, y + 10, 8, 12, RED);
            DrawCircle(heartX + 13, y + 16, 2, WHITE);
        } else {
            // Empty container
            DrawCircleLines(heartX + 16, y + 16, 8, DARKGRAY);
        }
    }
}

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "PROJECT: DEHUMANIZER - Spaceship Rogue 2.5D");
    
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 16.0f, 13.5f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 52.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    Texture2D charSpritesheet;
    if (FindResourcePath("spritesheet.png") != NULL) {
        charSpritesheet = LoadTextureWithChromaKey("spritesheet.png", BLACK);
    } else {
        charSpritesheet = GenerateProceduralSpritesheet();
    }
    
    Texture2D envSpritesheet;
    if (FindResourcePath("tile_spritesheet.png") != NULL) {
        envSpritesheet = LoadTextureWithChromaKey("tile_spritesheet.png", BLACK);
    } else {
        envSpritesheet = GenerateEnvironmentTileSheet();
    }
    InitStarfield();
    
    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model cubeModel = LoadModelFromMesh(cubeMesh);
    cubeModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = envSpritesheet;
    
    Mesh floorMesh = GenMeshPlane(1.0f, 1.0f, 1, 1);
    Model floorModel = LoadModelFromMesh(floorMesh);
    floorModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = envSpritesheet;
    
    Entity player = { 0 };
    Projectile projectiles[MAX_PROJECTILES] = { 0 };
    
    float gameTimer = 0.0f;
    float screenShake = 0.0f;
    GameScreen currentScreen = SCREEN_TITLE;
    
    auto ResetGame = [&]() {
        player.position = (Vector3){ 0.0f, 1.0f, 3.0f };
        player.radius = 0.5f;
        player.speed = 6.8f;
        player.state = STATE_IDLE;
        player.stateTimer = 0.0f;
        player.animTimer = 0.0f;
        player.animFrame = 0;
        
        // Difficulty player setup
        if (selectedDifficulty == DIFF_EASY) {
            playerMaxHearts = 4;
            playerHearts = 4;
            playerHalfHeartsHealth = 8;
        } else {
            playerMaxHearts = 3;
            playerHearts = 3;
            playerHalfHeartsHealth = 6;
        }
        
        hasCyberEye = false;
        hasThrusterBoots = false;
        hasAcidGlands = false;
        
        for (int i = 0; i < MAX_PROJECTILES; i++) projectiles[i].active = false;
        for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;
        for (int i = 0; i < MAX_IMPACTS; i++) impacts[i].active = false;
        
        GenerateProceduralDungeon();
        
        gameTimer = 0.0f;
        screenShake = 0.0f;
    };
    
    ResetGame();
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;
        
        if (currentScreen == SCREEN_TITLE) {
            // Main menu controls
            if (IsKeyPressed(KEY_ONE)) selectedDifficulty = DIFF_EASY;
            if (IsKeyPressed(KEY_TWO)) selectedDifficulty = DIFF_NORMAL;
            if (IsKeyPressed(KEY_THREE)) selectedDifficulty = DIFF_HARD;
            
            if (IsKeyPressed(KEY_ENTER)) {
                ResetGame();
                currentScreen = SCREEN_GAMEPLAY;
            }
        }
        else if (currentScreen == SCREEN_GAMEOVER || currentScreen == SCREEN_VICTORY) {
            if (IsKeyPressed(KEY_R)) {
                ResetGame();
                currentScreen = SCREEN_GAMEPLAY;
            }
        }
        else if (currentScreen == SCREEN_ROOM_TRANSITION) {
            transitionTimer += dt * 2.0f;
            if (transitionTimer >= 1.0f) {
                transitionTimer = 1.0f;
                currentRoomX = nextRoomX;
                currentRoomY = nextRoomY;
                player.position = transitionPlayerEnd;
                currentScreen = SCREEN_GAMEPLAY;
            } else {
                player.position = Vector3Lerp(transitionPlayerStart, transitionPlayerEnd, transitionTimer);
            }
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            gameTimer += dt;
            
            Room &currentRoom = dungeon[currentRoomY][currentRoomX];
            
            // Check Room Cleanliness
            bool monstersAlive = false;
            for (int e = 0; e < currentRoom.numEnemies; e++) {
                if (currentRoom.enemies[e].health > 0) {
                    monstersAlive = true;
                    break;
                }
            }
            if (!monstersAlive) {
                currentRoom.cleared = true;
            }
            
            // --- LOOT ITEMS PICKUP LOGIC ---
            for (int i = 0; i < currentRoom.numItems; i++) {
                GroundItem &it = currentRoom.items[i];
                if (it.active) {
                    float dx = player.position.x - it.position.x;
                    float dz = player.position.z - it.position.z;
                    float distSq = dx * dx + dz * dz;
                    float pickDist = player.radius + 0.6f;
                    if (distSq < pickDist * pickDist) {
                        it.active = false;
                        SpawnParticles(it.position, CYAN, 15);
                        
                        // Apply specific power-up passive
                        if (it.type == ITEM_HEAL) {
                            playerHalfHeartsHealth += 2; // heal full heart
                            if (playerHalfHeartsHealth > playerMaxHearts * 2) {
                                playerHalfHeartsHealth = playerMaxHearts * 2;
                            }
                        } else if (it.type == ITEM_CYBER_EYE) {
                            hasCyberEye = true;
                        } else if (it.type == ITEM_BOOTS) {
                            hasThrusterBoots = true;
                            player.speed = 9.2f; // increase speed!
                        } else if (it.type == ITEM_ACID) {
                            hasAcidGlands = true;
                        }
                    }
                }
            }
            
            // --- WASD PLAYER CONTROLLER ---
            Vector3 moveVector = { 0 };
            if (IsKeyDown(KEY_W)) moveVector.z -= 1.0f;
            if (IsKeyDown(KEY_S)) moveVector.z += 1.0f;
            if (IsKeyDown(KEY_A)) moveVector.x -= 1.0f;
            if (IsKeyDown(KEY_D)) moveVector.x += 1.0f;
            
            if (player.stateTimer > 0.0f) {
                player.stateTimer -= dt;
                if (player.stateTimer <= 0.0f) player.state = STATE_IDLE;
            }
            
            if (player.state != STATE_HURT) {
                if (Vector3Length(moveVector) > 0.0f) {
                    moveVector = Vector3Normalize(moveVector);
                    player.position = Vector3Add(player.position, Vector3Scale(moveVector, player.speed * dt));
                    player.state = STATE_RUN;
                    
                    player.animTimer += dt * 10.0f;
                    player.animFrame = ((int)player.animTimer) % 4;
                    player.direction = moveVector;
                    
                    // Thruster boots particle tail trail
                    if (hasThrusterBoots && GetRandomValue(0, 10) < 3) {
                        SpawnParticles(player.position, CYAN, 1);
                    }
                } else {
                    player.state = STATE_IDLE;
                    player.animFrame = 0;
                }
            }
            
            // Boundary checking / Transitioning through doors
            float wallLeft = -9.2f;
            float wallRight = 9.2f;
            float wallTop = -9.2f;
            float wallBottom = 9.2f;
            
            if (player.position.z < wallTop) {
                if (currentRoom.doors[0] && currentRoom.cleared && fabsf(player.position.x) < 1.2f) {
                    nextRoomX = currentRoomX;
                    nextRoomY = currentRoomY - 1;
                    transitionPlayerStart = player.position;
                    transitionPlayerEnd = (Vector3){ 0.0f, 1.0f, 8.8f };
                    transitionTimer = 0.0f;
                    currentScreen = SCREEN_ROOM_TRANSITION;
                    for (int i = 0; i < MAX_PROJECTILES; i++) projectiles[i].active = false;
                } else player.position.z = wallTop;
            }
            if (player.position.z > wallBottom) {
                if (currentRoom.doors[1] && currentRoom.cleared && fabsf(player.position.x) < 1.2f) {
                    nextRoomX = currentRoomX;
                    nextRoomY = currentRoomY + 1;
                    transitionPlayerStart = player.position;
                    transitionPlayerEnd = (Vector3){ 0.0f, 1.0f, -8.8f };
                    transitionTimer = 0.0f;
                    currentScreen = SCREEN_ROOM_TRANSITION;
                    for (int i = 0; i < MAX_PROJECTILES; i++) projectiles[i].active = false;
                } else player.position.z = wallBottom;
            }
            if (player.position.x < wallLeft) {
                if (currentRoom.doors[2] && currentRoom.cleared && fabsf(player.position.z) < 1.2f) {
                    nextRoomX = currentRoomX - 1;
                    nextRoomY = currentRoomY;
                    transitionPlayerStart = player.position;
                    transitionPlayerEnd = (Vector3){ 8.8f, 1.0f, 0.0f };
                    transitionTimer = 0.0f;
                    currentScreen = SCREEN_ROOM_TRANSITION;
                    for (int i = 0; i < MAX_PROJECTILES; i++) projectiles[i].active = false;
                } else player.position.x = wallLeft;
            }
            if (player.position.x > wallRight) {
                if (currentRoom.doors[3] && currentRoom.cleared && fabsf(player.position.z) < 1.2f) {
                    nextRoomX = currentRoomX + 1;
                    nextRoomY = currentRoomY;
                    transitionPlayerStart = player.position;
                    transitionPlayerEnd = (Vector3){ -8.8f, 1.0f, 0.0f };
                    transitionTimer = 0.0f;
                    currentScreen = SCREEN_ROOM_TRANSITION;
                    for (int i = 0; i < MAX_PROJECTILES; i++) projectiles[i].active = false;
                } else player.position.x = wallRight;
            }
            
            // Escape Trapdoor
            if (currentRoom.type == ROOM_BOSS && currentRoom.cleared) {
                float dx = player.position.x;
                float dz = player.position.z;
                if (dx * dx + dz * dz < 1.0f) {
                    currentScreen = SCREEN_VICTORY;
                }
            }
            
            // Pillars collisions
            for (int i = 0; i < currentRoom.numPillars; i++) {
                float dx = player.position.x - currentRoom.pillars[i].x;
                float dz = player.position.z - currentRoom.pillars[i].z;
                float distSq = dx * dx + dz * dz;
                float minDist = player.radius + 0.8f;
                if (distSq < minDist * minDist) {
                    float dist = sqrtf(distSq);
                    if (dist > 0.0f) {
                        float overlap = minDist - dist;
                        player.position.x += (dx / dist) * overlap;
                        player.position.z += (dz / dist) * overlap;
                    }
                }
            }
            
            // Check standing on Toxic Gas clouds
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].active && particles[i].isGas) {
                    float pdx = player.position.x - particles[i].position.x;
                    float pdz = player.position.z - particles[i].position.z;
                    float distSq = pdx * pdx + pdz * pdz;
                    if (distSq < 0.64f && player.state != STATE_HURT) {
                        // Toxic gas ticks half-heart
                        playerHalfHeartsHealth -= 1;
                        player.state = STATE_HURT;
                        player.stateTimer = 0.28f;
                        screenShake = 0.15f;
                        SpawnParticles(player.position, PURPLE, 6);
                        
                        if (playerHalfHeartsHealth <= 0) currentScreen = SCREEN_GAMEOVER;
                    }
                }
            }
            
            // --- SHOOT LOGIC ---
            Vector3 groundAim = GetMouseGroundIntersection(camera);
            Vector3 aimDir = Vector3Subtract(groundAim, player.position);
            aimDir.y = 0.0f;
            
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && player.state != STATE_HURT) {
                if (Vector3Length(aimDir) > 0.0f) {
                    Vector3 fireDir = Vector3Normalize(aimDir);
                    for (int i = 0; i < MAX_PROJECTILES; i++) {
                        if (!projectiles[i].active) {
                            projectiles[i].position = player.position;
                            projectiles[i].position.y = 1.0f;
                            projectiles[i].direction = fireDir;
                            projectiles[i].radius = hasAcidGlands ? 0.35f : 0.2f;
                            
                            // Cyber Eye upgrade increases projectile speed and damage
                            projectiles[i].speed = hasCyberEye ? 25.0f : 18.0f;
                            
                            projectiles[i].active = true;
                            projectiles[i].isEnemy = false;
                            projectiles[i].isAcid = hasAcidGlands;
                            
                            player.state = STATE_ATTACK;
                            
                            // Cyber eye increases fire rate cooldown
                            player.stateTimer = hasCyberEye ? 0.09f : 0.15f;
                            
                            SpawnParticles(player.position, hasAcidGlands ? LIME : SKYBLUE, 3);
                            break;
                        }
                    }
                }
            }
            
            // --- UPDATE PROJECTILES ---
            for (int i = 0; i < MAX_PROJECTILES; i++) {
                if (projectiles[i].active) {
                    projectiles[i].position = Vector3Add(projectiles[i].position, Vector3Scale(projectiles[i].direction, projectiles[i].speed * dt));
                    
                    if (projectiles[i].position.x > 10.0f || projectiles[i].position.x < -10.0f ||
                        projectiles[i].position.z > 10.0f || projectiles[i].position.z < -10.0f) {
                        projectiles[i].active = false;
                        SpawnImpact(projectiles[i].position);
                        continue;
                    }
                    
                    for (int p = 0; p < currentRoom.numPillars; p++) {
                        float dx = projectiles[i].position.x - currentRoom.pillars[p].x;
                        float dz = projectiles[i].position.z - currentRoom.pillars[p].z;
                        float distSq = dx * dx + dz * dz;
                        float minDist = projectiles[i].radius + 0.8f;
                        if (distSq < minDist * minDist) {
                            projectiles[i].active = false;
                            SpawnImpact(projectiles[i].position);
                            break;
                        }
                    }
                }
            }
            
            // --- UPDATE ENEMY AI & LOOPS ---
            for (int e = 0; e < currentRoom.numEnemies; e++) {
                Entity &enemy = currentRoom.enemies[e];
                if (enemy.state == STATE_DEAD) {
                    enemy.stateTimer += dt;
                    if (enemy.stateTimer >= 0.4f) {
                        // Drop consumable item on death!
                        if (enemy.health > -40.0f) {
                            enemy.health = -99.0f;
                            
                            // drop scaling based on difficulty
                            int dropChance = 35;
                            if (selectedDifficulty == DIFF_EASY) dropChance = 65;
                            else if (selectedDifficulty == DIFF_HARD) dropChance = 15;
                            
                            if (GetRandomValue(0, 100) < dropChance && currentRoom.numItems < MAX_GROUND_ITEMS) {
                                int itIdx = currentRoom.numItems;
                                currentRoom.items[itIdx].position = enemy.position;
                                currentRoom.items[itIdx].position.y = 0.5f;
                                currentRoom.items[itIdx].type = ITEM_HEAL; // Health battery core
                                currentRoom.items[itIdx].active = true;
                                currentRoom.items[itIdx].animTimer = 0.0f;
                                currentRoom.numItems++;
                            }
                        }
                    }
                    continue;
                }
                if (enemy.health <= -50.0f) continue;
                
                // Tick timers
                if (enemy.state == STATE_HURT) {
                    enemy.stateTimer -= dt;
                    if (enemy.stateTimer <= 0.0f) enemy.state = STATE_RUN;
                }
                else if (enemy.state == STATE_ATTACK) {
                    enemy.stateTimer += dt;
                    
                    if (enemy.enemyType == ENEMY_KAMIKAZE) {
                        // Melee Kamikaze explodes immediately upon charging close!
                        enemy.state = STATE_DEAD;
                        enemy.stateTimer = 0.0f;
                        screenShake = 0.45f;
                        
                        // Explosion particles
                        SpawnParticles(enemy.position, RED, 20);
                        SpawnParticles(enemy.position, ORANGE, 15);
                        
                        // Damage player
                        float pdx = player.position.x - enemy.position.x;
                        float pdz = player.position.z - enemy.position.z;
                        float distSq = pdx * pdx + pdz * pdz;
                        if (distSq < 4.0f && player.state != STATE_HURT) { // Explosion radius
                            // Kamikaze deals full heart in Normal/Hard
                            int kamDmg = (selectedDifficulty == DIFF_EASY) ? 1 : 2;
                            playerHalfHeartsHealth -= kamDmg;
                            player.state = STATE_HURT;
                            player.stateTimer = 0.28f;
                            if (playerHalfHeartsHealth <= 0) currentScreen = SCREEN_GAMEOVER;
                        }
                        continue;
                    }
                    
                    // Spore Sentry Ranged Spitter fires bursts
                    if (enemy.enemyType == ENEMY_SENTRY && enemy.stateTimer >= 0.25f && !enemy.hasFiredAttack) {
                        enemy.hasFiredAttack = true;
                        Vector3 fireDir = Vector3Subtract(player.position, enemy.position);
                        fireDir.y = 0.0f;
                        if (Vector3Length(fireDir) > 0.0f) {
                            fireDir = Vector3Normalize(fireDir);
                            // Fire 3 spore lasers spread slightly
                            for (int burst = 0; burst < 3; burst++) {
                                for (int i = 0; i < MAX_PROJECTILES; i++) {
                                    if (!projectiles[i].active) {
                                        projectiles[i].position = enemy.position;
                                        projectiles[i].position.y = 1.0f;
                                        
                                        // Spread laser directions
                                        float offsetAngle = (float)(burst - 1) * 0.15f;
                                        Vector3 rotDir = {
                                            fireDir.x * cosf(offsetAngle) - fireDir.z * sinf(offsetAngle),
                                            0.0f,
                                            fireDir.x * sinf(offsetAngle) + fireDir.z * cosf(offsetAngle)
                                        };
                                        
                                        projectiles[i].direction = rotDir;
                                        projectiles[i].speed = 11.0f;
                                        projectiles[i].radius = 0.22f;
                                        projectiles[i].active = true;
                                        projectiles[i].isEnemy = true;
                                        projectiles[i].isAcid = false;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (enemy.stateTimer >= 0.65f) {
                        enemy.state = STATE_RUN;
                        enemy.hasFiredAttack = false;
                    }
                }
                
                // Gas trail dropping for Spreader AoE Blob
                if (enemy.enemyType == ENEMY_SPREADER && enemy.state != STATE_HURT) {
                    enemy.gasTimer += dt;
                    if (enemy.gasTimer >= 0.7f) {
                        enemy.gasTimer = 0.0f;
                        // Spawn toxic purple flat particle cloud on floor
                        SpawnParticles(enemy.position, PURPLE, 1, true);
                    }
                }
                
                // Chase
                if (enemy.state != STATE_HURT && enemy.state != STATE_ATTACK) {
                    Vector3 toPlayer = Vector3Subtract(player.position, enemy.position);
                    toPlayer.y = 0.0f;
                    float dist = Vector3Length(toPlayer);
                    
                    float detectionRange = enemy.enemyType == ENEMY_SENTRY ? 9.5f : 12.0f;
                    float triggerRange = enemy.enemyType == ENEMY_SENTRY ? 8.0f : 
                                         enemy.enemyType == ENEMY_KAMIKAZE ? 1.5f : 4.0f;
                    
                    if (dist < triggerRange && dist > 0.4f) {
                        enemy.state = STATE_ATTACK;
                        enemy.stateTimer = 0.0f;
                        enemy.hasFiredAttack = false;
                    }
                    else if (dist < detectionRange && dist > 0.2f && enemy.speed > 0.0f) {
                        Vector3 chase = Vector3Normalize(toPlayer);
                        enemy.position = Vector3Add(enemy.position, Vector3Scale(chase, enemy.speed * dt));
                        enemy.state = STATE_RUN;
                        
                        enemy.animTimer += dt * 8.0f;
                        enemy.animFrame = ((int)enemy.animTimer) % 4;
                    }
                }
                
                // Pillar Clamping
                for (int p = 0; p < currentRoom.numPillars; p++) {
                    float dx = enemy.position.x - currentRoom.pillars[p].x;
                    float dz = enemy.position.z - currentRoom.pillars[p].z;
                    float distSq = dx * dx + dz * dz;
                    float minDist = enemy.radius + 0.8f;
                    if (distSq < minDist * minDist) {
                        float dist = sqrtf(distSq);
                        if (dist > 0.0f) {
                            float overlap = minDist - dist;
                            enemy.position.x += (dx / dist) * overlap;
                            enemy.position.z += (dz / dist) * overlap;
                        }
                    }
                }
                
                // Contact Damage player
                float pdx = player.position.x - enemy.position.x;
                float pdz = player.position.z - enemy.position.z;
                float pDistSq = pdx * pdx + pdz * pdz;
                float pMinDist = player.radius + enemy.radius;
                
                if (pDistSq < pMinDist * pMinDist && player.state != STATE_HURT) {
                    float dist = sqrtf(pDistSq);
                    if (dist > 0.0f) {
                        float overlap = pMinDist - dist;
                        player.position.x += (pdx / dist) * overlap * 0.6f;
                        player.position.z += (pdz / dist) * overlap * 0.6f;
                        
                        // Heart-based damage increments based on difficulty
                        int dmgPoints = 1; // standard half-heart
                        if (selectedDifficulty == DIFF_HARD || enemy.isBoss) dmgPoints = 2; // full heart
                        
                        playerHalfHeartsHealth -= dmgPoints;
                        player.state = STATE_HURT;
                        player.stateTimer = 0.26f;
                        screenShake = 0.35f;
                        SpawnParticles(player.position, RED, 10);
                        
                        if (playerHalfHeartsHealth <= 0) currentScreen = SCREEN_GAMEOVER;
                    }
                }
            }
            
            // --- DETECT PROJECTILE COLLISIONS ---
            for (int p = 0; p < MAX_PROJECTILES; p++) {
                if (!projectiles[p].active) continue;
                
                if (projectiles[p].isEnemy) {
                    // Enemy espora vs Player
                    float dx = projectiles[p].position.x - player.position.x;
                    float dz = projectiles[p].position.z - player.position.z;
                    float distSq = dx * dx + dz * dz;
                    float minDist = projectiles[p].radius + player.radius;
                    if (distSq < minDist * minDist && player.state != STATE_HURT) {
                        projectiles[p].active = false;
                        SpawnImpact(projectiles[p].position);
                        
                        int dmgPoints = (selectedDifficulty == DIFF_HARD) ? 2 : 1;
                        playerHalfHeartsHealth -= dmgPoints;
                        player.state = STATE_HURT;
                        player.stateTimer = 0.24f;
                        screenShake = 0.25f;
                        SpawnParticles(player.position, LIME, 10);
                        
                        if (playerHalfHeartsHealth <= 0) currentScreen = SCREEN_GAMEOVER;
                    }
                } else {
                    // Player tear/acid vs Enemy
                    for (int e = 0; e < currentRoom.numEnemies; e++) {
                        Entity &enemy = currentRoom.enemies[e];
                        if (enemy.health <= 0) continue;
                        float dx = projectiles[p].position.x - enemy.position.x;
                        float dz = projectiles[p].position.z - enemy.position.z;
                        float distSq = dx * dx + dz * dz;
                        float minDist = projectiles[p].radius + enemy.radius;
                        if (distSq < minDist * minDist) {
                            projectiles[p].active = false;
                            SpawnImpact(projectiles[p].position);
                            
                            // Acid glands deals double damage
                            float baseDmg = projectiles[p].isAcid ? 30.0f : 15.0f;
                            enemy.health -= baseDmg;
                            
                            enemy.state = STATE_HURT;
                            enemy.stateTimer = 0.20f;
                            
                            // Knockback
                            enemy.position = Vector3Add(enemy.position, Vector3Scale(projectiles[p].direction, 0.4f));
                            SpawnParticles(projectiles[p].position, projectiles[p].isAcid ? LIME : ORANGE, 8);
                            
                            if (enemy.health <= 0.0f) {
                                enemy.state = STATE_DEAD;
                                enemy.stateTimer = 0.0f;
                                SpawnParticles(enemy.position, RED, 18);
                            }
                            break;
                        }
                    }
                }
            }
            
            // --- UPDATE EFFECTS & PARTICLES ---
            for (int i = 0; i < MAX_IMPACTS; i++) {
                if (impacts[i].active) {
                    impacts[i].timer -= dt;
                    if (impacts[i].timer <= 0.0f) {
                        impacts[i].frame++;
                        impacts[i].timer = 0.07f;
                        if (impacts[i].frame >= 2) impacts[i].active = false;
                    }
                }
            }
            
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].active) {
                    if (particles[i].isGas) {
                        particles[i].life -= dt;
                    } else {
                        particles[i].position = Vector3Add(particles[i].position, Vector3Scale(particles[i].velocity, dt));
                        particles[i].velocity.y -= 9.8f * dt;
                        particles[i].life -= dt;
                    }
                    if (particles[i].life <= 0.0f) particles[i].active = false;
                }
            }
            
            // --- CAMERA TRACKING & SHAKE ---
            Vector3 targetCam = { player.position.x, 16.0f, player.position.z + 13.5f };
            camera.position = Vector3Lerp(camera.position, targetCam, 5.0f * dt);
            camera.target = Vector3Lerp(camera.target, player.position, 8.0f * dt);
            
            if (screenShake > 0.0f) {
                camera.position.x += (float)GetRandomValue(-100, 100) * 0.003f * screenShake;
                camera.position.y += (float)GetRandomValue(-100, 100) * 0.003f * screenShake;
                camera.position.z += (float)GetRandomValue(-100, 100) * 0.003f * screenShake;
                screenShake -= dt * 2.0f;
            }
            
            // --- UPDATE DECORATIONS & INSECTS ---
            for (int d = 0; d < currentRoom.numDecorations; d++) {
                Decoration &dec = currentRoom.decorations[d];
                if (!dec.active) continue;
                
                dec.animTimer += dt * 5.0f; // animation speed
                
                // Tick particles / bioluminescent glow (cyan/green/golden/purple sparks)
                if (dec.type == 3 || dec.type == 4 || dec.type == 6 || dec.type == 9) { // Cyan flower, Green flower, Purple mushroom, Firefly
                    if (GetRandomValue(0, 100) < 4) { // 4% chance per frame to spark
                        Color sparkCol = CYAN;
                        if (dec.type == 4) sparkCol = LIME;
                        else if (dec.type == 6) sparkCol = VIOLET;
                        else if (dec.type == 9) sparkCol = GOLD;
                        
                        // Spawn a glow spark particle
                        for (int p = 0; p < MAX_PARTICLES; p++) {
                            if (!particles[p].active) {
                                particles[p].active = true;
                                particles[p].position = dec.position;
                                if (dec.type == 9) particles[p].position.y = dec.position.y;
                                else particles[p].position.y = 0.5f;
                                
                                particles[p].velocity = (Vector3){
                                    (float)GetRandomValue(-4, 4) * 0.05f,
                                    (float)GetRandomValue(3, 8) * 0.1f, // floats upwards!
                                    (float)GetRandomValue(-4, 4) * 0.05f
                                };
                                particles[p].color = sparkCol;
                                particles[p].life = (float)GetRandomValue(6, 15) * 0.1f;
                                particles[p].maxLife = particles[p].life;
                                particles[p].isGas = false;
                                break;
                            }
                        }
                    }
                }
                
                if (!dec.isInsect) continue;
                
                // Bobbing hover effect for fireflies
                if (dec.isFly) {
                    dec.bobOffset += dt * 4.0f;
                    dec.position.y = dec.floatHeight + sinf(dec.bobOffset) * 0.15f;
                }
                
                // Check distance to player for fleeing behavior
                float pdx = player.position.x - dec.position.x;
                float pdz = player.position.z - dec.position.z;
                float distSq = pdx * pdx + pdz * pdz;
                float alertRadius = 3.5f; // player warning distance
                
                if (distSq < alertRadius * alertRadius) {
                    if (dec.fleeTimer == 0.0f) {
                        dec.fleeTimer = 2.0f; // start flee timer
                    }
                }
                
                if (dec.fleeTimer > 0.0f) {
                    dec.fleeTimer -= dt;
                    dec.alpha = dec.fleeTimer / 2.0f; // fade out opacity
                    if (dec.fleeTimer <= 0.0f) {
                        dec.active = false;
                        continue;
                    }
                    
                    if (dec.isFly) {
                        // Fly upwards into the dark ceiling!
                        dec.floatHeight += dt * 4.0f;
                        dec.position.y = dec.floatHeight;
                        // Float slightly away from player
                        float dist = sqrtf(distSq);
                        if (dist > 0.0f) {
                            dec.position.x -= (pdx / dist) * dt * 2.0f;
                            dec.position.z -= (pdz / dist) * dt * 2.0f;
                        }
                    } else {
                        // Beetle runs away in opposite direction!
                        float dist = sqrtf(distSq);
                        if (dist > 0.0f) {
                            dec.position.x -= (pdx / dist) * dt * dec.speed * 2.5f;
                            dec.position.z -= (pdz / dist) * dt * dec.speed * 2.5f;
                        }
                    }
                } else {
                    // Normal wandering behavior
                    dec.wanderTimer -= dt;
                    if (dec.wanderTimer <= 0.0f) {
                        dec.wanderTimer = (float)GetRandomValue(15, 35) * 0.1f;
                        if (GetRandomValue(0, 100) < 30) {
                            dec.velocity = (Vector3){ 0.0f, 0.0f, 0.0f }; // rest briefly
                        } else {
                            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
                            dec.velocity = (Vector3){ cosf(angle), 0.0f, sinf(angle) };
                        }
                    }
                    
                    // Move the insect
                    dec.position.x += dec.velocity.x * dt * dec.speed;
                    dec.position.z += dec.velocity.z * dt * dec.speed;
                    
                    // Boundaries check to keep insects inside the room grid
                    if (dec.position.x < -8.8f) { dec.position.x = -8.8f; dec.velocity.x *= -1.0f; }
                    if (dec.position.x > 8.8f) { dec.position.x = 8.8f; dec.velocity.x *= -1.0f; }
                    if (dec.position.z < -8.8f) { dec.position.z = -8.8f; dec.velocity.z *= -1.0f; }
                    if (dec.position.z > 8.8f) { dec.position.z = 8.8f; dec.velocity.z *= -1.0f; }
                }
            }
        }
        
        // --- DRAWING / RENDERING ---
        BeginDrawing();
            ClearBackground((Color){ 6, 6, 12, 255 }); // space void background
            
            if (currentScreen == SCREEN_TITLE) {
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){ 12, 14, 20, 255 });
                DrawText("PROJECT: DEHUMANIZER", screenWidth / 2 - MeasureText("PROJECT: DEHUMANIZER", 50) / 2, screenHeight / 2 - 160, 50, GOLD);
                DrawText("CAVE CRAWLER 2.5D", screenWidth / 2 - MeasureText("CAVE CRAWLER 2.5D", 20) / 2, screenHeight / 2 - 100, 20, CYAN);
                
                DrawText("SELECCIONA DIFICULTAD (Presiona numero):", screenWidth / 2 - 200, screenHeight / 2 - 40, 20, RAYWHITE);
                
                Color easCol = (selectedDifficulty == DIFF_EASY) ? LIME : GRAY;
                Color norCol = (selectedDifficulty == DIFF_NORMAL) ? GOLD : GRAY;
                Color harCol = (selectedDifficulty == DIFF_HARD) ? RED : GRAY;
                
                DrawText("1. FACIL (4 Corazones, Enemigos debiles, Drops abundantes)", screenWidth / 2 - 260, screenHeight / 2, 18, easCol);
                DrawText("2. NORMAL (3 Corazones, Balance estandar)", screenWidth / 2 - 260, screenHeight / 2 + 25, 18, norCol);
                DrawText("3. DIFICIL (Enemigos rapidos, Daño de 1 Corazon, Drops escasos)", screenWidth / 2 - 260, screenHeight / 2 + 50, 18, harCol);
                
                DrawText("PRESIONA ENTER PARA INICIAR LA EXPLORACION", screenWidth / 2 - MeasureText("PRESIONA ENTER PARA INICIAR LA EXPLORACION", 22) / 2, screenHeight / 2 + 130, 22, WHITE);
            }
            else {
                Room &room = dungeon[currentRoomY][currentRoomX];
                Vector3 groundAim = GetMouseGroundIntersection(camera);
                Vector3 mouseIntersect = groundAim;
                
                BeginMode3D(camera);
                    
                    // --- 0. PARALLAX SPACE STARFIELD (Distant space parallax stars) ---
                    for (int i = 0; i < MAX_STARS; i++) {
                        // Offset star coordinate based on camera panning to get realistic infinite drift depth
                        float driftX = spaceStars[i].position.x - camera.position.x * spaceStars[i].parallaxFactor;
                        float driftZ = spaceStars[i].position.z - camera.position.z * spaceStars[i].parallaxFactor;
                        Vector3 starPos = { driftX, spaceStars[i].position.y, driftZ };
                        
                        DrawBillboardRec(camera, charSpritesheet, (Rectangle){ 96.0f, 192.0f, 32.0f, 32.0f }, starPos, (Vector2){ spaceStars[i].size, spaceStars[i].size }, spaceStars[i].color);
                    }
                    
                    // Assign tile sector offset column indices based on room type
                    int tileOffsetCol = 0; // standard Steel grey compartment
                    if (room.type == ROOM_TREASURE) tileOffsetCol = 3; // luxury Gold/White
                    else if (room.type == ROOM_BOSS) tileOffsetCol = 6; // rusty Red warning
                    
                    // --- 1. FONDO LAYER (Dark back corridor grates behind doors) ---
                    for (int x = -11; x <= 11; x++) {
                        Rectangle backSrc = { (float)tileOffsetCol * 32.0f, 2.0f * 32.0f, 32.0f, 32.0f };
                        DrawBillboardRec(camera, envSpritesheet, backSrc, (Vector3){ (float)x, 2.5f, -11.6f }, (Vector2){ 1.0f, 5.0f }, (Color){ 65, 65, 70, 255 });
                        DrawBillboardRec(camera, envSpritesheet, backSrc, (Vector3){ (float)x, 2.5f, 11.6f }, (Vector2){ 1.0f, 5.0f }, (Color){ 65, 65, 70, 255 });
                    }
                    
                    // --- 2. SUELO LAYER (Spaceship compartment auto-tiled plates) ---
                    for (int z = 0; z < ROOM_GRID_SIZE; z++) {
                        for (int x = 0; x < ROOM_GRID_SIZE; x++) {
                            float px = (float)(x - 10);
                            float pz = (float)(z - 10);
                            
                            int tileCol = tileOffsetCol + room.floorTileVariants[z][x];
                            
                            if (room.type == ROOM_BOSS && room.cleared && x == 10 && z == 10) {
                                tileCol = 7; // Boss warning active red core trapdoor
                            }
                            else if (room.type == ROOM_TREASURE && room.cleared && x == 10 && z == 10) {
                                tileCol = 5; // Luxury gold-trim cyan trapdoor
                            }
                            
                            Rectangle tileSrc = { (float)tileCol * 32.0f, 2.0f * 32.0f, 32.0f, 32.0f };
                            DrawFloorTile(envSpritesheet, tileSrc, (Vector3){ px, 0.0f, pz }, (Vector2){ 1.0f, 1.0f }, WHITE);
                        }
                    }
                    
                    // --- 3. PARED LAYER (Compartment solid columns) ---
                    for (int z = 0; z < ROOM_GRID_SIZE; z++) {
                        for (int x = 0; x < ROOM_GRID_SIZE; x++) {
                            float px = (float)(x - 10);
                            float pz = (float)(z - 10);
                            
                            bool isBorder = (x == 0 || x == 20 || z == 0 || z == 20);
                            
                            if (isBorder) {
                                bool isDoorway = false;
                                if (z == 0 && x == 10 && room.doors[0]) isDoorway = true;
                                if (z == 20 && x == 10 && room.doors[1]) isDoorway = true;
                                if (x == 0 && z == 10 && room.doors[2]) isDoorway = true;
                                if (x == 20 && z == 10 && room.doors[3]) isDoorway = true;
                                
                                int wallCol = tileOffsetCol + room.wallTileVariants[z][x];
                                Rectangle wallSrc = { (float)wallCol * 32.0f, 0.0f, 32.0f, 32.0f };
                                
                                if (isDoorway) {
                                    if (!room.cleared) {
                                        // locked door (red cave block!)
                                        DrawWallBlock(envSpritesheet, wallSrc, (Vector3){ px, 2.0f, pz }, (Vector3){ 1.0f, 4.0f, 1.0f }, RED);
                                    } else {
                                        // open doorway floor tile path
                                        Rectangle openPathSrc = { (float)tileOffsetCol * 32.0f, 2.0f * 32.0f, 32.0f, 32.0f };
                                        DrawFloorTile(envSpritesheet, openPathSrc, (Vector3){ px, 0.0f, pz }, (Vector2){ 1.0f, 1.0f }, WHITE);
                                    }
                                } else {
                                    DrawWallBlock(envSpritesheet, wallSrc, (Vector3){ px, 2.0f, pz }, (Vector3){ 1.0f, 4.0f, 1.0f }, WHITE);
                                }
                            }
                        }
                    }
                    
                    for (int i = 0; i < room.numPillars; i++) {
                        Rectangle pillarSrc = { (float)tileOffsetCol * 32.0f, 0.0f, 32.0f, 32.0f };
                        DrawWallBlock(envSpritesheet, pillarSrc, room.pillars[i], (Vector3){ 1.6f, 4.0f, 1.6f }, WHITE);
                    }
                    
                    // Render Toxic Gas clouds on the ground flat
                    for (int i = 0; i < MAX_PARTICLES; i++) {
                        if (particles[i].active && particles[i].isGas) {
                            float alpha = particles[i].life / particles[i].maxLife;
                            DrawCircle3D(particles[i].position, 0.95f, (Vector3){ 1.0f, 0.0f, 0.0f }, 90.0f, Fade(particles[i].color, alpha * 0.7f));
                            DrawCircle3D(particles[i].position, 0.45f, (Vector3){ 1.0f, 0.0f, 0.0f }, 90.0f, Fade(WHITE, alpha * 0.4f));
                        }
                    }
                    
                    // --- 4. Z-SORTED ENTITIES GATHERING ---
                    billCount = 0;
                    
                    // Gather Room Decorations (moss, grass, flowers, mushrooms, beetles, fireflies)
                    for (int d = 0; d < room.numDecorations; d++) {
                        Decoration &dec = room.decorations[d];
                        if (!dec.active) continue;
                        
                        Rectangle src = { 0 };
                        Vector2 size = { 1.0f, 1.0f };
                        
                        if (dec.isInsect) {
                            int frame = (int)dec.animTimer % 3;
                            if (dec.isFly) {
                                // Firefly (row 6)
                                src = (Rectangle){ (float)frame * 32.0f, 6.0f * 32.0f, 32.0f, 32.0f };
                                size = (Vector2){ 0.7f, 0.7f };
                            } else {
                                // Beetle (row 5)
                                src = (Rectangle){ (float)frame * 32.0f, 5.0f * 32.0f, 32.0f, 32.0f };
                                size = (Vector2){ 0.65f, 0.65f };
                            }
                        } else {
                            // Flora (row 4)
                            src = (Rectangle){ (float)dec.type * 32.0f, 4.0f * 32.0f, 32.0f, 32.0f };
                            if (dec.type <= 1) { // Moss
                                size = (Vector2){ 1.3f, 1.3f };
                            } else if (dec.type == 7) { // Stalagmite
                                size = (Vector2){ 0.9f, 0.9f };
                            }
                        }
                        
                        Vector3 decRenderPos = dec.position;
                        if (dec.type <= 1) { // Moss lies very close to the ground
                            decRenderPos.y = 0.05f;
                        } else if (!dec.isInsect || !dec.isFly) {
                            decRenderPos.y = size.y / 2.0f; // stand up flora
                        }
                        
                        AddBillboardToRender(decRenderPos, envSpritesheet, src, size, Fade(WHITE, dec.alpha), 0, camera);
                    }
                    
                    // Ground items
                    for (int i = 0; i < room.numItems; i++) {
                        GroundItem &it = room.items[i];
                        if (it.active) {
                            it.animTimer += dt * 3.0f;
                            float bounceY = it.position.y + sinf(it.animTimer) * 0.15f;
                            Vector3 bouncePos = { it.position.x, bounceY, it.position.z };
                            Rectangle src = { (float)it.type * 32.0f, 7.0f * 32.0f, 32.0f, 32.0f };
                            AddBillboardToRender(bouncePos, charSpritesheet, src, (Vector2){ 1.1f, 1.1f }, WHITE, 1, camera);
                        }
                    }
                    
                    // Gather Projectiles
                    for (int i = 0; i < MAX_PROJECTILES; i++) {
                        if (projectiles[i].active) {
                            int col = projectiles[i].isEnemy ? 1 : projectiles[i].isAcid ? 2 : 0;
                            Rectangle src = { (float)col * 32.0f, 6.0f * 32.0f, 32.0f, 32.0f };
                            Vector2 sz = projectiles[i].isAcid ? (Vector2){ 1.3f, 1.3f } : (Vector2){ 0.8f, 0.8f };
                            AddBillboardToRender(projectiles[i].position, charSpritesheet, src, sz, WHITE, 1, camera);
                        }
                    }
                    
                    // Gather Impacts splash effects
                    for (int i = 0; i < MAX_IMPACTS; i++) {
                        if (impacts[i].active) {
                            Rectangle src = { (float)(3 + impacts[i].frame) * 32.0f, 6.0f * 32.0f, 32.0f, 32.0f };
                            AddBillboardToRender(impacts[i].position, charSpritesheet, src, (Vector2){ 1.3f, 1.3f }, WHITE, 1, camera);
                        }
                    }
                    
                    // Gather Enemies
                    for (int e = 0; e < room.numEnemies; e++) {
                        Entity &enemy = room.enemies[e];
                        if (enemy.health <= -50.0f) continue;
                        
                        Rectangle srcRec = { 0 };
                        Color tint = WHITE;
                        Vector2 size = enemy.isBoss ? (Vector2){ 3.2f, 3.2f } : (Vector2){ 1.8f, 1.8f };
                        
                        // Select sprite row offset based on enemy arquetype
                        int rowOffset = 3 + enemy.enemyType;
                        
                        if (enemy.state == STATE_DEAD) {
                            // explosion frame row 5
                            int dFrame = (int)(enemy.stateTimer / 0.1f);
                            if (dFrame > 3) dFrame = 3;
                            srcRec = (Rectangle){ (float)dFrame * 32.0f, 5.0f * 32.0f, 32.0f, 32.0f };
                        }
                        else if (enemy.state == STATE_ATTACK) {
                            int attFrame = 0;
                            if (enemy.stateTimer >= 0.25f && enemy.stateTimer < 0.4f) attFrame = 1;
                            else if (enemy.stateTimer >= 0.4f) attFrame = 2;
                            srcRec = (Rectangle){ (float)attFrame * 32.0f, (float)rowOffset * 32.0f, 32.0f, 32.0f };
                        }
                        else {
                            srcRec = (Rectangle){ (float)enemy.animFrame * 32.0f, (float)rowOffset * 32.0f, 32.0f, 32.0f };
                            if (enemy.state == STATE_HURT) tint = RED;
                        }
                        
                        Vector3 billPos = { enemy.position.x, enemy.position.y + (enemy.isBoss ? 0.3f : 0.1f), enemy.position.z };
                        AddBillboardToRender(billPos, charSpritesheet, srcRec, size, tint, 1, camera);
                    }
                    
                    // Gather Player (Stacked)
                    if (player.health > 0.0f || playerHalfHeartsHealth > 0) {
                        Color tint = (player.state == STATE_HURT) ? RED : WHITE;
                        
                        // 1. Legs
                        int legRow = (player.direction.z < 0.0f) ? 2 : 1;
                        Rectangle legSrc = { (float)player.animFrame * 32.0f, (float)legRow * 32.0f, 32.0f, 32.0f };
                        Vector3 legPos = { player.position.x, player.position.y - 0.2f, player.position.z };
                        AddBillboardToRender(legPos, charSpritesheet, legSrc, (Vector2){ 1.8f, 1.8f }, tint, 1, camera);
                        
                        // 2. Head
                        int headState = HEAD_LOOK_DOWN;
                        Vector3 aimDir = Vector3Subtract(groundAim, player.position);
                        aimDir.y = 0.0f;
                        bool flipHead = (aimDir.x > 0.0f);
                        bool isShooting = (player.state == STATE_ATTACK);
                        
                        if (aimDir.z < -0.5f) {
                            headState = isShooting ? HEAD_SHOOT_UP : HEAD_LOOK_UP;
                        } else if (fabsf(aimDir.x) > 0.5f) {
                            headState = isShooting ? HEAD_SHOOT_LEFT : HEAD_LOOK_LEFT;
                        } else {
                            headState = isShooting ? HEAD_SHOOT_DOWN : HEAD_LOOK_DOWN;
                        }
                        
                        Rectangle headSrc = { 
                            (float)headState * 32.0f, 
                            0.0f, 
                            flipHead ? -32.0f : 32.0f,
                            32.0f 
                        };
                        Vector3 headPos = { player.position.x, player.position.y + 0.6f, player.position.z };
                        AddBillboardToRender(headPos, charSpritesheet, headSrc, (Vector2){ 1.8f, 1.8f }, tint, 1, camera);
                    }
                    
                    // Z-Sort dynamically
                    SortRenderBillboards();
                    
                    // Draw sorted buffer (100% clean transparency overlaps!)
                    rlDisableDepthMask();
                    for (int i = 0; i < billCount; i++) {
                        DrawBillboardRec(camera, billBuffer[i].texture, billBuffer[i].source, billBuffer[i].position, billBuffer[i].size, billBuffer[i].tint);
                    }
                    rlEnableDepthMask();
                    
                    // Draw sparks falling on top (with perfect transparent alpha blending!)
                    rlDisableDepthMask();
                    for (int i = 0; i < MAX_PARTICLES; i++) {
                        if (particles[i].active && !particles[i].isGas) {
                            float alpha = particles[i].life / particles[i].maxLife;
                            DrawBillboardRec(camera, charSpritesheet, (Rectangle){ 128.0f, 192.0f, 32.0f, 32.0f }, particles[i].position, (Vector2){ 0.35f, 0.35f }, Fade(particles[i].color, alpha));
                        }
                    }
                    rlEnableDepthMask();
                    
                    // 3D reticle
                    DrawCircle3D(mouseIntersect, 0.4f, (Vector3){ 1.0f, 0.0f, 0.0f }, 90.0f, Fade(RED, 0.6f));
                    DrawCircle3D(mouseIntersect, 0.12f, (Vector3){ 1.0f, 0.0f, 0.0f }, 90.0f, RED);
                    
                EndMode3D();
                
                // --- 2D OVERLAYS & HUD UI ---
                // Enemy Screen space health bars
                for (int i = 0; i < room.numEnemies; i++) {
                    if (room.enemies[i].health <= 0) continue;
                    if (room.enemies[i].health < room.enemies[i].maxHealth) {
                        Vector3 worldPos = { room.enemies[i].position.x, room.enemies[i].position.y + (room.enemies[i].isBoss ? 1.8f : 1.1f), room.enemies[i].position.z };
                        Vector2 screenPos = GetWorldToScreen(worldPos, camera);
                        
                        int barW = room.enemies[i].isBoss ? 80 : 54;
                        int barH = room.enemies[i].isBoss ? 9 : 7;
                        DrawRectangle(screenPos.x - barW / 2 - 1, screenPos.y - barH / 2 - 1, barW + 2, barH + 2, BLACK);
                        DrawRectangle(screenPos.x - barW / 2, screenPos.y - barH / 2, barW, barH, MAROON);
                        float healthPerc = room.enemies[i].health / room.enemies[i].maxHealth;
                        DrawRectangle(screenPos.x - barW / 2, screenPos.y - barH / 2, (int)(barW * healthPerc), barH, room.enemies[i].isBoss ? RED : ORANGE);
                    }
                }
                
                // Active Upgrades HUD Icons
                int upgY = 120;
                if (hasCyberEye) {
                    DrawRectangle(40, upgY, 130, 24, Fade(BLACK, 0.6f));
                    DrawText("OJO CIBERNETICO", 46, upgY + 5, 12, LIME);
                    upgY += 30;
                }
                if (hasThrusterBoots) {
                    DrawRectangle(40, upgY, 130, 24, Fade(BLACK, 0.6f));
                    DrawText("BOTAS PROPULSORAS", 46, upgY + 5, 12, CYAN);
                    upgY += 30;
                }
                if (hasAcidGlands) {
                    DrawRectangle(40, upgY, 130, 24, Fade(BLACK, 0.6f));
                    DrawText("GLANDULAS ACIDO", 46, upgY + 5, 12, GREEN);
                }
                
                // Mini-map
                int mapX = screenWidth - 160;
                int mapY = 40;
                int roomSize = 22;
                int spacing = 6;
                DrawRectangle(mapX - 10, mapY - 10, 5 * (roomSize + spacing) + 14, 5 * (roomSize + spacing) + 14, Fade(BLACK, 0.6f));
                for (int ry = 0; ry < DUNGEON_SIZE; ry++) {
                    for (int rx = 0; rx < DUNGEON_SIZE; rx++) {
                        if (!dungeon[ry][rx].active) continue;
                        int rxPos = mapX + rx * (roomSize + spacing);
                        int ryPos = mapY + ry * (roomSize + spacing);
                        Color roomCol = (rx == currentRoomX && ry == currentRoomY) ? GOLD : dungeon[ry][rx].cleared ? GRAY : DARKGRAY;
                        if (dungeon[ry][rx].type == ROOM_BOSS) roomCol = RED;
                        else if (dungeon[ry][rx].type == ROOM_TREASURE) roomCol = PURPLE;
                        
                        DrawRectangle(rxPos, ryPos, roomSize, roomSize, roomCol);
                        DrawRectangleLines(rxPos, ryPos, roomSize, roomSize, WHITE);
                        if (dungeon[ry][rx].type == ROOM_START) DrawRectangle(rxPos + roomSize / 2 - 3, ryPos + roomSize / 2 - 3, 6, 6, GREEN);
                    }
                }
                
                // Draw heart UI energy containers
                DrawHeartUI(40, screenHeight - 65, playerHalfHeartsHealth, playerMaxHearts);
                
                DrawText(TextFormat("TIME: %.1fs", gameTimer), 40, 30, 22, GOLD);
                const char *typeStr = (room.type == ROOM_START) ? "COMPARTIMENTO DE ENTRADA (Seguro)" :
                                      (room.type == ROOM_TREASURE) ? "ZONA DE CARGA DEL TESORO" :
                                      (room.type == ROOM_BOSS) ? "¡NUCLEO DEL JEFE MUTANTE!" : "COMPARTIMENTO DE COMBATE";
                DrawText(typeStr, 40, 60, 18, CYAN);
                
                if (!room.cleared) DrawText("¡COMPARTIMENTOS SELLADOS! Purga la infestacion.", 40, 90, 16, RED);
                else DrawText("Zona purgada. Usa las compuertas WASD para avanzar.", 40, 90, 16, LIME);
                
                if (currentScreen == SCREEN_GAMEOVER) {
                    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));
                    DrawText("NUCLEO DE ENERGIA DESTRUIDO", screenWidth / 2 - MeasureText("NUCLEO DE ENERGIA DESTRUIDO", 40) / 2, screenHeight / 2 - 50, 40, RED);
                    DrawText("PRESIONA 'R' PARA REINTENTAR LA EXPLORACION", screenWidth / 2 - MeasureText("PRESIONA 'R' PARA REINTENTAR LA EXPLORACION", 20) / 2, screenHeight / 2 + 20, 20, RAYWHITE);
                }
                else if (currentScreen == SCREEN_VICTORY) {
                    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.85f));
                    DrawText("¡COMPARTIMENTO PURGADO Y NAVE SALVADA!", screenWidth / 2 - MeasureText("¡COMPARTIMENTO PURGADO Y NAVE SALVADA!", 40) / 2, screenHeight / 2 - 60, 40, LIME);
                    DrawText("Has escapado por la capsula del reactor nuclear del Jefe.", screenWidth / 2 - MeasureText("Has escapado por la capsula del reactor nuclear del Jefe.", 20) / 2, screenHeight / 2, 20, GOLD);
                    DrawText(TextFormat("TIEMPO FINAL: %.2fs", gameTimer), screenWidth / 2 - 80, screenHeight / 2 + 40, 20, WHITE);
                    DrawText("PRESIONA 'R' PARA REGENERAR UNA NUEVA NAVE PROCEDURAL", screenWidth / 2 - MeasureText("PRESIONA 'R' PARA REGENERAR UNA NUEVA NAVE PROCEDURAL", 20) / 2, screenHeight / 2 + 100, 20, RAYWHITE);
                }
            }
            
            DrawFPS(screenWidth - 80, screenHeight - 40);
            
        EndDrawing();
    }
    
    // --- CLEANUP ---
    UnloadModel(floorModel);
    UnloadModel(cubeModel);
    UnloadTexture(envSpritesheet);
    UnloadTexture(charSpritesheet);
    
    CloseWindow();
    return 0;
}
