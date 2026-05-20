#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_VORTICES 16

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
void SpawnParticles(Vector3 pos, Color color, int count, bool isGas = false);

const Color CYAN = (Color){ 0, 240, 240, 255 };
enum State { STATE_IDLE = 0, STATE_RUN, STATE_ATTACK, STATE_HURT, STATE_DEAD };
enum GameScreen { SCREEN_TITLE, SCREEN_NEXUS, SCREEN_INTRO, SCREEN_GAMEPLAY, SCREEN_GAMEOVER, SCREEN_VICTORY, SCREEN_ROOM_TRANSITION };
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

// --- PLANET & META-PROGRESSION STRUCTS ---
enum AtmosphericHazardType {
    HAZARD_NONE = 0,
    HAZARD_SOLAR_STORM = 1,
    HAZARD_TOXIC_FOG = 2,
    HAZARD_FROZEN_WASTE = 3
};

struct PlanetEnvironment {
    char name[64];
    float gravityMultiplier;      // 1.0f = standard gravity, <1.0f = low, >1.0f = high
    AtmosphericHazardType hazard;
    float hazardIntensity;        // Range 0.0f to 1.0f
    Color atmosphericTint;        // Camera viewport overlay color
    float groundFriction;         // X-Z movement friction coefficient
};

struct GreenhouseModule {
    int level;                      // 0 (Unbuilt) to 3
    float healingEfficiency;        // Scale: 1.0f to 1.50f
    float passiveHealOnRoomClear;   // Probability: 0.0f to 0.35f
    int herbYieldCount;
};

struct ArmoryModule {
    int level;                      // 0 to 3
    float baseDamageMultiplier;     // Scale: 1.0f to 1.45f
    int unlockedModSlots;           // 1, 2, or 3 slots
    bool hasQuantumTech;            // Quantum refraction enabled
};

struct EngineRoomModule {
    int level;                      // 0 to 3
    float travelRangeLightYears;    // Distance capability
    float fuelEfficiency;           // Travel resource consumption factor
    float gravityStabilization;     // Mitigates gravity-based speed penalty
};

struct MothershipSaveData {
    GreenhouseModule greenhouse;
    ArmoryModule armory;
    EngineRoomModule engineRoom;
    int isotopicResources;          // Core currency for upgrades
    int rescuedCrew;                // Rescued crew count
    int totalRunsCompleted;         // Run history count
    bool unlockedBounce;
    bool unlockedPiercing;
    bool unlockedQuantum;
    bool equippedBounce;
    bool equippedPiercing;
    bool equippedQuantum;
    unsigned int checksum;          // Verification key
};

struct CloneStatus {
    int cloneIndex;                 // Clone print number (1, 2, ...)
    float memoryCoherence;          // sanity percentage (0.0f to 100.0f)
    float paranoiaLevel;            // paranoia percentage (0.0f to 100.0f)
    int audioGlitchesCount;
    float synapicDegradationRate;   // degradation factor multiplier
};

void SaveCloneStatus(CloneStatus status, const char *filePath) {
    FILE *file = fopen(filePath, "wb");
    if (file != NULL) {
        fwrite(&status, sizeof(CloneStatus), 1, file);
        fclose(file);
    }
}

CloneStatus LoadCloneStatus(const char *filePath) {
    CloneStatus status = { 1, 100.0f, 0.0f, 0, 1.0f };
    FILE *file = fopen(filePath, "rb");
    if (file != NULL) {
        fread(&status, sizeof(CloneStatus), 1, file);
        fclose(file);
    }
    return status;
}

// --- WEAPON MODULES & SHADERS ---
struct ProjectileModuleData {
    int id;
    char name[32];
    float damageMod;
    float speedMultiplier;
    bool hasBounce;
    bool hasGravityPull;
};

struct TriggerModuleData {
    int id;
    char name[32];
    float cooldownMultiplier;
    int projectilesCount;
    float spreadAngleDegrees;
};

struct ModifierModuleData {
    int id;
    char name[32];
    float elementalDamage;
    bool hasPiercing;
    bool leavesTrail;
};

struct WeaponChassis {
    char name[32];
    float baseDamage;
    float baseCooldown;
    ProjectileModuleData projectileSlot;
    TriggerModuleData triggerSlot;
    ModifierModuleData modifierSlot;
    float currentCooldownTimer;
};

struct GravityVortex {
    Vector3 position;
    float radius;
    float pullForce;
    float damagePerSecond;
    float durationRemaining;
    bool active;
};

struct Relic {
    char name[64];
    char description[256];
    bool active;
    float bonusDamagePercent;
    float speedMultiplier;
    float maxHealthModifier;
    float sanityImpact;
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
    
    // Expanded physics properties
    float verticalVelocity;
    bool isGrounded;
    float suitIntegrity;
    float oxygenLevel;
    float hazardTimer;
    
    // Weapon module & Relic state slots
    WeaponChassis activeWeapon;
    bool relicEyeActive;
    bool relicHeartActive;
    bool relicBootsActive;
};

struct Projectile {
    Vector3 position;
    Vector3 direction;
    float speed;
    float radius;
    bool active;
    bool isEnemy;
    bool isAcid; // Acid glands poison bubble
    
    // Module effects
    bool hasBounce;
    bool hasPiercing;
    bool hasRefraction;
    int pierceCount;
};

struct Particle {
    Vector3 position;
    Vector3 velocity;
    Color color;
    float life;
    float maxLife;
    bool active;
    bool isGas; // Toxic gas cloud particle
    bool isAtmospheric;
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
    int flowField[ROOM_GRID_SIZE][ROOM_GRID_SIZE];
    
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
int selectedPlanetIdx = 0;
int activeCrewDialogIdx = 1;
int activeLeftTab = 0; // 0 = Modules, 1 = Weapon Customizer
int activeNexusOverlay = 0; // 0 = Walk Mode, 1 = Left Panel (Upgrades/Mods), 2 = Right Panel (Star Map)

// Upgrade states
bool hasCyberEye = false;
bool hasThrusterBoots = false;
bool hasAcidGlands = false;

// --- PLANET & SYSTEM GLOBALS ---
PlanetEnvironment currentPlanet = { 
    "CYON-IV (Planeta Helado)", 
    0.6f,                      // Low gravity
    HAZARD_FROZEN_WASTE, 
    0.5f,                      // Hazard intensity
    (Color){ 100, 180, 255, 40 }, 
    0.85f                      // Friction
};

void GenerateProceduralPlanet() {
    int randType = selectedPlanetIdx;
    if (randType == 3) {
        strcpy(currentPlanet.name, "CYON-IV (Planeta Helado)");
        currentPlanet.gravityMultiplier = 0.6f;
        currentPlanet.hazard = HAZARD_FROZEN_WASTE;
        currentPlanet.hazardIntensity = 0.4f + (float)GetRandomValue(0, 10) * 0.05f;
        currentPlanet.atmosphericTint = (Color){ 100, 180, 255, 30 };
        currentPlanet.groundFriction = 0.85f;
    } else if (randType == 1) {
        strcpy(currentPlanet.name, "SOLARIS-IX (Tormentas Solares)");
        currentPlanet.gravityMultiplier = 1.2f;
        currentPlanet.hazard = HAZARD_SOLAR_STORM;
        currentPlanet.hazardIntensity = 0.5f + (float)GetRandomValue(0, 10) * 0.04f;
        currentPlanet.atmosphericTint = (Color){ 255, 120, 0, 20 };
        currentPlanet.groundFriction = 0.95f;
    } else if (randType == 2) {
        strcpy(currentPlanet.name, "ZUL-GHAR (Niebla Toxica)");
        currentPlanet.gravityMultiplier = 0.9f;
        currentPlanet.hazard = HAZARD_TOXIC_FOG;
        currentPlanet.hazardIntensity = 0.4f + (float)GetRandomValue(0, 10) * 0.04f;
        currentPlanet.atmosphericTint = (Color){ 120, 255, 120, 25 };
        currentPlanet.groundFriction = 0.9f;
    } else {
        strcpy(currentPlanet.name, "AETHER (Gravedad Estandar)");
        currentPlanet.gravityMultiplier = 1.0f;
        currentPlanet.hazard = HAZARD_NONE;
        currentPlanet.hazardIntensity = 0.0f;
        currentPlanet.atmosphericTint = (Color){ 255, 255, 255, 0 };
        currentPlanet.groundFriction = 0.98f;
    }
}

MothershipSaveData motherShip = {
    {1, 1.0f, 0.0f, 0},        // Greenhouse level 1
    {1, 1.0f, 1, false},       // Armory level 1
    {1, 5.0f, 1.0f, 0.0f},     // Engine room level 1
    0, 0, 0,                   // Resources, crew, runs
    false, false, false,       // unlockedBounce, unlockedPiercing, unlockedQuantum
    false, false, false,       // equippedBounce, equippedPiercing, equippedQuantum
    0                          // checksum
};

CloneStatus currentClone = { 1, 100.0f, 0.0f, 0, 1.0f };
GravityVortex activeVortices[MAX_VORTICES] = { 0 };

Relic activeRelics[3] = {
    { "Ojo de la Nebulosa", "Doble dano critico, pero aberracion cromatica si tu salud baja de 30%.", false, 0.35f, 1.0f, 0.0f, 20.0f },
    { "Corazon de Enjambre", "Drones de plasma orbitales. Curacion recortada a la mitad.", false, 0.0f, 0.9f, 0.0f, 15.0f },
    { "Servomotores de Taquion", "Dashes continuos de velocidad. Desenfoque de movimiento persistente.", false, 0.0f, 1.4f, 0.0f, 25.0f }
};

// Shader source code strings compiled at runtime
const char* NebulaEyeShaderSource = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "out vec4 finalColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform float aberrationStrength;\n"
    "uniform float noiseIntensity;\n"
    "float rand(vec2 co) { return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }\n"
    "void main() {\n"
    "    vec2 uv = fragTexCoord;\n"
    "    vec4 rCol = texture(texture0, uv + vec2(aberrationStrength, 0.0));\n"
    "    vec4 gCol = texture(texture0, uv);\n"
    "    vec4 bCol = texture(texture0, uv - vec2(aberrationStrength, 0.0));\n"
    "    vec4 color = vec4(rCol.r, gCol.g, bCol.b, 1.0);\n"
    "    float noise = (rand(uv) - 0.5) * noiseIntensity;\n"
    "    finalColor = color + vec4(noise, noise, noise, 0.0);\n"
    "}\n";

const char* TachyonBootsShaderSource =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "out vec4 finalColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform float blurAmount;\n"
    "void main() {\n"
    "    vec2 uv = fragTexCoord;\n"
    "    vec4 color = vec4(0.0);\n"
    "    float total = 0.0;\n"
    "    for (float x = -3.0; x <= 3.0; x += 1.0) {\n"
    "        color += texture(texture0, uv + vec2(x * blurAmount, 0.0));\n"
    "        total += 1.0;\n"
    "    }\n"
    "    color /= total;\n"
    "    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));\n"
    "    finalColor = vec4(gray * 0.5, gray * 1.3, gray * 1.5, 1.0);\n"
    "}\n";

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
float introTimer = 0.0f;

// --- PERSISTENCE & UTILITY FUNCTIONS ---

unsigned int CalculateChecksum(const MothershipSaveData &data) {
    unsigned int hash = 5381;
    const unsigned char *ptr = (const unsigned char*)&data;
    size_t size = sizeof(MothershipSaveData) - sizeof(unsigned int);
    for (size_t i = 0; i < size; i++) {
        hash = ((hash << 5) + hash) + ptr[i];
    }
    return hash;
}

bool SaveMothershipState(const MothershipSaveData &state, const char *filePath) {
    MothershipSaveData dataToSave = state;
    dataToSave.checksum = CalculateChecksum(dataToSave);
    
    FILE *file = fopen(filePath, "wb");
    if (file == NULL) {
        printf("Error: No se pudo escribir la meta-progresion en %s\n", filePath);
        return false;
    }
    size_t written = fwrite(&dataToSave, sizeof(MothershipSaveData), 1, file);
    fclose(file);
    return (written == 1);
}

MothershipSaveData LoadMothershipState(const char *filePath) {
    MothershipSaveData loadedData;
    memset(&loadedData, 0, sizeof(MothershipSaveData));
    loadedData.greenhouse.level = 1;
    loadedData.greenhouse.healingEfficiency = 1.0f;
    loadedData.armory.level = 1;
    loadedData.armory.baseDamageMultiplier = 1.0f;
    loadedData.armory.unlockedModSlots = 1;
    loadedData.engineRoom.level = 1;
    loadedData.engineRoom.travelRangeLightYears = 5.0f;
    
    FILE *file = fopen(filePath, "rb");
    if (file == NULL) {
        loadedData.unlockedBounce = false;
        loadedData.unlockedPiercing = false;
        loadedData.unlockedQuantum = false;
        loadedData.equippedBounce = false;
        loadedData.equippedPiercing = false;
        loadedData.equippedQuantum = false;
        return loadedData;
    }
    size_t read = fread(&loadedData, sizeof(MothershipSaveData), 1, file);
    fclose(file);
    
    if (read == 1) {
        unsigned int computed = CalculateChecksum(loadedData);
        if (computed != loadedData.checksum) {
            printf("Aviso: Checksum corrupto en el savefile. Reseteando a nivel seguro.\n");
            memset(&loadedData, 0, sizeof(MothershipSaveData));
            loadedData.greenhouse.level = 1;
            loadedData.greenhouse.healingEfficiency = 1.0f;
            loadedData.armory.level = 1;
            loadedData.armory.baseDamageMultiplier = 1.0f;
            loadedData.armory.unlockedModSlots = 1;
            loadedData.engineRoom.level = 1;
            loadedData.engineRoom.travelRangeLightYears = 5.0f;
            loadedData.unlockedBounce = false;
            loadedData.unlockedPiercing = false;
            loadedData.unlockedQuantum = false;
            loadedData.equippedBounce = false;
            loadedData.equippedPiercing = false;
            loadedData.equippedQuantum = false;
        }
    }
    return loadedData;
}

// Gravity Vortex spawner & pull calculations
void SpawnGravityVortex(Vector3 pos, float radius, float pullForce, float dps, float duration) {
    for (int i = 0; i < MAX_VORTICES; i++) {
        if (!activeVortices[i].active) {
            activeVortices[i].position = pos;
            activeVortices[i].radius = radius;
            activeVortices[i].pullForce = pullForce;
            activeVortices[i].damagePerSecond = dps;
            activeVortices[i].durationRemaining = duration;
            activeVortices[i].active = true;
            break;
        }
    }
}

void UpdateGravityVortices(Entity *enemies, int numEnemies, float dt) {
    for (int v = 0; v < MAX_VORTICES; v++) {
        if (!activeVortices[v].active) continue;
        
        activeVortices[v].durationRemaining -= dt;
        if (activeVortices[v].durationRemaining <= 0.0f) {
            activeVortices[v].active = false;
            continue;
        }
        
        // Spawn orbital particles
        if (GetRandomValue(0, 100) < 40) {
            SpawnParticles(activeVortices[v].position, PURPLE, 2, true);
        }
        
        for (int e = 0; e < numEnemies; e++) {
            if (enemies[e].health <= 0.0f) continue;
            
            float dist = Vector3Distance(enemies[e].position, activeVortices[v].position);
            if (dist <= activeVortices[v].radius && dist > 0.15f) {
                Vector3 toVortex = Vector3Normalize(Vector3Subtract(activeVortices[v].position, enemies[e].position));
                float pullStrength = activeVortices[v].pullForce * (1.0f - (dist / activeVortices[v].radius));
                
                enemies[e].position = Vector3Add(enemies[e].position, Vector3Scale(toVortex, pullStrength * dt));
                if (activeVortices[v].damagePerSecond > 0.0f) {
                    enemies[e].health -= activeVortices[v].damagePerSecond * dt;
                }
            }
        }
    }
}

// Text Glitch drawing
void DrawTextGlitch(const char *text, int posX, int posY, int fontSize, Color color, float glitchIntensity) {
    char tempBuffer[256];
    strncpy(tempBuffer, text, sizeof(tempBuffer) - 1);
    tempBuffer[sizeof(tempBuffer) - 1] = '\0';
    
    int len = (int)strlen(tempBuffer);
    if (glitchIntensity > 0.1f) {
        for (int i = 0; i < len; i++) {
            if (tempBuffer[i] != ' ' && GetRandomValue(0, 100) < (int)(glitchIntensity * 22.0f)) {
                const char glitchPool[] = "0101#@$%&*!?[]<>/\\";
                tempBuffer[i] = glitchPool[GetRandomValue(0, sizeof(glitchPool) - 2)];
            }
        }
    }
    
    int offsetX = 0;
    int offsetY = 0;
    if (glitchIntensity > 0.35f) {
        offsetX = GetRandomValue(-4, 4) * (int)(glitchIntensity * 1.8f);
        offsetY = GetRandomValue(-4, 4) * (int)(glitchIntensity * 1.8f);
    }
    
    DrawText(tempBuffer, posX + offsetX, posY + offsetY, fontSize, color);
}

// Detailed HUD state
void DrawDehumanizerHUD(const CloneStatus &clone, int currentHealth, int maxHealth, int shieldValue, float timeSec) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    float glitchIntensity = 0.0f;
    Color hudColor = GREEN;
    
    if (clone.cloneIndex > 5 && clone.cloneIndex <= 20) {
        glitchIntensity = 0.25f;
        hudColor = (Color){ 220, 180, 50, 255 }; // Amber
    } else if (clone.cloneIndex > 20) {
        glitchIntensity = 0.85f;
        float pulse = sinf(timeSec * 8.0f) * 0.5f + 0.5f;
        hudColor = (Color){ (unsigned char)(180 + 75 * pulse), 10, 10, 255 }; // Pulsing Crimson
    }
    
    int panelY = 15;
    DrawRectangle(15, panelY, 320, 110, (Color){ 10, 10, 15, 220 });
    DrawRectangleLines(15, panelY, 320, 110, hudColor);
    
    char cloneLabel[64];
    sprintf(cloneLabel, "OPERADOR: CLON #%03d", clone.cloneIndex);
    DrawTextGlitch(cloneLabel, 30, panelY + 12, 18, hudColor, glitchIntensity);
    
    char cohLabel[64];
    sprintf(cohLabel, "COHERENCIA MEMORIA: %.1f%%", clone.memoryCoherence);
    DrawTextGlitch(cohLabel, 30, panelY + 38, 14, hudColor, glitchIntensity * 0.5f);
    
    DrawRectangle(30, panelY + 58, 280, 8, BLACK);
    DrawRectangle(30, panelY + 58, (int)(2.8f * (clone.memoryCoherence > 0.0f ? clone.memoryCoherence : 0.0f)), 8, hudColor);
    
    if (clone.cloneIndex > 20) {
        float alertPulse = sinf(timeSec * 14.0f);
        if (alertPulse > 0.0f) {
            DrawTextGlitch("ANOMALIA DE MEMORIA CRITICA", 30, panelY + 76, 11, RED, 0.9f);
        }
        
        int scanlineY = ((int)(timeSec * 250.0f)) % screenHeight;
        DrawLine(0, scanlineY, screenWidth, scanlineY, Fade(hudColor, 0.4f));
        DrawRectangle(0, scanlineY - 4, screenWidth, 8, Fade(hudColor, 0.15f));
    } else {
        DrawTextGlitch("INTEGRIDAD BIOLOGICA: CONECTADO", 30, panelY + 76, 11, hudColor, 0.0f);
    }
}

void SpawnParticles(Vector3 pos, Color color, int count, bool isGas) {
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
                particles[i].isAtmospheric = false;
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
        // Tolerance match for pure black or very dark colors (almost black, e.g. R, G, B all less than 32)
        bool isBlackKey = (pixels[i].r < 32 && pixels[i].g < 32 && pixels[i].b < 32);
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


// --- MAP GEN & TILE COLLISIONS ---
void CarveOrganicRoom(Room& room) {
    // 1. Initialize random grid
    for (int y = 0; y < ROOM_GRID_SIZE; y++) {
        for (int x = 0; x < ROOM_GRID_SIZE; x++) {
            if (x == 0 || x == ROOM_GRID_SIZE - 1 || y == 0 || y == ROOM_GRID_SIZE - 1) {
                room.wallTileVariants[y][x] = 1;
            } else {
                room.wallTileVariants[y][x] = (GetRandomValue(0, 100) < 42) ? 1 : 0;
            }
        }
    }
    
    // 2. Cellular Automata Smoothing (5 passes)
    for (int pass = 0; pass < 5; pass++) {
        int tempGrid[ROOM_GRID_SIZE][ROOM_GRID_SIZE];
        for (int y = 0; y < ROOM_GRID_SIZE; y++) {
            for (int x = 0; x < ROOM_GRID_SIZE; x++) {
                int wallNeighbors = 0;
                for (int ny = y - 1; ny <= y + 1; ny++) {
                    for (int nx = x - 1; nx <= x + 1; nx++) {
                        if (nx >= 0 && nx < ROOM_GRID_SIZE && ny >= 0 && ny < ROOM_GRID_SIZE) {
                            if (ny != y || nx != x) {
                                wallNeighbors += room.wallTileVariants[ny][nx];
                            }
                        } else {
                            wallNeighbors++; // out of bounds
                        }
                    }
                }
                
                if (room.wallTileVariants[y][x] == 1) {
                    tempGrid[y][x] = (wallNeighbors >= 4) ? 1 : 0;
                } else {
                    tempGrid[y][x] = (wallNeighbors >= 5) ? 1 : 0;
                }
            }
        }
        for (int y = 0; y < ROOM_GRID_SIZE; y++) {
            for (int x = 0; x < ROOM_GRID_SIZE; x++) {
                room.wallTileVariants[y][x] = tempGrid[y][x];
            }
        }
    }
    
    // 3. Clear paths and center
    for (int y = 9; y <= 11; y++) {
        for (int x = 9; x <= 11; x++) {
            room.wallTileVariants[y][x] = 0;
        }
    }
    if (room.doors[0]) { for(int y=0; y<3; y++) { room.wallTileVariants[y][10]=0; room.wallTileVariants[y][9]=0; room.wallTileVariants[y][11]=0; } } // Top
    if (room.doors[1]) { for(int y=ROOM_GRID_SIZE-3; y<ROOM_GRID_SIZE; y++) { room.wallTileVariants[y][10]=0; room.wallTileVariants[y][9]=0; room.wallTileVariants[y][11]=0; } } // Bottom
    if (room.doors[2]) { for(int x=0; x<3; x++) { room.wallTileVariants[10][x]=0; room.wallTileVariants[9][x]=0; room.wallTileVariants[11][x]=0; } } // Left
    if (room.doors[3]) { for(int x=ROOM_GRID_SIZE-3; x<ROOM_GRID_SIZE; x++) { room.wallTileVariants[10][x]=0; room.wallTileVariants[9][x]=0; room.wallTileVariants[11][x]=0; } } // Right
    
    // 4. Map to visuals
    for (int y = 0; y < ROOM_GRID_SIZE; y++) {
        for (int x = 0; x < ROOM_GRID_SIZE; x++) {
            if (room.wallTileVariants[y][x] > 0) {
                room.wallTileVariants[y][x] = GetRandomValue(0, 1);
                room.floorTileVariants[y][x] = 0;
            } else {
                room.wallTileVariants[y][x] = -1; // -1 means NO WALL!
                room.floorTileVariants[y][x] = GetRandomValue(0, 1);
            }
        }
    }
}

void ResolveTileCollisions(Vector3& pos, float radius, Room& currentRoom) {
    int gridX = (int)(pos.x + 10.5f);
    int gridZ = (int)(pos.z + 10.5f);
    
    for (int z = gridZ - 1; z <= gridZ + 1; z++) {
        for (int x = gridX - 1; x <= gridX + 1; x++) {
            if (x >= 0 && x < ROOM_GRID_SIZE && z >= 0 && z < ROOM_GRID_SIZE) {
                if (currentRoom.wallTileVariants[z][x] >= 0) {
                    float wallX = (float)(x - 10);
                    float wallZ = (float)(z - 10);
                    
                    float nearestX = fmaxf(wallX - 0.5f, fminf(pos.x, wallX + 0.5f));
                    float nearestZ = fmaxf(wallZ - 0.5f, fminf(pos.z, wallZ + 0.5f));
                    
                    float dx = pos.x - nearestX;
                    float dz = pos.z - nearestZ;
                    float distSq = dx * dx + dz * dz;
                    if (distSq < radius * radius && distSq > 0.001f) {
                        float dist = sqrtf(distSq);
                        float overlap = radius - dist;
                        pos.x += (dx / dist) * overlap;
                        pos.z += (dz / dist) * overlap;
                    }
                }
            }
        }
    }
}

bool CheckTileCollision(Vector3 pos, float radius, Room& currentRoom) {
    int gridX = (int)(pos.x + 10.5f);
    int gridZ = (int)(pos.z + 10.5f);
    for (int z = gridZ - 1; z <= gridZ + 1; z++) {
        for (int x = gridX - 1; x <= gridX + 1; x++) {
            if (x >= 0 && x < ROOM_GRID_SIZE && z >= 0 && z < ROOM_GRID_SIZE) {
                if (currentRoom.wallTileVariants[z][x] >= 0) {
                    float wallX = (float)(x - 10);
                    float wallZ = (float)(z - 10);
                    float nearestX = fmaxf(wallX - 0.5f, fminf(pos.x, wallX + 0.5f));
                    float nearestZ = fmaxf(wallZ - 0.5f, fminf(pos.z, wallZ + 0.5f));
                    float dx = pos.x - nearestX;
                    float dz = pos.z - nearestZ;
                    if (dx * dx + dz * dz < radius * radius) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
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

void AddBillboardToRender(Vector3 pos, Texture2D tex, Rectangle src, Vector2 sz, Color col, int layer, Camera3D camera, float depthOffset = 0.0f) {
    if (billCount >= MAX_RENDER_BILLBOARDS) return;
    
    Vector3 camToPos = Vector3Subtract(pos, camera.position);
    float dist = Vector3Length(camToPos);
    
    Vector3 renderPos = pos;
    if (layer == 1 && dist > 0.1f) {
        // Shift characters and dynamic objects slightly towards the camera to prevent them leaning back into walls/pillars
        Vector3 dir = Vector3Scale(camToPos, 1.0f / dist);
        renderPos = Vector3Subtract(pos, Vector3Scale(dir, 0.38f));
    }
    
    float depth = dist + depthOffset;
    
    billBuffer[billCount++] = { renderPos, tex, src, sz, col, depth, layer };
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

// --- STATE MACHINE (FSM) ---
#define MAX_STATE_STACK 10
GameScreen stateStack[MAX_STATE_STACK];
int stateStackTop = 0;

void InitStateStack() {
    stateStack[0] = SCREEN_TITLE;
    stateStackTop = 0;
}

void PushState(GameScreen screen) {
    if (stateStackTop < MAX_STATE_STACK - 1) {
        stateStackTop++;
        stateStack[stateStackTop] = screen;
    }
}

void PopState() {
    if (stateStackTop > 0) {
        stateStackTop--;
    }
}

void ChangeState(GameScreen screen) {
    stateStack[stateStackTop] = screen;
}

GameScreen GetCurrentState() {
    return stateStack[stateStackTop];
}

bool IsStateInStack(GameScreen screen) {
    for (int i = 0; i <= stateStackTop; i++) {
        if (stateStack[i] == screen) return true;
    }
    return false;
}

#define currentScreen GetCurrentState()


// --- AI FLOW FIELD NAVIGATION ---
void UpdateFlowField(Room& room, Vector3 targetPos) {
    // Reset flow field
    for (int y = 0; y < ROOM_GRID_SIZE; y++) {
        for (int x = 0; x < ROOM_GRID_SIZE; x++) {
            room.flowField[y][x] = 9999;
        }
    }
    
    int startX = (int)(targetPos.x + 10.5f);
    int startY = (int)(targetPos.z + 10.5f);
    if (startX < 0 || startX >= ROOM_GRID_SIZE || startY < 0 || startY >= ROOM_GRID_SIZE) return;
    
    struct Node { int x; int y; };
    Node queue[ROOM_GRID_SIZE * ROOM_GRID_SIZE];
    int qHead = 0, qTail = 0;
    
    queue[qTail++] = { startX, startY };
    room.flowField[startY][startX] = 0;
    
    int dx[] = { 0, 0, -1, 1, -1, 1, -1, 1 };
    int dy[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
    
    while (qHead < qTail) {
        Node curr = queue[qHead++];
        int dist = room.flowField[curr.y][curr.x];
        
        for (int i = 0; i < 8; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];
            
            if (nx >= 0 && nx < ROOM_GRID_SIZE && ny >= 0 && ny < ROOM_GRID_SIZE) {
                if (room.wallTileVariants[ny][nx] < 0) { // Walkable
                    int newDist = dist + (i < 4 ? 10 : 14); // 10 straight, 14 diagonal
                    if (newDist < room.flowField[ny][nx]) {
                        room.flowField[ny][nx] = newDist;
                        queue[qTail++] = { nx, ny };
                    }
                }
            }
        }
    }
}

Vector3 GetFlowFieldDirection(Room& room, Vector3 pos) {
    int x = (int)(pos.x + 10.5f);
    int y = (int)(pos.z + 10.5f);
    if (x < 0 || x >= ROOM_GRID_SIZE || y < 0 || y >= ROOM_GRID_SIZE) return {0,0,0};
    
    int dx[] = { 0, 0, -1, 1, -1, 1, -1, 1 };
    int dy[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
    
    int bestDist = room.flowField[y][x];
    int bestX = x;
    int bestY = y;
    
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < ROOM_GRID_SIZE && ny >= 0 && ny < ROOM_GRID_SIZE) {
            if (room.flowField[ny][nx] < bestDist) {
                bestDist = room.flowField[ny][nx];
                bestX = nx;
                bestY = ny;
            }
        }
    }
    
    if (bestX == x && bestY == y) return {0,0,0};
    
    Vector3 dir = { (float)(bestX - x), 0.0f, (float)(bestY - y) };
    return Vector3Normalize(dir);
}

int main(void) {
    int screenWidth = 1280;
    int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
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
    
    // Load Mothership state
    motherShip = LoadMothershipState("mothership.dat");
    currentClone = LoadCloneStatus("clone.dat");
    
    // Load postprocessing shaders
    Shader nebulaShader = LoadShaderFromMemory(NULL, NebulaEyeShaderSource);
    Shader tachyonShader = LoadShaderFromMemory(NULL, TachyonBootsShaderSource);
    
    int abLoc = GetShaderLocation(nebulaShader, "aberrationStrength");
    int noiseLoc = GetShaderLocation(nebulaShader, "noiseIntensity");
    int blurLoc = GetShaderLocation(tachyonShader, "blurAmount");
    
    RenderTexture2D targetTex = LoadRenderTexture(1280, 720);
    RenderTexture2D lightMap = LoadRenderTexture(1280, 720);
    
    float gameTimer = 0.0f;
    float screenShake = 0.0f;
    
    InitStateStack();

    auto ResetGame = [&]() {
        GenerateProceduralPlanet();
        
        motherShip.armory.baseDamageMultiplier = 1.0f + (motherShip.armory.level - 1) * 0.225f;
        motherShip.engineRoom.gravityStabilization = (motherShip.engineRoom.level == 1) ? 0.0f : (motherShip.engineRoom.level == 2) ? 0.15f : 0.40f;

        player.position = (Vector3){ 0.0f, 1.0f, 3.0f };
        player.radius = 0.5f;
        player.speed = 6.8f;
        player.state = STATE_IDLE;
        player.stateTimer = 0.0f;
        player.animTimer = 0.0f;
        player.animFrame = 0;
        
        player.verticalVelocity = 0.0f;
        player.isGrounded = true;
        player.suitIntegrity = 100.0f;
        player.oxygenLevel = 100.0f;
        player.hazardTimer = 0.0f;
        
        // Setup initial Weapon slots
        strcpy(player.activeWeapon.name, "Sagitario V1");
        player.activeWeapon.baseDamage = 15.0f;
        player.activeWeapon.baseCooldown = 0.15f;
        
        // Modules (Default + Equipped)
        player.activeWeapon.projectileSlot = (ProjectileModuleData){ 0, "Default Plasma", 0.0f, 1.0f, false, false };
        player.activeWeapon.triggerSlot = (TriggerModuleData){ 0, "Single-Fire", 1.0f, 1, 0.0f };
        player.activeWeapon.modifierSlot = (ModifierModuleData){ 0, "None", 0.0f, false, false };
        
        player.activeWeapon.projectileSlot.hasBounce = motherShip.equippedBounce;
        player.activeWeapon.modifierSlot.hasPiercing = motherShip.equippedPiercing;
        
        // Sinergia Agujero Negro: if bounce + piercing are both equipped, enable gravity pull!
        player.activeWeapon.projectileSlot.hasGravityPull = (motherShip.equippedBounce && motherShip.equippedPiercing);
        
        motherShip.armory.hasQuantumTech = motherShip.equippedQuantum;
        player.activeWeapon.currentCooldownTimer = 0.0f;
        
        // Relics status reset
        player.relicEyeActive = false;
        player.relicHeartActive = false;
        player.relicBootsActive = false;
        for (int r = 0; r < 3; r++) activeRelics[r].active = false;
        
        // Difficulty player setup
        if (selectedDifficulty == DIFF_EASY) {
            playerMaxHearts = 4 + (motherShip.greenhouse.level - 1);
            playerHearts = playerMaxHearts;
            playerHalfHeartsHealth = playerMaxHearts * 2;
        } else {
            playerMaxHearts = 3 + (motherShip.greenhouse.level - 1);
            playerHearts = playerMaxHearts;
            playerHalfHeartsHealth = playerMaxHearts * 2;
        }
        
        hasCyberEye = false;
        hasThrusterBoots = false;
        hasAcidGlands = false;
        
        for (int i = 0; i < MAX_PROJECTILES; i++) projectiles[i].active = false;
        for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;
        for (int i = 0; i < MAX_IMPACTS; i++) impacts[i].active = false;
        for (int i = 0; i < MAX_VORTICES; i++) activeVortices[i].active = false;
        
        GenerateProceduralDungeon();
        
        gameTimer = 0.0f;
        screenShake = 0.0f;
    };
    
    ResetGame();
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))) {
            ToggleFullscreen();
        }
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;
        
        if (currentScreen == SCREEN_TITLE || currentScreen == SCREEN_INTRO) {
            // Slowly rotate the camera around the origin for a cool cinematic space-drift effect
            float time = (float)GetTime() * 0.06f;
            camera.position = (Vector3){ sinf(time) * 16.0f, 11.0f, cosf(time) * 16.0f };
            camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
            
            if (currentScreen == SCREEN_TITLE) {
                if (IsKeyPressed(KEY_ONE)) selectedDifficulty = DIFF_EASY;
                if (IsKeyPressed(KEY_TWO)) selectedDifficulty = DIFF_NORMAL;
                if (IsKeyPressed(KEY_THREE)) selectedDifficulty = DIFF_HARD;
                
                Vector2 mousePos = GetMousePosition();
                
                // Menu buttons bounds (synchronized with rendering)
                int btnW = 380;
                int btnH = 34;
                int startY = screenHeight / 2 - 15;
                
                Rectangle btnEasy = { (float)(screenWidth / 2 - btnW / 2), (float)startY, (float)btnW, (float)btnH };
                Rectangle btnNorm = { (float)(screenWidth / 2 - btnW / 2), (float)(startY + 42), (float)btnW, (float)btnH };
                Rectangle btnHard = { (float)(screenWidth / 2 - btnW / 2), (float)(startY + 84), (float)btnW, (float)btnH };
                Rectangle btnStart = { (float)(screenWidth / 2 - 160), (float)(startY + 150), 320.0f, 45.0f };
                
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (CheckCollisionPointRec(mousePos, btnEasy)) selectedDifficulty = DIFF_EASY;
                    if (CheckCollisionPointRec(mousePos, btnNorm)) selectedDifficulty = DIFF_NORMAL;
                    if (CheckCollisionPointRec(mousePos, btnHard)) selectedDifficulty = DIFF_HARD;
                    if (CheckCollisionPointRec(mousePos, btnStart)) {
                        ChangeState(SCREEN_NEXUS);
                    }
                }
                
                if (IsKeyPressed(KEY_ENTER)) {
                    ChangeState(SCREEN_NEXUS);
                }
            }
            else { // SCREEN_INTRO
                introTimer += dt;
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    ResetGame();
                    ChangeState(SCREEN_GAMEPLAY);
                }
            }
        }
        else if (currentScreen == SCREEN_NEXUS) {
            Vector2 mousePos = GetMousePosition();
            
            // Layout bounds for mouse buttons
            int panelX = 40;
            int startY = 110;
            int btnW = 120;
            int btnH = 30;
            
            Rectangle btnGreenhouse = { (float)(panelX + 270), (float)(startY + 55), (float)btnW, (float)btnH };
            Rectangle btnArmory = { (float)(panelX + 270), (float)(startY + 165), (float)btnW, (float)btnH };
            Rectangle btnEngine = { (float)(panelX + 270), (float)(startY + 275), (float)btnW, (float)btnH };
            
            int mapX = screenWidth - 440;
            int planetStartY = 110;
            Rectangle btnPlanets[4];
            for (int p = 0; p < 4; p++) {
                btnPlanets[p] = (Rectangle){ (float)(mapX + 20), (float)(planetStartY + 45 + p * 34), 360.0f, 28.0f };
            }
            
            Rectangle btnLaunch = { (float)(screenWidth / 2 - 200), (float)(screenHeight - 85), 400.0f, 50.0f };
            
            Rectangle btnTabModules = { (float)panelX, (float)(startY - 32), 150.0f, 30.0f };
            Rectangle btnTabWeapon = { (float)(panelX + 155), (float)(startY - 32), 150.0f, 30.0f };
            Rectangle btnBounce = { (float)(panelX + 270), (float)(startY + 65), 120.0f, 30.0f };
            Rectangle btnPiercing = { (float)(panelX + 270), (float)(startY + 175), 120.0f, 30.0f };
            Rectangle btnQuantum = { (float)(panelX + 270), (float)(startY + 285), 120.0f, 30.0f };
            
            // Positions of interactive objects on the Bridge
            Vector3 greenhousePos = { -4.5f, 1.0f, -2.0f };
            Vector3 armoryPos = { 4.5f, 1.0f, -2.0f };
            Vector3 navigationPos = { 0.0f, 1.0f, -4.5f };
            Vector3 iaPos = { 0.0f, 1.0f, -1.0f };
            Vector3 sciNPC_Pos = { -3.0f, 1.0f, 2.0f };
            Vector3 soldNPC_Pos = { 3.0f, 1.0f, 2.0f };
            
            // Check distances
            float distGreenhouse = Vector3Distance(player.position, greenhousePos);
            float distArmory = Vector3Distance(player.position, armoryPos);
            float distNavigation = Vector3Distance(player.position, navigationPos);
            float distIA = Vector3Distance(player.position, iaPos);
            float distSci = Vector3Distance(player.position, sciNPC_Pos);
            float distSold = Vector3Distance(player.position, soldNPC_Pos);
            
            if (activeNexusOverlay == 0) {
                // 1. WASD Player movement inside the Bridge
                Vector3 moveVector = { 0 };
                if (IsKeyDown(KEY_W)) moveVector.z -= 1.0f;
                if (IsKeyDown(KEY_S)) moveVector.z += 1.0f;
                if (IsKeyDown(KEY_A)) moveVector.x -= 1.0f;
                if (IsKeyDown(KEY_D)) moveVector.x += 1.0f;
                
                if (Vector3Length(moveVector) > 0.0f) {
                    moveVector = Vector3Normalize(moveVector);
                    player.position = Vector3Add(player.position, Vector3Scale(moveVector, 5.2f * dt));
                    player.state = STATE_RUN;
                    player.animTimer += dt * 10.0f;
                    player.animFrame = ((int)player.animTimer) % 4;
                    player.direction = moveVector;
                } else {
                    player.state = STATE_IDLE;
                    player.animFrame = 0;
                }
                
                // Keep player inside boundary
                if (player.position.x < -6.0f) player.position.x = -6.0f;
                if (player.position.x > 6.0f) player.position.x = 6.0f;
                if (player.position.z < -6.0f) player.position.z = -6.0f;
                if (player.position.z > 6.0f) player.position.z = 6.0f;
                
                // Camera follows player on the Bridge
                Vector3 targetCam = { player.position.x, 9.5f, player.position.z + 10.0f };
                camera.position = Vector3Lerp(camera.position, targetCam, 5.0f * dt);
                camera.target = Vector3Lerp(camera.target, player.position, 8.0f * dt);
                
                // Interaction trigger
                if (IsKeyPressed(KEY_E)) {
                    if (distGreenhouse < 2.0f) {
                        activeNexusOverlay = 1;
                        activeLeftTab = 0;
                    } else if (distArmory < 2.0f) {
                        activeNexusOverlay = 1;
                        activeLeftTab = 1;
                    } else if (distNavigation < 2.0f) {
                        activeNexusOverlay = 2;
                    } else if (distIA < 1.8f) {
                        activeCrewDialogIdx = 1;
                    } else if (distSci < 1.8f) {
                        activeCrewDialogIdx = 2;
                    } else if (distSold < 1.8f) {
                        activeCrewDialogIdx = 3;
                    }
                }
            } else {
                // If menu is open, handle closing
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_E)) {
                    activeNexusOverlay = 0;
                }
                
                // Keep camera focused on console/screen while operating
                Vector3 consoleCamTarget = player.position;
                if (activeNexusOverlay == 1) {
                    consoleCamTarget = (activeLeftTab == 0) ? greenhousePos : armoryPos;
                } else if (activeNexusOverlay == 2) {
                    consoleCamTarget = navigationPos;
                }
                Vector3 targetCam = { consoleCamTarget.x, 8.0f, consoleCamTarget.z + 8.5f };
                camera.position = Vector3Lerp(camera.position, targetCam, 5.0f * dt);
                camera.target = Vector3Lerp(camera.target, consoleCamTarget, 8.0f * dt);
                
                // Handle click checks
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (activeNexusOverlay == 1) {
                        // Tab Selection
                        if (CheckCollisionPointRec(mousePos, btnTabModules)) activeLeftTab = 0;
                        if (CheckCollisionPointRec(mousePos, btnTabWeapon)) activeLeftTab = 1;
                        
                        if (activeLeftTab == 0) {
                            // 1. Upgrade Greenhouse
                            if (CheckCollisionPointRec(mousePos, btnGreenhouse)) {
                                if (motherShip.greenhouse.level < 3) {
                                    int cost = (motherShip.greenhouse.level == 1) ? 180 : 500;
                                    int reqCrew = (motherShip.greenhouse.level == 1) ? 2 : 6;
                                    if (motherShip.isotopicResources >= cost && motherShip.rescuedCrew >= reqCrew) {
                                        motherShip.isotopicResources -= cost;
                                        motherShip.greenhouse.level++;
                                        SaveMothershipState(motherShip, "mothership.dat");
                                    }
                                }
                            }
                            
                            // 2. Upgrade Armory
                            if (CheckCollisionPointRec(mousePos, btnArmory)) {
                                if (motherShip.armory.level < 3) {
                                    int cost = (motherShip.armory.level == 1) ? 250 : 600;
                                    int reqCrew = (motherShip.armory.level == 1) ? 3 : 8;
                                    if (motherShip.isotopicResources >= cost && motherShip.rescuedCrew >= reqCrew) {
                                        motherShip.isotopicResources -= cost;
                                        motherShip.armory.level++;
                                        SaveMothershipState(motherShip, "mothership.dat");
                                    }
                                }
                            }
                            
                            // 3. Upgrade Engine Room
                            if (CheckCollisionPointRec(mousePos, btnEngine)) {
                                if (motherShip.engineRoom.level < 3) {
                                    int cost = (motherShip.engineRoom.level == 1) ? 150 : 400;
                                    int reqCrew = (motherShip.engineRoom.level == 1) ? 2 : 5;
                                    if (motherShip.isotopicResources >= cost && motherShip.rescuedCrew >= reqCrew) {
                                        motherShip.isotopicResources -= cost;
                                        motherShip.engineRoom.level++;
                                        SaveMothershipState(motherShip, "mothership.dat");
                                    }
                                }
                            }
                        }
                        else { // activeLeftTab == 1 (Weapon Modules Customize)
                            int maxSlots = motherShip.armory.level;
                            int currentlyEquipped = (motherShip.equippedBounce ? 1 : 0) + 
                                                     (motherShip.equippedPiercing ? 1 : 0) + 
                                                     (motherShip.equippedQuantum ? 1 : 0);
                                                     
                            // Bounce module
                            if (CheckCollisionPointRec(mousePos, btnBounce)) {
                                if (!motherShip.unlockedBounce) {
                                    if (motherShip.isotopicResources >= 100) {
                                        motherShip.isotopicResources -= 100;
                                        motherShip.unlockedBounce = true;
                                        SaveMothershipState(motherShip, "mothership.dat");
                                    }
                                } else {
                                    if (motherShip.equippedBounce) {
                                        motherShip.equippedBounce = false;
                                        SaveMothershipState(motherShip, "mothership.dat");
                                    } else {
                                        if (currentlyEquipped < maxSlots) {
                                            motherShip.equippedBounce = true;
                                            SaveMothershipState(motherShip, "mothership.dat");
                                        }
                                    }
                                }
                            }
                            
                            // Piercing module
                            if (CheckCollisionPointRec(mousePos, btnPiercing)) {
                                if (!motherShip.unlockedPiercing) {
                                    if (motherShip.isotopicResources >= 150) {
                                        motherShip.isotopicResources -= 150;
                                        motherShip.unlockedPiercing = true;
                                        SaveMothershipState(motherShip, "mothership.dat");
                                    }
                                } else {
                                    if (motherShip.equippedPiercing) {
                                        motherShip.equippedPiercing = false;
                                        SaveMothershipState(motherShip, "mothership.dat");
                                    } else {
                                        if (currentlyEquipped < maxSlots) {
                                            motherShip.equippedPiercing = true;
                                            SaveMothershipState(motherShip, "mothership.dat");
                                        }
                                    }
                                }
                            }
                            
                            // Quantum module
                            if (CheckCollisionPointRec(mousePos, btnQuantum)) {
                                if (motherShip.armory.level >= 3) {
                                    if (!motherShip.unlockedQuantum) {
                                        if (motherShip.isotopicResources >= 250) {
                                            motherShip.isotopicResources -= 250;
                                            motherShip.unlockedQuantum = true;
                                            SaveMothershipState(motherShip, "mothership.dat");
                                        }
                                    } else {
                                        if (motherShip.equippedQuantum) {
                                            motherShip.equippedQuantum = false;
                                            SaveMothershipState(motherShip, "mothership.dat");
                                        } else {
                                            if (currentlyEquipped < maxSlots) {
                                                motherShip.equippedQuantum = true;
                                                SaveMothershipState(motherShip, "mothership.dat");
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (activeNexusOverlay == 2) {
                        // Select Planet
                        for (int p = 0; p < 4; p++) {
                            if (CheckCollisionPointRec(mousePos, btnPlanets[p])) {
                                bool canTravel = false;
                                if (p == 0 || p == 1) canTravel = (motherShip.engineRoom.level >= 1);
                                else if (p == 2) canTravel = (motherShip.engineRoom.level >= 2);
                                else if (p == 3) canTravel = (motherShip.engineRoom.level >= 3);
                                
                                if (canTravel) {
                                    selectedPlanetIdx = p;
                                }
                            }
                        }
                        
                        // Launch Sequence!
                        if (CheckCollisionPointRec(mousePos, btnLaunch)) {
                            activeNexusOverlay = 0;
                            ChangeState(SCREEN_INTRO);
                            introTimer = 0.0f;
                        }
                    }
                }
            }
        }
        else if (currentScreen == SCREEN_GAMEOVER || currentScreen == SCREEN_VICTORY) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_R) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                ChangeState(SCREEN_NEXUS);
            }
        }
        else if (currentScreen == SCREEN_ROOM_TRANSITION) {
            float oldTimer = transitionTimer;
            transitionTimer += dt * 2.0f;
            if (oldTimer < 0.5f && transitionTimer >= 0.5f) {
                currentRoomX = nextRoomX;
                currentRoomY = nextRoomY;
            }
            if (transitionTimer >= 1.0f) {
                transitionTimer = 1.0f;
                currentRoomX = nextRoomX;
                currentRoomY = nextRoomY;
                player.position = transitionPlayerEnd;
                ChangeState(SCREEN_GAMEPLAY);
            } else {
                player.position = Vector3Lerp(transitionPlayerStart, transitionPlayerEnd, transitionTimer);
            }
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            gameTimer += dt;
            
            // Spawn atmospheric particles automatically
            if (currentPlanet.hazard != HAZARD_NONE && GetRandomValue(1, 100) < 18) {
                Vector3 spawnPos = player.position;
                spawnPos.x += (float)GetRandomValue(-150, 150) * 0.1f;
                spawnPos.z += (float)GetRandomValue(-150, 150) * 0.1f;
                
                Color partCol = WHITE;
                Vector3 partVel = { 0.0f, 0.0f, 0.0f };
                float maxLife = (float)GetRandomValue(120, 280) * 0.01f;
                
                if (currentPlanet.hazard == HAZARD_SOLAR_STORM) {
                    // Ash/Sparks rising
                    spawnPos.y = 0.1f;
                    partCol = (GetRandomValue(0, 1) == 0) ? ORANGE : RED;
                    partVel = (Vector3){
                        (float)GetRandomValue(-15, 15) * 0.01f,
                        (float)GetRandomValue(10, 30) * 0.02f,
                        (float)GetRandomValue(-15, 15) * 0.01f
                    };
                } else if (currentPlanet.hazard == HAZARD_TOXIC_FOG) {
                    // Floating spores
                    spawnPos.y = (float)GetRandomValue(5, 35) * 0.1f;
                    partCol = LIME;
                    partVel = (Vector3){
                        (float)GetRandomValue(-10, 10) * 0.01f,
                        (float)GetRandomValue(-8, 8) * 0.01f,
                        (float)GetRandomValue(-10, 10) * 0.01f
                    };
                } else if (currentPlanet.hazard == HAZARD_FROZEN_WASTE) {
                    // Snow falling
                    spawnPos.y = 5.0f;
                    partCol = (GetRandomValue(0, 1) == 0) ? WHITE : SKYBLUE;
                    partVel = (Vector3){
                        (float)GetRandomValue(-15, 15) * 0.01f,
                        (float)GetRandomValue(-25, -12) * 0.02f,
                        (float)GetRandomValue(-15, 15) * 0.01f
                    };
                }
                
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (!particles[i].active) {
                        particles[i].position = spawnPos;
                        particles[i].velocity = partVel;
                        particles[i].color = partCol;
                        particles[i].life = maxLife;
                        particles[i].maxLife = maxLife;
                        particles[i].isGas = false;
                        particles[i].isAtmospheric = true;
                        particles[i].active = true;
                        break;
                    }
                }
            }
            
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
                if (!currentRoom.cleared) {
                    currentRoom.cleared = true;
                    // Passive greenhouse healing
                    float passiveHealChance = 0.0f;
                    if (motherShip.greenhouse.level == 2) passiveHealChance = 0.15f;
                    else if (motherShip.greenhouse.level == 3) passiveHealChance = 0.35f;
                    
                    if (passiveHealChance > 0.0f && ((float)GetRandomValue(0, 100) / 100.0f) < passiveHealChance) {
                        playerHalfHeartsHealth += 1; // heal half heart
                        if (playerHalfHeartsHealth > playerMaxHearts * 2) {
                            playerHalfHeartsHealth = playerMaxHearts * 2;
                        }
                        SpawnParticles(player.position, GREEN, 8);
                    }
                    
                    // Earn resources on room clearance
                    int isotopicEarned = GetRandomValue(5, 15);
                    motherShip.isotopicResources += isotopicEarned;
                    printf("Habitacion purgada: +%d Isotopos (Total: %d)\n", isotopicEarned, motherShip.isotopicResources);
                    SaveMothershipState(motherShip, "mothership.dat");
                }
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
                            int healAmt = (int)(2.0f * (motherShip.greenhouse.level == 1 ? 1.0f : motherShip.greenhouse.level == 2 ? 1.30f : 1.50f));
                            if (activeRelics[1].active) {
                                healAmt = (int)(healAmt * 0.5f);
                                if (healAmt < 1) healAmt = 1;
                            }
                            playerHalfHeartsHealth += healAmt;
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
            
            // --- TESTING & UPGRADE CONTROLLER KEYS ---
            if (IsKeyPressed(KEY_U)) {
                motherShip.greenhouse.level = (motherShip.greenhouse.level % 3) + 1;
                playerMaxHearts = 3 + (motherShip.greenhouse.level - 1);
                playerHalfHeartsHealth = playerHearts * 2;
                printf("[TEST] Invernadero nivel: %d. Corazones maximos actualizados a %d\n", motherShip.greenhouse.level, playerMaxHearts);
                SaveMothershipState(motherShip, "mothership.dat");
            }
            if (IsKeyPressed(KEY_I)) {
                motherShip.armory.level = (motherShip.armory.level % 3) + 1;
                motherShip.armory.baseDamageMultiplier = 1.0f + (motherShip.armory.level - 1) * 0.15f;
                printf("[TEST] Armeria nivel: %d. Multiplicador de dano a %.2f\n", motherShip.armory.level, motherShip.armory.baseDamageMultiplier);
                SaveMothershipState(motherShip, "mothership.dat");
            }
            if (IsKeyPressed(KEY_O)) {
                motherShip.engineRoom.level = (motherShip.engineRoom.level % 3) + 1;
                motherShip.engineRoom.gravityStabilization = 1.0f - (motherShip.engineRoom.level - 1) * 0.45f;
                printf("[TEST] Sala de Motores nivel: %d. Estabilizacion gravedad a %.2f\n", motherShip.engineRoom.level, motherShip.engineRoom.gravityStabilization);
                SaveMothershipState(motherShip, "mothership.dat");
            }
            if (IsKeyPressed(KEY_ONE)) {
                activeRelics[0].active = !activeRelics[0].active;
                player.relicEyeActive = activeRelics[0].active;
                printf("[TEST] Ojo de la Nebulosa relic: %s\n", activeRelics[0].active ? "ACTIVADA" : "DESACTIVADA");
            }
            if (IsKeyPressed(KEY_TWO)) {
                activeRelics[1].active = !activeRelics[1].active;
                player.relicHeartActive = activeRelics[1].active;
                printf("[TEST] Corazon de Enjambre relic: %s\n", activeRelics[1].active ? "ACTIVADA" : "DESACTIVADA");
            }
            if (IsKeyPressed(KEY_THREE)) {
                activeRelics[2].active = !activeRelics[2].active;
                player.relicBootsActive = activeRelics[2].active;
                printf("[TEST] Servomotores de Taquion relic: %s\n", activeRelics[2].active ? "ACTIVADA" : "DESACTIVADA");
            }
            if (IsKeyPressed(KEY_G)) {
                if (player.activeWeapon.projectileSlot.hasBounce) {
                    player.activeWeapon.projectileSlot.hasBounce = false;
                    player.activeWeapon.modifierSlot.hasPiercing = false;
                    player.activeWeapon.projectileSlot.hasGravityPull = false;
                    printf("[TEST] Modulos de Arma removidos.\n");
                } else {
                    player.activeWeapon.projectileSlot.hasBounce = true;
                    player.activeWeapon.modifierSlot.hasPiercing = true;
                    player.activeWeapon.projectileSlot.hasGravityPull = true;
                    printf("[TEST] Modulos de Arma equipados: Rebote Gravitatorio + Perforacion de Plasma (SINERGIA AGUJERO NEGRO ACTIVADA).\n");
                }
            }
            if (IsKeyPressed(KEY_H)) {
                motherShip.isotopicResources += 100;
                printf("[TEST] +100 Recursos Isotopicos (Total: %d)\n", motherShip.isotopicResources);
                SaveMothershipState(motherShip, "mothership.dat");
            }
            
            // --- SUIT AND ATMOSPHERIC HAZARD LOGIC ---
            if (currentPlanet.hazard != HAZARD_NONE && currentRoom.type != ROOM_START) {
                player.hazardTimer += dt;
                if (player.hazardTimer >= 1.0f) {
                    player.hazardTimer = 0.0f;
                    
                    float drainAmount = currentPlanet.hazardIntensity * 1.5f;
                    // Low coherence/high clones increase drainage
                    drainAmount *= (1.0f + (currentClone.cloneIndex - 1) * 0.05f);
                    
                    player.suitIntegrity -= drainAmount;
                    if (player.suitIntegrity < 0.0f) {
                        player.suitIntegrity = 0.0f;
                        player.oxygenLevel -= drainAmount * 1.8f;
                        
                        if (player.oxygenLevel <= 0.0f) {
                            player.oxygenLevel = 0.0f;
                            // Deal damage directly to player health (half heart at a time)
                            playerHalfHeartsHealth -= 1;
                            player.state = STATE_HURT;
                            player.stateTimer = 0.15f;
                            screenShake = 0.2f;
                            SpawnParticles(player.position, RED, 5);
                            if (playerHalfHeartsHealth <= 0) ChangeState(SCREEN_GAMEOVER);
                        }
                    }
                }
            }
            
            // Update gravity vortices
            UpdateGravityVortices(currentRoom.enemies, currentRoom.numEnemies, dt);
            
            // --- WASD PLAYER CONTROLLER & GRAVITY ---
            Vector3 moveVector = { 0 };
            if (IsKeyDown(KEY_W)) moveVector.z -= 1.0f;
            if (IsKeyDown(KEY_S)) moveVector.z += 1.0f;
            if (IsKeyDown(KEY_A)) moveVector.x -= 1.0f;
            if (IsKeyDown(KEY_D)) moveVector.x += 1.0f;
            
            // Speed modifications based on gravity, boots, ground friction and relics
            float baseSpeed = 6.8f;
            if (hasThrusterBoots) baseSpeed = 9.2f;
            
            float relicSpeedMult = 1.0f;
            for (int r = 0; r < 3; r++) {
                if (activeRelics[r].active) {
                    relicSpeedMult *= activeRelics[r].speedMultiplier;
                }
            }
            
            float gravitySpeedPenalty = 1.0f;
            if (currentPlanet.gravityMultiplier > 1.0f) {
                float penalty = (currentPlanet.gravityMultiplier - 1.0f) * 0.3f;
                float stabilization = motherShip.engineRoom.gravityStabilization;
                if (motherShip.engineRoom.level == 2) stabilization = 0.5f;
                else if (motherShip.engineRoom.level == 3) stabilization = 0.1f;
                penalty *= stabilization;
                gravitySpeedPenalty = 1.0f - penalty;
                if (gravitySpeedPenalty < 0.2f) gravitySpeedPenalty = 0.2f;
            }
            
            player.speed = baseSpeed * relicSpeedMult * gravitySpeedPenalty * currentPlanet.groundFriction;
            
            // Jumping vertical movement (3D vertical jump)
            float jumpForce = 9.8f / (currentPlanet.gravityMultiplier > 0.1f ? sqrtf(currentPlanet.gravityMultiplier) : 0.3f);
            
            if (IsKeyPressed(KEY_SPACE) && player.isGrounded) {
                player.verticalVelocity = jumpForce;
                player.isGrounded = false;
            }
            
            if (!player.isGrounded) {
                float gravityAccel = 9.81f * currentPlanet.gravityMultiplier;
                player.verticalVelocity -= gravityAccel * dt;
                player.position.y += player.verticalVelocity * dt;
                
                if (player.position.y <= 1.0f) {
                    player.position.y = 1.0f;
                    player.verticalVelocity = 0.0f;
                    player.isGrounded = true;
                }
            }
            
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
                    ChangeState(SCREEN_ROOM_TRANSITION);
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
                    ChangeState(SCREEN_ROOM_TRANSITION);
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
                    ChangeState(SCREEN_ROOM_TRANSITION);
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
                    ChangeState(SCREEN_ROOM_TRANSITION);
                    for (int i = 0; i < MAX_PROJECTILES; i++) projectiles[i].active = false;
                } else player.position.x = wallRight;
            }
            
            // Escape Trapdoor
            if (currentRoom.type == ROOM_BOSS && currentRoom.cleared) {
                float dx = player.position.x;
                float dz = player.position.z;
                if (dx * dx + dz * dz < 1.0f) {
                    ChangeState(SCREEN_VICTORY);
                }
            }
            
            ResolveTileCollisions(player.position, player.radius, currentRoom);
            
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
                        
                        if (playerHalfHeartsHealth <= 0) ChangeState(SCREEN_GAMEOVER);
                    }
                }
            }
            
            // Cooldown update
            if (player.activeWeapon.currentCooldownTimer > 0.0f) {
                player.activeWeapon.currentCooldownTimer -= dt;
            }
            
            // --- SWARM HEART DRONES ---
            static float droneAttackTimer = 0.0f;
            if (activeRelics[1].active) {
                droneAttackTimer += dt;
                if (droneAttackTimer >= 0.8f) {
                    droneAttackTimer = 0.0f;
                    
                    float closestDistSq = 9999.0f;
                    int targetEnemyIdx = -1;
                    for (int e = 0; e < currentRoom.numEnemies; e++) {
                        if (currentRoom.enemies[e].health > 0.0f) {
                            float dx = currentRoom.enemies[e].position.x - player.position.x;
                            float dz = currentRoom.enemies[e].position.z - player.position.z;
                            float distSq = dx * dx + dz * dz;
                            if (distSq < closestDistSq) {
                                closestDistSq = distSq;
                                targetEnemyIdx = e;
                            }
                        }
                    }
                    
                    if (targetEnemyIdx != -1) {
                        Vector3 targetPos = currentRoom.enemies[targetEnemyIdx].position;
                        for (int i = 0; i < MAX_PROJECTILES; i++) {
                            if (!projectiles[i].active) {
                                projectiles[i].position = player.position;
                                projectiles[i].position.y = 1.2f;
                                Vector3 fireDir = Vector3Normalize(Vector3Subtract(targetPos, player.position));
                                projectiles[i].direction = fireDir;
                                projectiles[i].speed = 10.0f;
                                projectiles[i].radius = 0.15f;
                                projectiles[i].active = true;
                                projectiles[i].isEnemy = false;
                                projectiles[i].isAcid = false;
                                
                                projectiles[i].hasBounce = false;
                                projectiles[i].hasPiercing = false;
                                projectiles[i].hasRefraction = false;
                                projectiles[i].pierceCount = 0;
                                break;
                            }
                        }
                    }
                }
            }
            
            // --- SHOOT LOGIC ---
            Vector3 groundAim = GetMouseGroundIntersection(camera);
            Vector3 aimDir = Vector3Subtract(groundAim, player.position);
            aimDir.y = 0.0f;
            
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && player.state != STATE_HURT && player.activeWeapon.currentCooldownTimer <= 0.0f) {
                if (Vector3Length(aimDir) > 0.0f) {
                    Vector3 fireDir = Vector3Normalize(aimDir);
                    
                    int projCount = player.activeWeapon.triggerSlot.projectilesCount;
                    float spread = player.activeWeapon.triggerSlot.spreadAngleDegrees * DEG2RAD;
                    
                    for (int p = 0; p < projCount; p++) {
                        Vector3 currentFireDir = fireDir;
                        if (projCount > 1) {
                            float angleOffset = -spread/2.0f + (spread / (float)(projCount - 1)) * p;
                            float cosA = cosf(angleOffset);
                            float sinA = sinf(angleOffset);
                            currentFireDir.x = fireDir.x * cosA - fireDir.z * sinA;
                            currentFireDir.z = fireDir.x * sinA + fireDir.z * cosA;
                        }
                        
                        for (int i = 0; i < MAX_PROJECTILES; i++) {
                            if (!projectiles[i].active) {
                                projectiles[i].position = player.position;
                                projectiles[i].position.y = 1.0f;
                                projectiles[i].direction = currentFireDir;
                                projectiles[i].radius = hasAcidGlands ? 0.35f : 0.2f;
                                
                                float baseSpeed = hasCyberEye ? 25.0f : 18.0f;
                                projectiles[i].speed = baseSpeed * player.activeWeapon.projectileSlot.speedMultiplier;
                                
                                projectiles[i].active = true;
                                projectiles[i].isEnemy = false;
                                projectiles[i].isAcid = hasAcidGlands;
                                
                                projectiles[i].hasBounce = player.activeWeapon.projectileSlot.hasBounce;
                                projectiles[i].hasPiercing = player.activeWeapon.modifierSlot.hasPiercing;
                                projectiles[i].hasRefraction = motherShip.armory.hasQuantumTech;
                                projectiles[i].pierceCount = player.activeWeapon.modifierSlot.hasPiercing ? 3 : 0;
                                
                                break;
                            }
                        }
                    }
                    
                    player.state = STATE_ATTACK;
                    
                    float baseCooldown = hasCyberEye ? 0.09f : player.activeWeapon.baseCooldown;
                    player.activeWeapon.currentCooldownTimer = baseCooldown * player.activeWeapon.triggerSlot.cooldownMultiplier;
                    player.stateTimer = player.activeWeapon.currentCooldownTimer;
                    
                    SpawnParticles(player.position, hasAcidGlands ? LIME : SKYBLUE, 3);
                }
            }
            
            // --- UPDATE PROJECTILES ---
            for (int i = 0; i < MAX_PROJECTILES; i++) {
                if (projectiles[i].active) {
                    if (!projectiles[i].isEnemy && currentPlanet.gravityMultiplier != 1.0f) {
                        projectiles[i].position.y -= (currentPlanet.gravityMultiplier - 1.0f) * 1.5f * dt;
                    }
                    
                    projectiles[i].position = Vector3Add(projectiles[i].position, Vector3Scale(projectiles[i].direction, projectiles[i].speed * dt));
                    
                    bool hitWall = false;
                    bool bounceX = false;
                    bool bounceZ = false;
                    
                    if (projectiles[i].position.x > 9.8f) { hitWall = true; bounceX = true; projectiles[i].position.x = 9.8f; }
                    else if (projectiles[i].position.x < -9.8f) { hitWall = true; bounceX = true; projectiles[i].position.x = -9.8f; }
                    
                    if (projectiles[i].position.z > 9.8f) { hitWall = true; bounceZ = true; projectiles[i].position.z = 9.8f; }
                    else if (projectiles[i].position.z < -9.8f) { hitWall = true; bounceZ = true; projectiles[i].position.z = -9.8f; }
                    
                    if (projectiles[i].position.y < 0.1f) {
                        hitWall = true;
                    }
                    
                    if (hitWall) {
                        if (!projectiles[i].isEnemy && projectiles[i].hasBounce) {
                            if (bounceX) projectiles[i].direction.x = -projectiles[i].direction.x;
                            if (bounceZ) projectiles[i].direction.z = -projectiles[i].direction.z;
                            if (!bounceX && !bounceZ) projectiles[i].direction.y = -projectiles[i].direction.y;
                            projectiles[i].hasBounce = false;
                            
                            if (projectiles[i].hasRefraction) {
                                projectiles[i].hasRefraction = false;
                                float angle = 25.0f * DEG2RAD;
                                float cosA = cosf(angle);
                                float sinA = sinf(angle);
                                Vector3 splitDir = projectiles[i].direction;
                                splitDir.x = projectiles[i].direction.x * cosA - projectiles[i].direction.z * sinA;
                                splitDir.z = projectiles[i].direction.x * sinA + projectiles[i].direction.z * cosA;
                                
                                for (int k = 0; k < MAX_PROJECTILES; k++) {
                                    if (!projectiles[k].active) {
                                        projectiles[k] = projectiles[i];
                                        projectiles[k].direction = splitDir;
                                        projectiles[k].active = true;
                                        break;
                                    }
                                }
                            }
                            
                            if (player.activeWeapon.projectileSlot.hasGravityPull && projectiles[i].hasPiercing) {
                                SpawnGravityVortex(projectiles[i].position, 2.5f, 5.0f, 20.0f, 3.0f);
                                SpawnParticles(projectiles[i].position, PURPLE, 15);
                            } else {
                                SpawnParticles(projectiles[i].position, SKYBLUE, 5);
                            }
                        } else {
                            projectiles[i].active = false;
                            SpawnImpact(projectiles[i].position);
                            
                            if (!projectiles[i].isEnemy && player.activeWeapon.projectileSlot.hasGravityPull && projectiles[i].hasPiercing) {
                                SpawnGravityVortex(projectiles[i].position, 2.5f, 5.0f, 20.0f, 3.0f);
                                SpawnParticles(projectiles[i].position, PURPLE, 15);
                            }
                            continue;
                        }
                    }
                    
                    for (int p = 0; p < currentRoom.numPillars; p++) {
                        float dx = projectiles[i].position.x - currentRoom.pillars[p].x;
                        float dz = projectiles[i].position.z - currentRoom.pillars[p].z;
                        float distSq = dx * dx + dz * dz;
                        float minDist = projectiles[i].radius + 0.8f;
                        if (distSq < minDist * minDist) {
                            if (!projectiles[i].isEnemy && projectiles[i].hasBounce) {
                                Vector3 normal = Vector3Normalize(Vector3Subtract(projectiles[i].position, currentRoom.pillars[p]));
                                normal.y = 0.0f;
                                float dot = projectiles[i].direction.x * normal.x + projectiles[i].direction.z * normal.z;
                                projectiles[i].direction.x -= 2.0f * dot * normal.x;
                                projectiles[i].direction.z -= 2.0f * dot * normal.z;
                                projectiles[i].direction = Vector3Normalize(projectiles[i].direction);
                                projectiles[i].hasBounce = false;
                                
                                if (player.activeWeapon.projectileSlot.hasGravityPull && projectiles[i].hasPiercing) {
                                    SpawnGravityVortex(projectiles[i].position, 2.5f, 5.0f, 20.0f, 3.0f);
                                    SpawnParticles(projectiles[i].position, PURPLE, 15);
                                }
                            } else {
                                projectiles[i].active = false;
                                SpawnImpact(projectiles[i].position);
                                if (!projectiles[i].isEnemy && player.activeWeapon.projectileSlot.hasGravityPull && projectiles[i].hasPiercing) {
                                    SpawnGravityVortex(projectiles[i].position, 2.5f, 5.0f, 20.0f, 3.0f);
                                    SpawnParticles(projectiles[i].position, PURPLE, 15);
                                }
                            }
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
                            if (playerHalfHeartsHealth <= 0) ChangeState(SCREEN_GAMEOVER);
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
                        
                        if (playerHalfHeartsHealth <= 0) ChangeState(SCREEN_GAMEOVER);
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
                        
                        if (playerHalfHeartsHealth <= 0) ChangeState(SCREEN_GAMEOVER);
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
                            
                            float critMult = 1.0f;
                            if (activeRelics[0].active && GetRandomValue(0, 100) < 35) {
                                critMult = 2.0f;
                                SpawnParticles(projectiles[p].position, GOLD, 12);
                            }
                            float armoryMult = motherShip.armory.baseDamageMultiplier;
                            float baseDmg = (projectiles[p].isAcid ? 30.0f : 15.0f) * armoryMult * critMult;
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
                        if (!particles[i].isAtmospheric) {
                            particles[i].velocity.y -= 9.8f * dt;
                        }
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
            
            // Centralized player death check
            if (playerHalfHeartsHealth <= 0) {
                playerHalfHeartsHealth = 0;
                ChangeState(SCREEN_GAMEOVER);
                currentClone.cloneIndex++;
                currentClone.memoryCoherence -= 15.0f;
                if (currentClone.memoryCoherence < 0.0f) currentClone.memoryCoherence = 0.0f;
                currentClone.paranoiaLevel = 100.0f - currentClone.memoryCoherence;
                
                motherShip.totalRunsCompleted++;
                SaveMothershipState(motherShip, "mothership.dat");
                SaveCloneStatus(currentClone, "clone.dat");
            }
        }
        
        // --- DRAWING / RENDERING ---
        Shader activeShader = { 0 };
        if (activeRelics[0].active && playerHalfHeartsHealth <= (playerMaxHearts * 2 * 0.3f)) {
            activeShader = nebulaShader;
            float strength = 0.007f + 0.005f * sinf(gameTimer * 10.0f);
            float noise = 0.08f;
            SetShaderValue(nebulaShader, abLoc, &strength, SHADER_UNIFORM_FLOAT);
            SetShaderValue(nebulaShader, noiseLoc, &noise, SHADER_UNIFORM_FLOAT);
        } else if (activeRelics[2].active) {
            activeShader = tachyonShader;
            float blur = 0.0025f + 0.001f * sinf(gameTimer * 5.0f);
            SetShaderValue(tachyonShader, blurLoc, &blur, SHADER_UNIFORM_FLOAT);
        }
        
        bool useRenderTarget = (activeShader.id > 0);
        
        if (useRenderTarget) {
            BeginTextureMode(targetTex);
        } else {
            BeginDrawing();
        }
        
        ClearBackground((Color){ 6, 6, 12, 255 }); // space void background
            
            if (currentScreen == SCREEN_TITLE) {
                // 1. Draw Starfield background in 3D
                BeginMode3D(camera);
                    for (int i = 0; i < MAX_STARS; i++) {
                        float driftX = spaceStars[i].position.x - camera.position.x * spaceStars[i].parallaxFactor;
                        float driftZ = spaceStars[i].position.z - camera.position.z * spaceStars[i].parallaxFactor;
                        Vector3 starPos = { driftX, spaceStars[i].position.y, driftZ };
                        DrawBillboardRec(camera, charSpritesheet, (Rectangle){ 96.0f, 192.0f, 32.0f, 32.0f }, starPos, (Vector2){ spaceStars[i].size, spaceStars[i].size }, spaceStars[i].color);
                    }
                EndMode3D();
                
                // 2. Dark overlay for readability
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.65f));
                
                // Decorative grid/hud lines
                DrawRectangleLines(20, 20, screenWidth - 40, screenHeight - 40, Fade(CYAN, 0.3f));
                DrawRectangleLines(24, 24, screenWidth - 48, screenHeight - 48, Fade(CYAN, 0.1f));
                
                // Corner decors
                DrawRectangle(15, 15, 30, 6, CYAN);
                DrawRectangle(15, 15, 6, 30, CYAN);
                DrawRectangle(screenWidth - 45, 15, 30, 6, CYAN);
                DrawRectangle(screenWidth - 21, 15, 6, 30, CYAN);
                DrawRectangle(15, screenHeight - 21, 30, 6, CYAN);
                DrawRectangle(15, screenHeight - 45, 6, 30, CYAN);
                DrawRectangle(screenWidth - 45, screenHeight - 21, 30, 6, CYAN);
                DrawRectangle(screenWidth - 21, screenHeight - 45, 6, 30, CYAN);
                
                // 3. Draw Title/Logo
                const char *titleText = "PROJECT: DEHUMANIZER";
                int titleSize = 48;
                int titleW = MeasureText(titleText, titleSize);
                int titleX = screenWidth / 2 - titleW / 2;
                int titleY = screenHeight / 2 - 190;
                if (titleY < 40) titleY = 40;
                
                // CRT Shadow effect
                DrawText(titleText, titleX + 3, titleY + 3, titleSize, Fade(RED, 0.6f));
                DrawText(titleText, titleX - 2, titleY - 2, titleSize, Fade(BLUE, 0.6f));
                DrawText(titleText, titleX, titleY, titleSize, GOLD);
                
                const char *subText = "SPACESHIP ROGUE 2.5D CRAWLER";
                int subSize = 18;
                int subW = MeasureText(subText, subSize);
                DrawText(subText, screenWidth / 2 - subW / 2, titleY + 55, subSize, Fade(CYAN, 0.9f));
                
                // 4. Draw interactive buttons
                Vector2 mousePos = GetMousePosition();
                int btnW = 380;
                int btnH = 34;
                int startY = screenHeight / 2 - 15;
                
                Rectangle btnEasy = { (float)(screenWidth / 2 - btnW / 2), (float)startY, (float)btnW, (float)btnH };
                Rectangle btnNorm = { (float)(screenWidth / 2 - btnW / 2), (float)(startY + 42), (float)btnW, (float)btnH };
                Rectangle btnHard = { (float)(screenWidth / 2 - btnW / 2), (float)(startY + 84), (float)btnW, (float)btnH };
                Rectangle btnStart = { (float)(screenWidth / 2 - 160), (float)(startY + 150), 320.0f, 45.0f };
                
                // Easy button
                bool easyHover = CheckCollisionPointRec(mousePos, btnEasy);
                Color easyColor = (selectedDifficulty == DIFF_EASY) ? LIME : GRAY;
                DrawRectangleRec(btnEasy, (selectedDifficulty == DIFF_EASY) ? Fade(LIME, 0.15f) : (easyHover ? Fade(GRAY, 0.1f) : BLANK));
                DrawRectangleLinesEx(btnEasy, easyHover ? 2 : 1, easyColor);
                DrawText("1. DIFICULTAD FACIL", btnEasy.x + 20, btnEasy.y + 8, 16, easyColor);
                if (selectedDifficulty == DIFF_EASY) {
                    DrawText("[4 CORAZONES - DROPS MULTIPLES]", btnEasy.x + btnEasy.width - 240, btnEasy.y + 11, 11, LIME);
                }
                
                // Normal button
                bool normHover = CheckCollisionPointRec(mousePos, btnNorm);
                Color normColor = (selectedDifficulty == DIFF_NORMAL) ? GOLD : GRAY;
                DrawRectangleRec(btnNorm, (selectedDifficulty == DIFF_NORMAL) ? Fade(GOLD, 0.15f) : (normHover ? Fade(GRAY, 0.1f) : BLANK));
                DrawRectangleLinesEx(btnNorm, normHover ? 2 : 1, normColor);
                DrawText("2. DIFICULTAD NORMAL", btnNorm.x + 20, btnNorm.y + 8, 16, normColor);
                if (selectedDifficulty == DIFF_NORMAL) {
                    DrawText("[3 CORAZONES - BALANCE ESTANDAR]", btnNorm.x + btnNorm.width - 240, btnNorm.y + 11, 11, GOLD);
                }
                
                // Hard button
                bool hardHover = CheckCollisionPointRec(mousePos, btnHard);
                Color hardColor = (selectedDifficulty == DIFF_HARD) ? RED : GRAY;
                DrawRectangleRec(btnHard, (selectedDifficulty == DIFF_HARD) ? Fade(RED, 0.15f) : (hardHover ? Fade(GRAY, 0.1f) : BLANK));
                DrawRectangleLinesEx(btnHard, hardHover ? 2 : 1, hardColor);
                DrawText("3. DIFICULTAD EXPERTO", btnHard.x + 20, btnHard.y + 8, 16, hardColor);
                if (selectedDifficulty == DIFF_HARD) {
                    DrawText("[DAÑO CRITICO - ENEMIGOS VELOCES]", btnHard.x + btnHard.width - 240, btnHard.y + 11, 11, RED);
                }
                
                // Start button
                bool startHover = CheckCollisionPointRec(mousePos, btnStart);
                float startPulse = sinf((float)GetTime() * 5.0f) * 0.4f + 0.6f;
                Color startBtnCol = startHover ? GOLD : Fade(GOLD, startPulse);
                DrawRectangleRec(btnStart, startHover ? Fade(GOLD, 0.2f) : Fade(GOLD, 0.05f));
                DrawRectangleLinesEx(btnStart, startHover ? 3 : 2, startBtnCol);
                
                const char *startLabel = "INICIAR OPERACION";
                int slSize = 20;
                int slW = MeasureText(startLabel, slSize);
                DrawText(startLabel, btnStart.x + btnStart.width / 2 - slW / 2, btnStart.y + btnStart.height / 2 - slSize / 2, slSize, startBtnCol);
            }
            else if (currentScreen == SCREEN_NEXUS) {
                // Positions of interactive objects on the Bridge
                Vector3 greenhousePos = { -4.5f, 1.0f, -2.0f };
                Vector3 armoryPos = { 4.5f, 1.0f, -2.0f };
                Vector3 navigationPos = { 0.0f, 1.0f, -4.5f };
                Vector3 iaPos = { 0.0f, 1.0f, -1.0f };
                Vector3 sciNPC_Pos = { -3.0f, 1.0f, 2.0f };
                Vector3 soldNPC_Pos = { 3.0f, 1.0f, 2.0f };
                
                float distGreenhouse = Vector3Distance(player.position, greenhousePos);
                float distArmory = Vector3Distance(player.position, armoryPos);
                float distNavigation = Vector3Distance(player.position, navigationPos);
                float distIA = Vector3Distance(player.position, iaPos);
                float distSci = Vector3Distance(player.position, sciNPC_Pos);
                float distSold = Vector3Distance(player.position, soldNPC_Pos);

                // 1. Draw 3D Spaceship Bridge Environment
                BeginMode3D(camera);
                    // A. Parallax space stars background
                    for (int i = 0; i < MAX_STARS; i++) {
                        float driftX = spaceStars[i].position.x - camera.position.x * spaceStars[i].parallaxFactor;
                        float driftZ = spaceStars[i].position.z - camera.position.z * spaceStars[i].parallaxFactor;
                        Vector3 starPos = { driftX, spaceStars[i].position.y, driftZ };
                        DrawBillboardRec(camera, charSpritesheet, (Rectangle){ 96.0f, 192.0f, 32.0f, 32.0f }, starPos, (Vector2){ spaceStars[i].size, spaceStars[i].size }, spaceStars[i].color);
                    }
                    
                    // A.5 Planet in the background viewport
                    float planetOrbitTime = (float)GetTime() * 0.05f;
                    Vector3 planetPos = { 
                        cosf(planetOrbitTime) * 15.0f, 
                        -5.0f, 
                        -45.0f + sinf(planetOrbitTime) * 5.0f 
                    };
                    
                    // Base color based on atmospheric tint (darkened)
                    Color baseColor = { 
                        (unsigned char)(currentPlanet.atmosphericTint.r / 2),
                        (unsigned char)(currentPlanet.atmosphericTint.g / 2),
                        (unsigned char)(currentPlanet.atmosphericTint.b / 2),
                        255 
                    };
                    
                    // Draw Planet Core
                    DrawSphere(planetPos, 18.0f, baseColor);
                    
                    // Draw Atmosphere Glow (Additive-like overlay)
                    rlDisableDepthMask();
                    for (int i = 0; i < 3; i++) {
                        DrawSphere(planetPos, 18.2f + (float)i * 0.4f, Fade(currentPlanet.atmosphericTint, 0.3f - (float)i * 0.1f));
                    }
                    
                    // Draw Rings if hazard is high
                    if (currentPlanet.hazardIntensity > 0.5f) {
                        rlPushMatrix();
                            rlTranslatef(planetPos.x, planetPos.y, planetPos.z);
                            rlRotatef(25.0f, 1.0f, 0.0f, 1.0f);
                            DrawCylinderWires((Vector3){0,0,0}, 28.0f, 28.0f, 0.1f, 32, Fade(baseColor, 0.5f));
                            DrawCylinderWires((Vector3){0,0,0}, 32.0f, 32.0f, 0.1f, 32, Fade(WHITE, 0.2f));
                        rlPopMatrix();
                    }
                    rlEnableDepthMask();
                    
                    // B. Spaceship cockpit floor grid (metallic grey compartments)
                    for (int z = -6; z <= 6; z++) {
                        for (int x = -6; x <= 6; x++) {
                            Rectangle floorSrc = { 0.0f, 2.0f * 32.0f, 32.0f, 32.0f };
                            DrawFloorTile(envSpritesheet, floorSrc, (Vector3){ (float)x, 0.0f, (float)z }, (Vector2){ 1.0f, 1.0f }, (Color){ 70, 75, 80, 255 });
                        }
                    }
                    
                    // C. Walls surrounding the bridge room
                    for (int x = -7; x <= 7; x++) {
                        Rectangle wallSrc = { 0.0f, 0.0f, 32.0f, 32.0f };
                        // Back wall (cockpit viewport)
                        if (x == -7 || x == 7) {
                            DrawWallBlock(envSpritesheet, wallSrc, (Vector3){ (float)x, 2.0f, -7.0f }, (Vector3){ 1.0f, 4.0f, 1.0f }, (Color){ 45, 50, 55, 255 });
                        } else {
                            // low border under front viewport window
                            DrawWallBlock(envSpritesheet, wallSrc, (Vector3){ (float)x, 0.5f, -7.0f }, (Vector3){ 1.0f, 1.0f, 1.0f }, (Color){ 55, 60, 65, 255 });
                        }
                        // Front entrance wall
                        DrawWallBlock(envSpritesheet, wallSrc, (Vector3){ (float)x, 2.0f, 7.0f }, (Vector3){ 1.0f, 4.0f, 1.0f }, (Color){ 50, 55, 60, 255 });
                    }
                    // Left and right walls
                    for (int z = -6; z <= 6; z++) {
                        Rectangle wallSrc = { 0.0f, 0.0f, 32.0f, 32.0f };
                        DrawWallBlock(envSpritesheet, wallSrc, (Vector3){ -7.0f, 2.0f, (float)z }, (Vector3){ 1.0f, 4.0f, 1.0f }, (Color){ 50, 55, 60, 255 });
                        DrawWallBlock(envSpritesheet, wallSrc, (Vector3){ 7.0f, 2.0f, (float)z }, (Vector3){ 1.0f, 4.0f, 1.0f }, (Color){ 50, 55, 60, 255 });
                    }
                    
                    // D. Interactive 3D modules
                    // Greenhouse Cylinder (Green)
                    DrawCylinder((Vector3){ -4.5f, 0.8f, -2.0f }, 0.6f, 0.6f, 1.6f, 16, Fade(GREEN, 0.25f));
                    DrawCylinderWires((Vector3){ -4.5f, 0.8f, -2.0f }, 0.6f, 0.6f, 1.6f, 16, GREEN);
                    if (GetRandomValue(0, 100) < 6) {
                        SpawnParticles((Vector3){ -4.5f + (float)GetRandomValue(-2, 2) * 0.1f, 0.2f, -2.0f + (float)GetRandomValue(-2, 2) * 0.1f }, GREEN, 1);
                    }
                    
                    // Armory Module Cylinder (Red)
                    DrawCylinder((Vector3){ 4.5f, 0.8f, -2.0f }, 0.6f, 0.6f, 1.6f, 16, Fade(RED, 0.25f));
                    DrawCylinderWires((Vector3){ 4.5f, 0.8f, -2.0f }, 0.6f, 0.6f, 1.6f, 16, RED);
                    if (GetRandomValue(0, 100) < 6) {
                        SpawnParticles((Vector3){ 4.5f + (float)GetRandomValue(-2, 2) * 0.1f, 0.2f, -2.0f + (float)GetRandomValue(-2, 2) * 0.1f }, ORANGE, 1);
                    }
                    
                    // Navigation Console Projector (Cyan)
                    DrawCylinder((Vector3){ 0.0f, 0.5f, -4.5f }, 0.8f, 0.8f, 1.0f, 16, Fade(CYAN, 0.3f));
                    DrawCylinderWires((Vector3){ 0.0f, 0.5f, -4.5f }, 0.8f, 0.8f, 1.0f, 16, CYAN);
                    DrawSphere((Vector3){ 0.0f, 1.2f, -4.5f }, 0.22f, Fade(CYAN, 0.8f));
                    
                    // IA Holographic Projector
                    DrawCylinder((Vector3){ 0.0f, 0.2f, -1.0f }, 0.4f, 0.4f, 0.4f, 16, Fade(DARKGRAY, 0.8f));
                    DrawSphere((Vector3){ 0.0f, 0.9f, -1.0f }, 0.15f + sinf((float)GetTime() * 4.0f) * 0.03f, Fade(CYAN, 0.9f));
                    
                    // E. Draw NPC Billboards
                    // Scientist (Row 3, Column 0)
                    Rectangle sciSrc = { 0.0f, 3.0f * 32.0f, 32.0f, 32.0f };
                    DrawBillboardRec(camera, charSpritesheet, sciSrc, (Vector3){ sciNPC_Pos.x, sciNPC_Pos.y - 0.2f, sciNPC_Pos.z }, (Vector2){ 1.8f, 1.8f }, WHITE);
                    
                    // Soldier (Row 4, Column 0)
                    Rectangle soldSrc = { 0.0f, 4.0f * 32.0f, 32.0f, 32.0f };
                    DrawBillboardRec(camera, charSpritesheet, soldSrc, (Vector3){ soldNPC_Pos.x, soldNPC_Pos.y - 0.2f, soldNPC_Pos.z }, (Vector2){ 1.8f, 1.8f }, WHITE);
                    
                    // F. Draw Player Clone Avatar
                    if (player.health > 0.0f || playerHalfHeartsHealth > 0) {
                        Color pColor = WHITE;
                        // Legs
                        int legRow = (player.direction.z < 0.0f) ? 2 : 1;
                        Rectangle legSrc = { (float)player.animFrame * 32.0f, (float)legRow * 32.0f, 32.0f, 32.0f };
                        Vector3 legPos = { player.position.x, player.position.y - 0.2f, player.position.z };
                        DrawBillboardRec(camera, charSpritesheet, legSrc, legPos, (Vector2){ 1.8f, 1.8f }, pColor);
                        
                        // Head
                        int headState = HEAD_LOOK_DOWN;
                        bool flipHead = (player.direction.x > 0.0f);
                        if (player.direction.z < -0.5f) {
                            headState = HEAD_LOOK_UP;
                        } else if (fabsf(player.direction.x) > 0.5f) {
                            headState = HEAD_LOOK_LEFT;
                        } else {
                            headState = HEAD_LOOK_DOWN;
                        }
                        Rectangle headSrc = { 
                            (float)headState * 32.0f, 
                            0.0f, 
                            flipHead ? -32.0f : 32.0f,
                            32.0f 
                        };
                        Vector3 headPos = { player.position.x, player.position.y + 0.6f, player.position.z };
                        DrawBillboardRec(camera, charSpritesheet, headSrc, headPos, (Vector2){ 1.8f, 1.8f }, pColor);
                    }
                    
                    // G. Draw Sparks Particles inside cockpit
                    for (int i = 0; i < MAX_PARTICLES; i++) {
                        if (particles[i].active && !particles[i].isGas) {
                            float alpha = particles[i].life / particles[i].maxLife;
                            DrawBillboardRec(camera, charSpritesheet, (Rectangle){ 128.0f, 192.0f, 32.0f, 32.0f }, particles[i].position, (Vector2){ 0.35f, 0.35f }, Fade(particles[i].color, alpha));
                        }
                    }
                EndMode3D();

                // Auto-close dialogue when walking away
                if (activeCrewDialogIdx == 1 && distIA > 2.2f) activeCrewDialogIdx = 0;
                if (activeCrewDialogIdx == 2 && distSci > 2.2f) activeCrewDialogIdx = 0;
                if (activeCrewDialogIdx == 3 && distSold > 2.2f) activeCrewDialogIdx = 0;

                // 2D Interactive prompts when walking around (activeNexusOverlay == 0)
                if (activeNexusOverlay == 0) {
                    if (distGreenhouse < 2.0f) {
                        Vector2 sPos = GetWorldToScreen((Vector3){ greenhousePos.x, greenhousePos.y + 1.2f, greenhousePos.z }, camera);
                        DrawRectangle(sPos.x - 90, sPos.y - 12, 180, 24, Fade(BLACK, 0.8f));
                        DrawRectangleLines(sPos.x - 90, sPos.y - 12, 180, 24, GREEN);
                        DrawText("[E] TERMINAL INVERNADERO", sPos.x - 80, sPos.y - 6, 11, GREEN);
                    }
                    else if (distArmory < 2.0f) {
                        Vector2 sPos = GetWorldToScreen((Vector3){ armoryPos.x, armoryPos.y + 1.2f, armoryPos.z }, camera);
                        DrawRectangle(sPos.x - 95, sPos.y - 12, 190, 24, Fade(BLACK, 0.8f));
                        DrawRectangleLines(sPos.x - 95, sPos.y - 12, 190, 24, RED);
                        DrawText("[E] CONTROL DE ARMAS Y MODS", sPos.x - 90, sPos.y - 6, 11, RED);
                    }
                    else if (distNavigation < 2.0f) {
                        Vector2 sPos = GetWorldToScreen((Vector3){ navigationPos.x, navigationPos.y + 1.2f, navigationPos.z }, camera);
                        DrawRectangle(sPos.x - 90, sPos.y - 12, 180, 24, Fade(BLACK, 0.8f));
                        DrawRectangleLines(sPos.x - 90, sPos.y - 12, 180, 24, CYAN);
                        DrawText("[E] CONSOLA DE NAVEGACION", sPos.x - 80, sPos.y - 6, 11, CYAN);
                    }
                    else if (distIA < 1.8f) {
                        Vector2 sPos = GetWorldToScreen((Vector3){ iaPos.x, iaPos.y + 0.8f, iaPos.z }, camera);
                        DrawRectangle(sPos.x - 80, sPos.y - 12, 160, 24, Fade(BLACK, 0.8f));
                        DrawRectangleLines(sPos.x - 80, sPos.y - 12, 160, 24, CYAN);
                        DrawText("[E] CONVERSAR CON IA", sPos.x - 70, sPos.y - 6, 11, CYAN);
                    }
                    else if (distSci < 1.8f) {
                        Vector2 sPos = GetWorldToScreen((Vector3){ sciNPC_Pos.x, sciNPC_Pos.y + 1.0f, sciNPC_Pos.z }, camera);
                        DrawRectangle(sPos.x - 85, sPos.y - 12, 170, 24, Fade(BLACK, 0.8f));
                        DrawRectangleLines(sPos.x - 85, sPos.y - 12, 170, 24, LIME);
                        DrawText("[E] CIENCIA Y BIOMASA", sPos.x - 75, sPos.y - 6, 11, LIME);
                    }
                    else if (distSold < 1.8f) {
                        Vector2 sPos = GetWorldToScreen((Vector3){ soldNPC_Pos.x, soldNPC_Pos.y + 1.0f, soldNPC_Pos.z }, camera);
                        DrawRectangle(sPos.x - 85, sPos.y - 12, 170, 24, Fade(BLACK, 0.8f));
                        DrawRectangleLines(sPos.x - 85, sPos.y - 12, 170, 24, PINK);
                        DrawText("[E] REPORTAR CON SOLDADO", sPos.x - 75, sPos.y - 6, 11, PINK);
                    }
                }
                
                // Top Header HUD bar
                DrawRectangle(0, 0, screenWidth, 75, Fade(DARKGRAY, 0.4f));
                DrawLine(0, 75, screenWidth, 75, CYAN);
                
                char headerTitle[] = "NAVE NODRIZA USG DEHUMANIZER - COCKPIT DE COMANDO";
                DrawTextGlitch(headerTitle, 30, 24, 20, CYAN, 0.05f);
                
                // Resources on top right
                char resText[256];
                sprintf(resText, "ISOTOPOS: %d | TRIPULANTES: %d | OPERADOR CLON: #%03d (COHERENCIA: %.1f%%)", 
                        motherShip.isotopicResources, motherShip.rescuedCrew, currentClone.cloneIndex, currentClone.memoryCoherence);
                int resW = MeasureText(resText, 14);
                DrawText(resText, screenWidth - resW - 30, 28, 14, GREEN);
                
                Vector2 mousePos = GetMousePosition();
                
                // ==================== LEFT PANEL: UPGRADES ====================
                if (activeNexusOverlay == 1) {
                    // Darken background overlay
                    DrawRectangle(0, 75, screenWidth, screenHeight - 75, Fade(BLACK, 0.5f));
                    
                    int panelX = 40;
                    int startY = 110;
                    int panelW = 410;
                    
                    // Draw Tabs
                    Rectangle btnTabModules = { (float)panelX, (float)(startY - 32), 150.0f, 30.0f };
                    Rectangle btnTabWeapon = { (float)(panelX + 155), (float)(startY - 32), 150.0f, 30.0f };
                    
                    bool modulesTabHover = CheckCollisionPointRec(mousePos, btnTabModules);
                    bool weaponTabHover = CheckCollisionPointRec(mousePos, btnTabWeapon);
                    
                    // Modules Tab
                    DrawRectangleRec(btnTabModules, activeLeftTab == 0 ? Fade(CYAN, 0.2f) : (modulesTabHover ? Fade(CYAN, 0.05f) : BLANK));
                    DrawRectangleLinesEx(btnTabModules, activeLeftTab == 0 ? 2 : 1, activeLeftTab == 0 ? CYAN : GRAY);
                    DrawText("MODULOS NAVE", btnTabModules.x + 25, btnTabModules.y + 8, 13, activeLeftTab == 0 ? CYAN : RAYWHITE);
                    
                    // Weapon Tab
                    DrawRectangleRec(btnTabWeapon, activeLeftTab == 1 ? Fade(CYAN, 0.2f) : (weaponTabHover ? Fade(CYAN, 0.05f) : BLANK));
                    DrawRectangleLinesEx(btnTabWeapon, activeLeftTab == 1 ? 2 : 1, activeLeftTab == 1 ? CYAN : GRAY);
                    DrawText("ARMAMENTO Y MODS", btnTabWeapon.x + 15, btnTabWeapon.y + 8, 13, activeLeftTab == 1 ? CYAN : RAYWHITE);
                    
                    DrawRectangle(panelX, startY, panelW, 430, Fade(BLACK, 0.8f));
                    DrawRectangleLines(panelX, startY, panelW, 430, Fade(CYAN, 0.6f));
                    
                    if (activeLeftTab == 0) {
                        DrawText("MODULOS DE LA NAVE NODRIZA", panelX + 20, startY + 15, 18, GOLD);
                        DrawLine(panelX + 20, startY + 40, panelX + panelW - 20, startY + 40, Fade(CYAN, 0.3f));
                        
                        // Greenhouse Upgrade Row (Y: startY + 55)
                        {
                            int rowY = startY + 55;
                            char label[64];
                            sprintf(label, "INVERNADERO HIDROPONICO (Lvl %d/3)", motherShip.greenhouse.level);
                            DrawText(label, panelX + 20, rowY, 14, RAYWHITE);
                            
                            if (motherShip.greenhouse.level < 3) {
                                int cost = (motherShip.greenhouse.level == 1) ? 180 : 500;
                                int reqCrew = (motherShip.greenhouse.level == 1) ? 2 : 6;
                                
                                char costTxt[128];
                                sprintf(costTxt, "Coste: %d Isot. / %d Trip.", cost, reqCrew);
                                DrawText(costTxt, panelX + 20, rowY + 18, 11, GRAY);
                                DrawText("Efecto: Aumenta max corazones y curacion.", panelX + 20, rowY + 32, 11, LIME);
                                
                                // Upgrade Button
                                Rectangle btn = { (float)(panelX + 270), (float)(rowY + 10), 120.0f, 30.0f };
                                bool hover = CheckCollisionPointRec(mousePos, btn);
                                bool canAfford = (motherShip.isotopicResources >= cost && motherShip.rescuedCrew >= reqCrew);
                                
                                Color btnCol = canAfford ? (hover ? GREEN : Fade(GREEN, 0.8f)) : GRAY;
                                DrawRectangleRec(btn, hover && canAfford ? Fade(GREEN, 0.2f) : BLANK);
                                DrawRectangleLinesEx(btn, hover ? 2 : 1, btnCol);
                                DrawText("MEJORAR", btn.x + 30, btn.y + 8, 13, btnCol);
                            } else {
                                DrawText("Efecto: Aumenta max corazones y curacion.", panelX + 20, rowY + 18, 11, LIME);
                                DrawText("NIVEL MAXIMO ALCANZADO", panelX + 20, rowY + 32, 12, GOLD);
                            }
                        }
                        
                        // Armory Upgrade Row (Y: startY + 165)
                        {
                            int rowY = startY + 165;
                            char label[64];
                            sprintf(label, "ARMERIA CRISTALINA (Lvl %d/3)", motherShip.armory.level);
                            DrawText(label, panelX + 20, rowY, 14, RAYWHITE);
                            
                            if (motherShip.armory.level < 3) {
                                int cost = (motherShip.armory.level == 1) ? 250 : 600;
                                int reqCrew = (motherShip.armory.level == 1) ? 3 : 8;
                                
                                char costTxt[128];
                                sprintf(costTxt, "Coste: %d Isot. / %d Trip.", cost, reqCrew);
                                DrawText(costTxt, panelX + 20, rowY + 18, 11, GRAY);
                                DrawText("Efecto: Incrementa dano base del arma.", panelX + 20, rowY + 32, 11, LIME);
                                
                                // Upgrade Button
                                Rectangle btn = { (float)(panelX + 270), (float)(rowY + 10), 120.0f, 30.0f };
                                bool hover = CheckCollisionPointRec(mousePos, btn);
                                bool canAfford = (motherShip.isotopicResources >= cost && motherShip.rescuedCrew >= reqCrew);
                                
                                Color btnCol = canAfford ? (hover ? GREEN : Fade(GREEN, 0.8f)) : GRAY;
                                DrawRectangleRec(btn, hover && canAfford ? Fade(GREEN, 0.2f) : BLANK);
                                DrawRectangleLinesEx(btn, hover ? 2 : 1, btnCol);
                                DrawText("MEJORAR", btn.x + 30, btn.y + 8, 13, btnCol);
                            } else {
                                DrawText("Efecto: Incrementa dano base del arma.", panelX + 20, rowY + 18, 11, LIME);
                                DrawText("NIVEL MAXIMO ALCANZADO", panelX + 20, rowY + 32, 12, GOLD);
                            }
                        }
                        
                        // Engine Room Upgrade Row (Y: startY + 275)
                        {
                            int rowY = startY + 275;
                            char label[64];
                            sprintf(label, "SALA DE MOTORES HYPER-G (Lvl %d/3)", motherShip.engineRoom.level);
                            DrawText(label, panelX + 20, rowY, 14, RAYWHITE);
                            
                            if (motherShip.engineRoom.level < 3) {
                                int cost = (motherShip.engineRoom.level == 1) ? 150 : 400;
                                int reqCrew = (motherShip.engineRoom.level == 1) ? 2 : 5;
                                
                                char costTxt[128];
                                sprintf(costTxt, "Coste: %d Isot. / %d Trip.", cost, reqCrew);
                                DrawText(costTxt, panelX + 20, rowY + 18, 11, GRAY);
                                DrawText("Efecto: Neutraliza la gravedad de planetas.", panelX + 20, rowY + 32, 11, LIME);
                                
                                // Upgrade Button
                                Rectangle btn = { (float)(panelX + 270), (float)(rowY + 10), 120.0f, 30.0f };
                                bool hover = CheckCollisionPointRec(mousePos, btn);
                                bool canAfford = (motherShip.isotopicResources >= cost && motherShip.rescuedCrew >= reqCrew);
                                
                                Color btnCol = canAfford ? (hover ? GREEN : Fade(GREEN, 0.8f)) : GRAY;
                                DrawRectangleRec(btn, hover && canAfford ? Fade(GREEN, 0.2f) : BLANK);
                                DrawRectangleLinesEx(btn, hover ? 2 : 1, btnCol);
                                DrawText("MEJORAR", btn.x + 30, btn.y + 8, 13, btnCol);
                            } else {
                                DrawText("Efecto: Neutraliza la gravedad de planetas.", panelX + 20, rowY + 18, 11, LIME);
                                DrawText("NIVEL MAXIMO ALCANZADO", panelX + 20, rowY + 32, 12, GOLD);
                            }
                        }
                        
                        // Help text at bottom of left panel
                        DrawText("Consigue Isotopos matando enemigos y rescatando", panelX + 20, startY + 375, 11, GRAY);
                        DrawText("tripulantes en el nucleo del Reactor Final (Jefe).", panelX + 20, startY + 390, 11, GRAY);
                    }
                    else {
                        DrawText("SISTEMA DE MODULOS DE ARMA", panelX + 20, startY + 15, 18, GOLD);
                        DrawLine(panelX + 20, startY + 40, panelX + panelW - 20, startY + 40, Fade(CYAN, 0.3f));
                        
                        int maxSlots = motherShip.armory.level;
                        int currentlyEquipped = (motherShip.equippedBounce ? 1 : 0) + 
                                                 (motherShip.equippedPiercing ? 1 : 0) + 
                                                 (motherShip.equippedQuantum ? 1 : 0);
                                                 
                        char slotsStr[128];
                        sprintf(slotsStr, "Ranuras Activas: %d / %d (Aumenta con Armeria Lvl)", currentlyEquipped, maxSlots);
                        DrawText(slotsStr, panelX + 20, startY + 46, 12, CYAN);
                        
                        // Bounce Mod Row (startY + 65)
                        {
                            int rowY = startY + 65;
                            DrawText("REBOTE GRAVITATORIO", panelX + 20, rowY, 14, RAYWHITE);
                            DrawText("Los proyectiles rebotan en las paredes.", panelX + 20, rowY + 18, 11, GRAY);
                            
                            Rectangle btn = { (float)(panelX + 270), (float)(rowY + 5), 120.0f, 30.0f };
                            bool hover = CheckCollisionPointRec(mousePos, btn);
                            
                            if (!motherShip.unlockedBounce) {
                                bool canAfford = motherShip.isotopicResources >= 100;
                                Color btnCol = canAfford ? (hover ? GREEN : Fade(GREEN, 0.8f)) : GRAY;
                                DrawRectangleRec(btn, hover && canAfford ? Fade(GREEN, 0.2f) : BLANK);
                                DrawRectangleLinesEx(btn, hover ? 2 : 1, btnCol);
                                DrawText("DESBLOQ (100)", btn.x + 12, btn.y + 8, 12, btnCol);
                            } else {
                                Color btnCol = motherShip.equippedBounce ? CYAN : GRAY;
                                if (hover) btnCol = RAYWHITE;
                                DrawRectangleRec(btn, motherShip.equippedBounce ? Fade(CYAN, 0.2f) : (hover ? Fade(GRAY, 0.1f) : BLANK));
                                DrawRectangleLinesEx(btn, hover ? 2 : 1, btnCol);
                                DrawText(motherShip.equippedBounce ? "EQUIPADO" : "EQUIPAR", btn.x + 28, btn.y + 8, 12, btnCol);
                            }
                        }
                        
                        // Piercing Mod Row (startY + 175)
                        {
                            int rowY = startY + 175;
                            DrawText("PERFORACION DE PLASMA", panelX + 20, rowY, 14, RAYWHITE);
                            DrawText("Proyectiles atraviesan hasta 3 enemigos.", panelX + 20, rowY + 18, 11, GRAY);
                            
                            Rectangle btn = { (float)(panelX + 270), (float)(rowY + 5), 120.0f, 30.0f };
                            bool hover = CheckCollisionPointRec(mousePos, btn);
                            
                            if (!motherShip.unlockedPiercing) {
                                bool canAfford = motherShip.isotopicResources >= 150;
                                Color btnCol = canAfford ? (hover ? GREEN : Fade(GREEN, 0.8f)) : GRAY;
                                DrawRectangleRec(btn, hover && canAfford ? Fade(GREEN, 0.2f) : BLANK);
                                DrawRectangleLinesEx(btn, hover ? 2 : 1, btnCol);
                                DrawText("DESBLOQ (150)", btn.x + 12, btn.y + 8, 12, btnCol);
                            } else {
                                Color btnCol = motherShip.equippedPiercing ? CYAN : GRAY;
                                if (hover) btnCol = RAYWHITE;
                                DrawRectangleRec(btn, motherShip.equippedPiercing ? Fade(CYAN, 0.2f) : (hover ? Fade(GRAY, 0.1f) : BLANK));
                                DrawRectangleLinesEx(btn, hover ? 2 : 1, btnCol);
                                DrawText(motherShip.equippedPiercing ? "EQUIPADO" : "EQUIPAR", btn.x + 28, btn.y + 8, 12, btnCol);
                            }
                        }
                        
                        // Quantum Mod Row (startY + 285)
                        {
                            int rowY = startY + 285;
                            DrawText("REFRACCION CUANTICA", panelX + 20, rowY, 14, RAYWHITE);
                            DrawText("Divide proyectiles en dos al impactar.", panelX + 20, rowY + 18, 11, GRAY);
                            
                            Rectangle btn = { (float)(panelX + 270), (float)(rowY + 5), 120.0f, 30.0f };
                            bool hover = CheckCollisionPointRec(mousePos, btn);
                            
                            if (motherShip.armory.level < 3) {
                                DrawRectangleLinesEx(btn, 1, DARKGRAY);
                                DrawText("BLOQUEADO", btn.x + 24, btn.y + 8, 12, RED);
                                DrawText("Requiere Armeria Lvl 3", panelX + 20, rowY + 34, 11, RED);
                            } else if (!motherShip.unlockedQuantum) {
                                bool canAfford = motherShip.isotopicResources >= 250;
                                Color btnCol = canAfford ? (hover ? GREEN : Fade(GREEN, 0.8f)) : GRAY;
                                DrawRectangleRec(btn, hover && canAfford ? Fade(GREEN, 0.2f) : BLANK);
                                DrawRectangleLinesEx(btn, hover ? 2 : 1, btnCol);
                                DrawText("DESBLOQ (250)", btn.x + 12, btn.y + 8, 12, btnCol);
                            } else {
                                Color btnCol = motherShip.equippedQuantum ? CYAN : GRAY;
                                if (hover) btnCol = RAYWHITE;
                                DrawRectangleRec(btn, motherShip.equippedQuantum ? Fade(CYAN, 0.2f) : (hover ? Fade(GRAY, 0.1f) : BLANK));
                                DrawRectangleLinesEx(btn, hover ? 2 : 1, btnCol);
                                DrawText(motherShip.equippedQuantum ? "EQUIPADO" : "EQUIPAR", btn.x + 28, btn.y + 8, 12, btnCol);
                            }
                        }
                        
                        // Double check synergy warning at bottom
                        if (motherShip.equippedBounce && motherShip.equippedPiercing) {
                            DrawText("SINERGIA: AGUJERO NEGRO ACTIVA (Bounce + Pierce)", panelX + 20, startY + 375, 11, GOLD);
                            DrawText("Muros crean vortices de atraccion de plasma.", panelX + 20, startY + 390, 11, GOLD);
                        } else {
                            DrawText("Combina Rebote + Perforacion para Sinergia", panelX + 20, startY + 375, 11, GRAY);
                            DrawText("de Atraccion de Agujeros Negros de Plasma.", panelX + 20, startY + 390, 11, GRAY);
                        }
                    }
                    
                    // Notice to close terminal
                    DrawText("PULSA [ESC] O [E] PARA SALIR DE LA TERMINAL", panelX + 20, startY + 413, 11, GRAY);
                }
                
                // ==================== RIGHT PANEL: PLANETS ====================
                else if (activeNexusOverlay == 2) {
                    // Darken background overlay
                    DrawRectangle(0, 75, screenWidth, screenHeight - 75, Fade(BLACK, 0.5f));
                    
                    int mapX = screenWidth / 2 - 200;
                    int rightPanelW = 400;
                    int planetStartY = 110;
                    
                    // Destination Panel
                    DrawRectangle(mapX, planetStartY, rightPanelW, 230, Fade(BLACK, 0.8f));
                    DrawRectangleLines(mapX, planetStartY, rightPanelW, 230, Fade(CYAN, 0.6f));
                    DrawText("SELECCIONAR PLANETA DE DESTINO", mapX + 20, planetStartY + 15, 15, GOLD);
                    DrawLine(mapX + 20, planetStartY + 35, mapX + rightPanelW - 20, planetStartY + 35, Fade(CYAN, 0.3f));
                    
                    const char *planetNames[4] = { "AETHER (Rango: 4.8 LY)", "SOLARIS-IX (Rango: 5.0 LY)", "ZUL-GHAR (Rango: 11.5 LY)", "CYON-IV (Rango: 28.2 LY)" };
                    const char *planetSpecs[4] = {
                        "AETHER: Gravedad 1.0G. Atmosfera segura. Traccion estable.",
                        "SOLARIS: Gravedad 1.2G. Tormentas solares severas (drena escudo).",
                        "ZUL-GHAR: Gravedad 0.9G. Niebla acida corrosiva (danio directo).",
                        "CYON-IV: Gravedad 0.6G. Frio extremo (lentitud constante)."
                    };
                    
                    for (int p = 0; p < 4; p++) {
                        Rectangle btn = { (float)(mapX + 20), (float)(planetStartY + 45 + p * 34), 360.0f, 28.0f };
                        bool hover = CheckCollisionPointRec(mousePos, btn);
                        
                        bool unlocked = false;
                        if (p == 0 || p == 1) unlocked = (motherShip.engineRoom.level >= 1);
                        else if (p == 2) unlocked = (motherShip.engineRoom.level >= 2);
                        else if (p == 3) unlocked = (motherShip.engineRoom.level >= 3);
                        
                        Color txtCol = unlocked ? (p == selectedPlanetIdx ? CYAN : GRAY) : DARKGRAY;
                        if (hover && unlocked) txtCol = RAYWHITE;
                        
                        DrawRectangleRec(btn, p == selectedPlanetIdx ? Fade(CYAN, 0.15f) : (hover && unlocked ? Fade(CYAN, 0.05f) : BLANK));
                        DrawRectangleLinesEx(btn, p == selectedPlanetIdx ? 2 : 1, p == selectedPlanetIdx ? CYAN : Fade(txtCol, 0.4f));
                        
                        DrawText(planetNames[p], btn.x + 15, btn.y + 7, 13, txtCol);
                        if (!unlocked) {
                            DrawText("BLOQUEADO (Motor Lvl requerido)", btn.x + 180, btn.y + 7, 10, RED);
                        }
                    }
                    
                    // Show specifications of the selected planet
                    DrawText(planetSpecs[selectedPlanetIdx], mapX + 20, planetStartY + 195, 11, CYAN);
                    
                    // ==================== CENTER BOTTOM: LAUNCH BUTTON ====================
                    Rectangle btnLaunch = { (float)(screenWidth / 2 - 200), (float)(screenHeight - 165), 400.0f, 50.0f };
                    bool launchHover = CheckCollisionPointRec(mousePos, btnLaunch);
                    
                    float launchPulse = sinf((float)GetTime() * 6.0f) * 0.4f + 0.6f;
                    Color launchBtnCol = launchHover ? GOLD : Fade(CYAN, launchPulse);
                    DrawRectangleRec(btnLaunch, launchHover ? Fade(CYAN, 0.2f) : Fade(CYAN, 0.05f));
                    DrawRectangleLinesEx(btnLaunch, launchHover ? 3 : 2, launchBtnCol);
                    
                    const char *launchLabel = "INICIAR SECUENCIA DE LANZAMIENTO";
                    int lW = MeasureText(launchLabel, 16);
                    DrawText(launchLabel, btnLaunch.x + btnLaunch.width / 2 - lW / 2, btnLaunch.y + btnLaunch.height / 2 - 8, 16, launchBtnCol);
                    
                    DrawText("PULSA [ESC] O [E] PARA SALIR DE LA CONSOLA", screenWidth / 2 - 130, screenHeight - 105, 11, GRAY);
                }
                
                // If a dialog is active, draw dialogue panel at the bottom center of the screen
                if (activeCrewDialogIdx > 0 && activeNexusOverlay == 0) {
                    int boxW = 600;
                    int boxH = 110;
                    int boxX = screenWidth / 2 - boxW / 2;
                    int boxY = screenHeight - boxH - 25;
                    
                    DrawRectangle(boxX, boxY, boxW, boxH, Fade(BLACK, 0.85f));
                    
                    // Show Dialogue based on active crew and clone index
                    char quoteMsg[380] = { 0 };
                    Color quoteCol = RAYWHITE;
                    
                    if (activeCrewDialogIdx == 1) {
                        quoteCol = CYAN;
                        if (currentClone.cloneIndex <= 5) {
                            sprintf(quoteMsg, "IA: \"Clon #%d en linea. Sincronizacion neuronal al %.1f%%. Bienvenido de vuelta, Operador. Estabilidad de red estable.\"", currentClone.cloneIndex, currentClone.memoryCoherence);
                        } else if (currentClone.cloneIndex <= 20) {
                            sprintf(quoteMsg, "IA: \"Clon... #%d... impreso. Memoria aproximada al %.1f%%. Se detectan anomalias de redundancia en tu cortex.\"", currentClone.cloneIndex, currentClone.memoryCoherence);
                        } else {
                            sprintf(quoteMsg, "IA: \"ERROR DE COMPATIBILIDAD. Clon #%d impreso. %.1f%% de datos coherentes. Reciclando despojos orgánicos. ¿Por que sigues intentándolo?\"", currentClone.cloneIndex, currentClone.memoryCoherence);
                        }
                    } else if (activeCrewDialogIdx == 2) {
                        quoteCol = LIME;
                        if (currentClone.cloneIndex <= 5) {
                            sprintf(quoteMsg, "Cientifico: \"Los datos que recuperaste en tu vida anterior son utiles. Ten cuidado ahi fuera, Hepape.\"");
                        } else if (currentClone.cloneIndex <= 20) {
                            sprintf(quoteMsg, "Cientifico: \"¿Seguro que eres el mismo que bajo al planeta helado? Tus pupilas no reaccionan al espectro visible habitual...\"");
                        } else {
                            sprintf(quoteMsg, "Cientifico: \"No me mires con esos ojos impresos. Eres solo biomasa reformada numero %d. No sientes dolor, solo imitas el reflejo.\"", currentClone.cloneIndex);
                        }
                    } else if (activeCrewDialogIdx == 3) {
                        quoteCol = PINK;
                        if (currentClone.cloneIndex <= 5) {
                            sprintf(quoteMsg, "Soldado: \"Es un alivio verte de nuevo en pie. El vacio no nos vencera.\"");
                        } else if (currentClone.cloneIndex <= 20) {
                            sprintf(quoteMsg, "Soldado: \"Te mueves de forma extrana. ¿De verdad eres tu, o la maquina solo ha rellenado los huecos con datos basura?\"");
                        } else {
                            sprintf(quoteMsg, "Soldado: \"¿Quien era el original? Ya no queda nadie de la tripulacion inicial. Todos somos fotocopias de fotocopias...\"");
                        }
                    }
                    
                    DrawRectangleLinesEx((Rectangle){ (float)boxX, (float)boxY, (float)boxW, (float)boxH }, 2, quoteCol);
                    
                    float textGlitchAmt = (currentClone.cloneIndex <= 5) ? 0.0f : (currentClone.cloneIndex <= 20) ? 0.15f : 0.65f;
                    char line1[80] = { 0 };
                    char line2[80] = { 0 };
                    char line3[120] = { 0 };
                    
                    int quoteLen = strlen(quoteMsg);
                    if (quoteLen < 62) {
                        strcpy(line1, quoteMsg);
                    } else {
                        strncpy(line1, quoteMsg, 60);
                        int spaceIdx = 60;
                        while (spaceIdx > 0 && quoteMsg[spaceIdx] != ' ') spaceIdx--;
                        if (spaceIdx > 10) {
                            memset(line1, 0, sizeof(line1));
                            strncpy(line1, quoteMsg, spaceIdx);
                            strncpy(line2, quoteMsg + spaceIdx + 1, 60);
                            int spaceIdx2 = spaceIdx + 1 + 60;
                            int lastSpace = spaceIdx2;
                            while (lastSpace > spaceIdx + 1 && quoteMsg[lastSpace] != ' ') lastSpace--;
                            if (lastSpace > spaceIdx + 10) {
                                memset(line2, 0, sizeof(line2));
                                strncpy(line2, quoteMsg + spaceIdx + 1, lastSpace - (spaceIdx + 1));
                                strcpy(line3, quoteMsg + lastSpace + 1);
                            } else {
                                strcpy(line3, quoteMsg + spaceIdx2);
                            }
                        } else {
                            strncpy(line2, quoteMsg + 60, 60);
                            strcpy(line3, quoteMsg + 120);
                        }
                    }
                    
                    if (textGlitchAmt > 0.0f) {
                        DrawTextGlitch(line1, boxX + 20, boxY + 20, 13, quoteCol, textGlitchAmt);
                        if (strlen(line2) > 0) DrawTextGlitch(line2, boxX + 20, boxY + 45, 13, quoteCol, textGlitchAmt);
                        if (strlen(line3) > 0) DrawTextGlitch(line3, boxX + 20, boxY + 70, 13, quoteCol, textGlitchAmt);
                    } else {
                        DrawText(line1, boxX + 20, boxY + 20, 13, quoteCol);
                        if (strlen(line2) > 0) DrawText(line2, boxX + 20, boxY + 45, 13, quoteCol);
                        if (strlen(line3) > 0) DrawText(line3, boxX + 20, boxY + 70, 13, quoteCol);
                    }
                }
            }
            else if (currentScreen == SCREEN_INTRO) {
                // 1. Draw Starfield background
                BeginMode3D(camera);
                    for (int i = 0; i < MAX_STARS; i++) {
                        float driftX = spaceStars[i].position.x - camera.position.x * spaceStars[i].parallaxFactor;
                        float driftZ = spaceStars[i].position.z - camera.position.z * spaceStars[i].parallaxFactor;
                        Vector3 starPos = { driftX, spaceStars[i].position.y, driftZ };
                        DrawBillboardRec(camera, charSpritesheet, (Rectangle){ 96.0f, 192.0f, 32.0f, 32.0f }, starPos, (Vector2){ spaceStars[i].size, spaceStars[i].size }, spaceStars[i].color);
                    }
                EndMode3D();
                
                // Dark overlay to make text readable
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));
                
                // Render decorative borders (cyber sci-fi style HUD)
                DrawRectangleLines(30, 30, screenWidth - 60, screenHeight - 60, Fade(CYAN, 0.4f));
                DrawRectangleLines(34, 34, screenWidth - 68, screenHeight - 68, Fade(CYAN, 0.15f));
                
                // Story lines or Clone diagnosis depending on index
                if (currentClone.cloneIndex == 1) {
                    const char *storyLines[] = {
                        "REGISTRO DE MISION: NAVE DE INVESTIGACION USG DEHUMANIZER",
                        "FECHA ESTELAR: 2146.05.19",
                        "",
                        "Hace 48 horas, se perdio toda conexion con la nave medica insignia.",
                        "Un patogeno alienigena desconocido ha infestado los sistemas biologicos,",
                        "mutando a la tripulacion en aberraciones ciberneticas hostiles.",
                        "",
                        "Como ultimo miembro del escuadron de limpieza tactica:",
                        "Tu mision es infiltrarte en la nave a traves del muelle de carga,",
                        "purgar la infestacion de cada compartimento de combate,",
                        "y sobrecargar el nucleo del reactor principal para vaporizar la amenaza.",
                        "",
                        "INSTRUCCIONES DE SUPERVIVENCIA:",
                        "- Moverse: Teclas [W], [A], [S], [D]",
                        "- Apuntar y Disparar: Mover el MOUSE y boton CLIC IZQUIERDO",
                        "- Cambiar Sala: Cruza los pasillos cuando esten despejados",
                        "- Mejoras: Encuentra las salas del tesoro para aumentar tu potencia"
                    };
                    int numLines = 17;
                    int startY = screenHeight / 2 - 220;
                    if (startY < 45) startY = 45;
                    
                    for (int i = 0; i < numLines; i++) {
                        Color col = RAYWHITE;
                        int size = 18;
                        if (i == 0) { col = GOLD; size = 20; }
                        else if (i == 1) { col = CYAN; size = 15; }
                        else if (i >= 12) { col = LIME; size = 16; }
                        
                        int textW = MeasureText(storyLines[i], size);
                        
                        // Typewriter fade-in effect based on introTimer
                        float lineDelay = (float)i * 0.4f;
                        float lineProgress = (introTimer - lineDelay) * 2.0f;
                        if (lineProgress < 0.0f) lineProgress = 0.0f;
                        if (lineProgress > 1.0f) lineProgress = 1.0f;
                        
                        Color lineCol = Fade(col, lineProgress);
                        DrawText(storyLines[i], screenWidth / 2 - textW / 2, startY + i * 24, size, lineCol);
                    }
                } else {
                    // Clone diagnosis mode
                    char title[128];
                    char subtitle[128];
                    sprintf(title, "DIAGNOSTICO DE RE-IMPRESION ORGANICA (CLON #%03d)", currentClone.cloneIndex);
                    sprintf(subtitle, "COHERENCIA DE MEMORIA: %.1f%%  |  PARANOIA SINAPTICA: %.1f%%", currentClone.memoryCoherence, currentClone.paranoiaLevel);
                    
                    char msgIA[256];
                    char msgCien[256];
                    char msgSold[256];
                    
                    Color statusColor = GREEN;
                    if (currentClone.cloneIndex <= 5) {
                        sprintf(msgIA, "IA DE A BORDO: \"Clon #%d en linea. Sincronizacion neuronal al %.1f%%. Bienvenido, Operador.\"", currentClone.cloneIndex, currentClone.memoryCoherence);
                        sprintf(msgCien, "CIENTIFICO: \"Los datos que recuperaste en tu vida anterior son utiles. Ten cuidado ahi fuera, Hepape.\"");
                        sprintf(msgSold, "SOLDADO: \"Es un alivio verte de nuevo en pie. El vacio no nos vencera.\"");
                    } else if (currentClone.cloneIndex <= 20) {
                        statusColor = ORANGE;
                        sprintf(msgIA, "IA DE A BORDO: \"Clon... #%d... impreso. Memoria al %.1f%%. Se detectan anomalias de redundancia.\"", currentClone.cloneIndex, currentClone.memoryCoherence);
                        sprintf(msgCien, "CIENTIFICO: \"¿Seguro que eres el mismo? Tus pupilas no reaccionan al espectro visible habitual...\"");
                        sprintf(msgSold, "SOLDADO: \"Te mueves raro. ¿De verdad eres tu, o la maquina solo relleno los huecos con datos basura?\"");
                    } else {
                        statusColor = RED;
                        sprintf(msgIA, "IA DE A BORDO: \"ERROR DE COMPATIBILIDAD. Clon #%d impreso. %.1f%% coherencia. Reciclando despojos...\"", currentClone.cloneIndex, currentClone.memoryCoherence);
                        sprintf(msgCien, "CIENTIFICO: \"No me mires con esos ojos impresos. Eres solo biomasa reformada #%d. Solo imitas el reflejo.\"", currentClone.cloneIndex);
                        sprintf(msgSold, "SOLDADO: \"¿Quien era el original? Ya no queda nadie de la tripulacion inicial. Fotocopias de fotocopias...\"");
                    }
                    
                    int startY = screenHeight / 2 - 180;
                    if (startY < 45) startY = 45;
                    
                    // Render title
                    int textW = MeasureText(title, 20);
                    DrawText(title, screenWidth / 2 - textW / 2, startY, 20, statusColor);
                    
                    // Render subtitle
                    textW = MeasureText(subtitle, 16);
                    DrawText(subtitle, screenWidth / 2 - textW / 2, startY + 30, 16, RAYWHITE);
                    
                    // Line separator
                    DrawLine(50, startY + 60, screenWidth - 50, startY + 60, Fade(statusColor, 0.5f));
                    
                    // Crew dialogue quotes
                    const char* quotes[6] = {
                        "--- TRANSMISIONES DE LA NAVE ---",
                        "",
                        msgIA,
                        msgCien,
                        msgSold,
                        ""
                    };
                    
                    for (int i = 0; i < 6; i++) {
                        Color col = RAYWHITE;
                        int size = 16;
                        if (i == 0) { col = GOLD; size = 16; }
                        else if (i == 2) { col = CYAN; }
                        else if (i == 3) { col = LIME; }
                        else if (i == 4) { col = PINK; }
                        
                        // Glitch text for dialogue if clone stage is high
                        float glitchAmt = (currentClone.cloneIndex <= 5) ? 0.0f : (currentClone.cloneIndex <= 20) ? 0.15f : 0.75f;
                        
                        int textW = MeasureText(quotes[i], size);
                        if (glitchAmt > 0.0f && i >= 2) {
                            DrawTextGlitch(quotes[i], screenWidth / 2 - textW / 2, startY + 90 + i * 32, size, col, glitchAmt);
                        } else {
                            DrawText(quotes[i], screenWidth / 2 - textW / 2, startY + 90 + i * 32, size, col);
                        }
                    }
                }
                
                // Pulsing indicator to continue
                if (introTimer > 1.5f) {
                    float pulse = sinf(introTimer * 4.0f) * 0.5f + 0.5f;
                    const char *prompt = "PRESIONA ENTER O CLIC PARA INICIAR LA OPERACION";
                    int pW = MeasureText(prompt, 20);
                    DrawText(prompt, screenWidth / 2 - pW / 2, screenHeight - 90, 20, Fade(GOLD, pulse));
                }
            }
            else {
                Room &room = dungeon[currentRoomY][currentRoomX];
                Vector3 groundAim = GetMouseGroundIntersection(camera);
                Vector3 mouseIntersect = groundAim;
                
                Color roomTint = WHITE;
                if (currentPlanet.hazard == HAZARD_SOLAR_STORM) {
                    roomTint = (Color){ 255, 210, 180, 255 };
                } else if (currentPlanet.hazard == HAZARD_TOXIC_FOG) {
                    roomTint = (Color){ 190, 255, 190, 255 };
                } else if (currentPlanet.hazard == HAZARD_FROZEN_WASTE) {
                    roomTint = (Color){ 180, 220, 255, 255 };
                }
                
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
                            DrawFloorTile(envSpritesheet, tileSrc, (Vector3){ px, 0.0f, pz }, (Vector2){ 1.0f, 1.0f }, roomTint);
                        }
                    }
                    
                    // --- 3. PARED LAYER (Compartment solid columns) ---
                    for (int z = 0; z < ROOM_GRID_SIZE; z++) {
                        for (int x = 0; x < ROOM_GRID_SIZE; x++) {
                            float px = (float)(x - 10);
                            float pz = (float)(z - 10);
                            
                            if (room.wallTileVariants[z][x] >= 0) {
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
                                        DrawFloorTile(envSpritesheet, openPathSrc, (Vector3){ px, 0.0f, pz }, (Vector2){ 1.0f, 1.0f }, roomTint);
                                    }
                                } else {
                                    DrawWallBlock(envSpritesheet, wallSrc, (Vector3){ px, 2.0f, pz }, (Vector3){ 1.0f, 4.0f, 1.0f }, roomTint);
                                }
                            }
                        }
                    }
                    
                    for (int i = 0; i < room.numPillars; i++) {
                        Rectangle pillarSrc = { (float)tileOffsetCol * 32.0f, 0.0f, 32.0f, 32.0f };
                        DrawWallBlock(envSpritesheet, pillarSrc, room.pillars[i], (Vector3){ 1.6f, 4.0f, 1.6f }, roomTint);
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
                        AddBillboardToRender(legPos, charSpritesheet, legSrc, (Vector2){ 1.8f, 1.8f }, tint, 1, camera, 0.0f);
                        
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
                        AddBillboardToRender(headPos, charSpritesheet, headSrc, (Vector2){ 1.8f, 1.8f }, tint, 1, camera, -0.01f);
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
                    
                    // Draw Swarm Heart drones in 3D space
                    if (activeRelics[1].active) {
                        float time = (float)GetTime() * 3.0f;
                        for (int d = 0; d < 3; d++) {
                            float angle = time + (float)d * 2.0f * PI / 3.0f;
                            Vector3 dronePos = {
                                player.position.x + cosf(angle) * 1.2f,
                                player.position.y + 0.4f + sinf(time * 2.0f + (float)d) * 0.15f,
                                player.position.z + sinf(angle) * 1.2f
                            };
                            DrawSphere(dronePos, 0.15f, LIME);
                            if (GetRandomValue(0, 100) < 30) {
                                SpawnParticles(dronePos, LIME, 1);
                            }
                        }
                    }
                    
                EndMode3D();
                
                // --- 2D DYNAMIC LIGHTING PASS ---
                if (GetCurrentState() == SCREEN_GAMEPLAY) {
                    if (useRenderTarget) EndTextureMode();
                    
                    BeginTextureMode(lightMap);
                    ClearBackground((Color){ 10, 10, 15, 255 }); // Dark ambient base
                    
                    BeginBlendMode(BLEND_ADDITIVE);
                    
                    // Player Light
                    Vector2 playerScreenPos = GetWorldToScreen(player.position, camera);
                    DrawCircleGradient(playerScreenPos.x, playerScreenPos.y, 250.0f, Fade(YELLOW, 0.45f), BLANK);
                    
                    // Projectile Lights
                    for (int i = 0; i < MAX_PROJECTILES; i++) {
                        if (projectiles[i].active) {
                            Vector2 projScreenPos = GetWorldToScreen(projectiles[i].position, camera);
                            Color lightCol = projectiles[i].isAcid ? GREEN : (projectiles[i].isEnemy ? RED : CYAN);
                            DrawCircleGradient(projScreenPos.x, projScreenPos.y, 110.0f, Fade(lightCol, 0.65f), BLANK);
                        }
                    }
                    
                    // Particle Lights
                    for (int i = 0; i < MAX_PARTICLES; i++) {
                        if (particles[i].active && !particles[i].isAtmospheric && !particles[i].isGas) {
                            Vector2 partScreenPos = GetWorldToScreen(particles[i].position, camera);
                            DrawCircleGradient(partScreenPos.x, partScreenPos.y, 50.0f, Fade(particles[i].color, 0.5f), BLANK);
                        }
                    }
                    
                    EndBlendMode();
                    EndTextureMode();
                    
                    if (useRenderTarget) BeginTextureMode(targetTex);
                    
                    // Apply Lightmap via Multiplicative Blending
                    BeginBlendMode(BLEND_MULTIPLIED);
                    DrawTexturePro(lightMap.texture, 
                                   (Rectangle){ 0.0f, 0.0f, (float)lightMap.texture.width, (float)-lightMap.texture.height },
                                   (Rectangle){ 0.0f, 0.0f, (float)screenWidth, (float)screenHeight },
                                   (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                    EndBlendMode();
                }
                
                // --- 2D OVERLAYS & HUD UI ---
                if (currentPlanet.hazard != HAZARD_NONE) {
                    DrawRectangle(0, 0, screenWidth, screenHeight, currentPlanet.atmosphericTint);
                }
                
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
                
                // Draw Dehumanizer HUD!
                DrawDehumanizerHUD(currentClone, playerHalfHeartsHealth * 10, playerMaxHearts * 20, 0, gameTimer);
                
                // Draw Atmospheric Hazard HUD
                if (currentPlanet.hazard != HAZARD_NONE) {
                    int hazY = 135;
                    DrawRectangle(screenWidth - 250, hazY, 210, 85, Fade(BLACK, 0.6f));
                    DrawRectangleLines(screenWidth - 250, hazY, 210, 85, DARKGRAY);
                    Color hazardCol = RED;
                    const char* hazName = "DESCONOCIDO";
                    if (currentPlanet.hazard == HAZARD_SOLAR_STORM) {
                        hazName = "TORMENTA SOLAR";
                        hazardCol = ORANGE;
                    } else if (currentPlanet.hazard == HAZARD_TOXIC_FOG) {
                        hazName = "NIEBLA TOXICA";
                        hazardCol = LIME;
                    } else if (currentPlanet.hazard == HAZARD_FROZEN_WASTE) {
                        hazName = "PLANETA HELADO";
                        hazardCol = SKYBLUE;
                    }
                    
                    float glitch = currentClone.cloneIndex > 20 ? 0.35f : 0.0f;
                    DrawTextGlitch(TextFormat("PELIGRO: %s", hazName), screenWidth - 235, hazY + 8, 12, hazardCol, glitch);
                    
                    DrawText(TextFormat("TRAJE INTEGRIDAD: %.0f%%", player.suitIntegrity), screenWidth - 235, hazY + 28, 11, player.suitIntegrity > 25.0f ? WHITE : RED);
                    DrawRectangle(screenWidth - 235, hazY + 41, 180, 6, BLACK);
                    DrawRectangle(screenWidth - 235, hazY + 41, (int)(1.8f * player.suitIntegrity), 6, hazardCol);
                    
                    DrawText(TextFormat("OXIGENO RESERVA: %.0f%%", player.oxygenLevel), screenWidth - 235, hazY + 54, 11, player.oxygenLevel > 25.0f ? CYAN : RED);
                    DrawRectangle(screenWidth - 235, hazY + 67, 180, 6, BLACK);
                    DrawRectangle(screenWidth - 235, hazY + 67, (int)(1.8f * player.oxygenLevel), 6, CYAN);
                }
                
                // Mothership meta-progression HUD
                int msY = 230;
                DrawRectangle(screenWidth - 250, msY, 210, 95, Fade(BLACK, 0.6f));
                DrawRectangleLines(screenWidth - 250, msY, 210, 95, DARKGRAY);
                DrawText("NAVE NODRIZA (Meta)", screenWidth - 235, msY + 8, 12, GOLD);
                DrawText(TextFormat("Invernadero: Lvl %d", motherShip.greenhouse.level), screenWidth - 235, msY + 28, 11, GREEN);
                DrawText(TextFormat("Armeria: Lvl %d", motherShip.armory.level), screenWidth - 235, msY + 44, 11, RED);
                DrawText(TextFormat("Motores: Lvl %d", motherShip.engineRoom.level), screenWidth - 235, msY + 60, 11, CYAN);
                DrawText(TextFormat("Recursos: %d Isotopos", motherShip.isotopicResources), screenWidth - 235, msY + 76, 11, ORANGE);
                
                // Active Upgrades / Relics HUD Icons
                int upgY = 230;
                for (int r = 0; r < 3; r++) {
                    if (activeRelics[r].active) {
                        DrawRectangle(40, upgY, 220, 24, Fade(BLACK, 0.6f));
                        DrawRectangleLines(40, upgY, 220, 24, PURPLE);
                        DrawText(TextFormat("REL: %s", activeRelics[r].name), 46, upgY + 5, 11, VIOLET);
                        upgY += 28;
                    }
                }
                if (hasCyberEye) {
                    DrawRectangle(40, upgY, 220, 24, Fade(BLACK, 0.6f));
                    DrawText("MEJORA: OJO CIBERNETICO", 46, upgY + 5, 11, LIME);
                    upgY += 28;
                }
                if (hasThrusterBoots) {
                    DrawRectangle(40, upgY, 220, 24, Fade(BLACK, 0.6f));
                    DrawText("MEJORA: BOTAS PROPULSORAS", 46, upgY + 5, 11, CYAN);
                    upgY += 28;
                }
                if (hasAcidGlands) {
                    DrawRectangle(40, upgY, 220, 24, Fade(BLACK, 0.6f));
                    DrawText("MEJORA: GLANDULAS DE ACIDO", 46, upgY + 5, 11, GREEN);
                    upgY += 28;
                }
                if (player.activeWeapon.projectileSlot.hasBounce) {
                    DrawRectangle(40, upgY, 220, 24, Fade(BLACK, 0.6f));
                    DrawRectangleLines(40, upgY, 220, 24, SKYBLUE);
                    DrawText("MOD: REBOTE GRAVITATORIO", 46, upgY + 5, 11, SKYBLUE);
                    upgY += 28;
                }
                if (player.activeWeapon.modifierSlot.hasPiercing) {
                    DrawRectangle(40, upgY, 220, 24, Fade(BLACK, 0.6f));
                    DrawRectangleLines(40, upgY, 220, 24, RED);
                    DrawText("MOD: PERFORACION PLASMA", 46, upgY + 5, 11, RED);
                    upgY += 28;
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
                
                float textGlitch = currentClone.cloneIndex > 20 ? 0.4f : currentClone.cloneIndex > 5 ? 0.1f : 0.0f;
                DrawTextGlitch(TextFormat("TIME: %.1fs", gameTimer), 40, 135, 22, GOLD, textGlitch);
                const char *typeStr = (room.type == ROOM_START) ? "COMPARTIMENTO DE ENTRADA (Seguro)" :
                                      (room.type == ROOM_TREASURE) ? "ZONA DE CARGA DEL TESORO" :
                                      (room.type == ROOM_BOSS) ? "¡NUCLEO DEL JEFE MUTANTE!" : "COMPARTIMENTO DE COMBATE";
                DrawTextGlitch(typeStr, 40, 165, 18, CYAN, textGlitch);
                
                if (!room.cleared) DrawTextGlitch("¡COMPARTIMENTOS SELLADOS! Purga la infestation.", 40, 195, 16, RED, textGlitch);
                else DrawTextGlitch("Zona purgada. Usa las compuertas WASD para avanzar.", 40, 195, 16, LIME, textGlitch);
                
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
                else if (currentScreen == SCREEN_ROOM_TRANSITION) {
                    float alpha = 0.0f;
                    if (transitionTimer < 0.5f) {
                        alpha = transitionTimer / 0.5f;
                    } else {
                        alpha = 1.0f - (transitionTimer - 0.5f) / 0.5f;
                    }
                    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, alpha));
                }
            }
            
            if (useRenderTarget) {
                EndTextureMode();
                
                BeginDrawing();
                ClearBackground(BLACK);
                BeginShaderMode(activeShader);
                    DrawTexturePro(targetTex.texture, 
                        (Rectangle){ 0.0f, 0.0f, (float)targetTex.texture.width, (float)-targetTex.texture.height },
                        (Rectangle){ 0.0f, 0.0f, (float)screenWidth, (float)screenHeight },
                        (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                EndShaderMode();
            }
            
            DrawFPS(screenWidth - 80, screenHeight - 40);
        EndDrawing();
    }
    
    // --- CLEANUP ---
    UnloadModel(floorModel);
    UnloadModel(cubeModel);
    UnloadTexture(envSpritesheet);
    UnloadTexture(charSpritesheet);
    
    UnloadShader(nebulaShader);
    UnloadShader(tachyonShader);
    UnloadRenderTexture(targetTex);
    UnloadRenderTexture(lightMap);
    
    CloseWindow();
    return 0;
}
