import re

code = open('main.cpp').read()

# 1. Add flowField to Room struct
code = code.replace("int wallTileVariants[ROOM_GRID_SIZE][ROOM_GRID_SIZE];", "int wallTileVariants[ROOM_GRID_SIZE][ROOM_GRID_SIZE];\n    int flowField[ROOM_GRID_SIZE][ROOM_GRID_SIZE];")

# 2. Add UpdateFlowField helper before main()
helper = """
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

int main(void) {"""

code = code.replace("int main(void) {", helper)

# 3. Update the enemy AI to use Flow Fields instead of Vector3Subtract
# Search for the enemy movement block in main loop

ai_logic = """                    // Use Flow Field AI navigation for movement
                    Vector3 moveDir = GetFlowFieldDirection(currentRoom, enemy.position);
                    if (moveDir.x == 0.0f && moveDir.z == 0.0f) {
                        // Fallback naive steering if lost
                        moveDir = Vector3Normalize(Vector3Subtract(target, enemy.position));
                    }
                    
                    // Simple Boids avoidance (separation) from other enemies
                    Vector3 separation = {0,0,0};
                    int neighbors = 0;
                    for (int j = 0; j < currentRoom.numEnemies; j++) {
                        if (e != j && currentRoom.enemies[j].health > 0) {
                            Vector3 otherPos = currentRoom.enemies[j].position;
                            float dX = enemy.position.x - otherPos.x;
                            float dZ = enemy.position.z - otherPos.z;
                            float dSq = dX*dX + dZ*dZ;
                            if (dSq > 0.001f && dSq < 1.0f) {
                                separation.x += dX / dSq;
                                separation.z += dZ / dSq;
                                neighbors++;
                            }
                        }
                    }
                    if (neighbors > 0) {
                        moveDir.x += separation.x * 0.3f;
                        moveDir.z += separation.z * 0.3f;
                        moveDir = Vector3Normalize(moveDir);
                    }
                    
                    enemy.position.x += moveDir.x * eSpeed * dt;
                    enemy.position.z += moveDir.z * eSpeed * dt;"""

code = re.sub(r'                    Vector3 moveDir = Vector3Normalize\(Vector3Subtract\(target, enemy.position\)\);\n                    enemy.position.x \+= moveDir.x \* eSpeed \* dt;\n                    enemy.position.z \+= moveDir.z \* eSpeed \* dt;', ai_logic, code)

# 4. We must call UpdateFlowField(currentRoom, player.position) at the start of gameplay update loop
ff_update = """            // Screen-Shake decay
            if (screenShake > 0.0f) {
                screenShake -= 2.0f * GetFrameTime();
                if (screenShake < 0.0f) screenShake = 0.0f;
            }
            
            // Generate Pathfinding Flow Field to Player
            if (currentRoom.numEnemies > 0 && !currentRoom.cleared) {
                UpdateFlowField(currentRoom, player.position);
            }
            """
code = code.replace("            // Screen-Shake decay\n            if (screenShake > 0.0f) {\n                screenShake -= 2.0f * GetFrameTime();\n                if (screenShake < 0.0f) screenShake = 0.0f;\n            }", ff_update)

open('main.cpp', 'w').write(code)
