import re

code = open('main.cpp').read()

# 1. Insert Cellular Automata Map Gen and Collision Helpers
helpers = """
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
"""

code = code.replace("// Procedural Spaceship Dungeon Generator\n", helpers)


# 2. Modify GenerateProceduralDungeon to call CarveOrganicRoom
carve_call = """        int rx = spawnedCoords[i].x;
        int ry = spawnedCoords[i].y;
        
        CarveOrganicRoom(dungeon[ry][rx]);
        
        // Spawn pillars"""

code = re.sub(r'        int rx = spawnedCoords\[i\]\.x;\n        int ry = spawnedCoords\[i\]\.y;\n\s+// Spawn pillars', carve_call, code)

# Also apply CarveOrganicRoom to Boss and Treasure room logic, which was skipped because they might not have spawned pillars.
# Actually, the pillar spawn loop iterates through ALL spawnedCoords:
# for (int i = 0; i < spawnedCount; i++)
# So adding it there covers ALL active rooms!


# 3. Modify drawing logic to use room.wallTileVariants[z][x] >= 0
draw_logic = """                            if (room.wallTileVariants[z][x] >= 0) {
                                bool isDoorway = false;
                                if (z == 0 && x == 10 && room.doors[0]) isDoorway = true;
                                if (z == 20 && x == 10 && room.doors[1]) isDoorway = true;
                                if (x == 0 && z == 10 && room.doors[2]) isDoorway = true;
                                if (x == 20 && z == 10 && room.doors[3]) isDoorway = true;
                                
                                int wallCol = tileOffsetCol + room.wallTileVariants[z][x];"""

code = re.sub(r'                            bool isBorder = \(x == 0 \|\| x == 20 \|\| z == 0 \|\| z == 20\);\n\s+if \(isBorder\) {\n\s+bool isDoorway = false;\n\s+if \(z == 0 && x == 10 && room.doors\[0\]\) isDoorway = true;\n\s+if \(z == 20 && x == 10 && room.doors\[1\]\) isDoorway = true;\n\s+if \(x == 0 && z == 10 && room.doors\[2\]\) isDoorway = true;\n\s+if \(x == 20 && z == 10 && room.doors\[3\]\) isDoorway = true;\n\s+int wallCol = tileOffsetCol \+ room\.wallTileVariants\[z\]\[x\];', draw_logic, code)


# 4. Modify collisions
# Player
code = code.replace("// Pillars collisions", "ResolveTileCollisions(player.position, player.radius, currentRoom);\n            \n            // Pillars collisions")

# Enemy
code = code.replace("// Check bounds", "ResolveTileCollisions(enemy.position, 0.4f, currentRoom);\n                    \n                    // Check bounds")

# Projectile
proj_col = """                    if (CheckTileCollision(projectiles[i].position, 0.2f, currentRoom)) {
                        projectiles[i].active = false;
                        SpawnParticles(projectiles[i].position, projectiles[i].isEnemy ? RED : CYAN, 5);
                        continue;
                    }
                    
                    // Wall bounds"""
code = code.replace("// Wall bounds", proj_col)

open('main.cpp', 'w').write(code)
