// --- CONSTANTS ---
const char TERMINATOR = '\n';
const char SEPARATOR = ' ';
const int MAX_SIZE = 20;

// Ship sizes we are tracking (index 0 and 1 are unused for simple indexing)
const int SHIP_SIZES[] = {0, 0, 2, 3, 4}; // Ship sizes are 2, 3, and 4
const int NUM_SHIP_TYPES = 3; // Corresponds to sizes 2, 3, 4

// --- GLOBAL VARIABLES ---
int MAP_WIDTH = 0;
int MAP_HEIGHT = 0;
char gameMap[MAX_SIZE][MAX_SIZE]; 
char publicMap[MAX_SIZE][MAX_SIZE];

int hitsRemaining = 0;
char WATER = '~';
char HIT = 'X';
char MISS = 'M';

// Array to store the COUNT of ships of each size (index 2 for size 2, etc.)
int shipCounts[5] = {0}; // shipCounts[2] = count of size-2 ships, shipCounts[3] = count of size-3 ships, etc.


bool initializeGame();
void placeShipsRandomly();
void printMap();
void handleAttack();
bool isShipSunk(int , int , char);

// --- CORE FUNCTION: CHECK FOR GLOBAL COMMANDS (RESTART) ---
bool checkGlobalCommand() {
    // We only read a short string, expecting a simple command like "restart"
    String inputString = Serial.readStringUntil(TERMINATOR);
    inputString.trim();
    
    // Convert to lowercase for case-insensitivity
    inputString.toLowerCase();

    if (inputString.equals("restart")) {
        // Resetting the game state by setting the initialization flag back to zero
        MAP_WIDTH = 0;
        MAP_HEIGHT = 0;
        hitsRemaining = 0;
        // Optionally, reset shipCounts array if needed, but it will be overwritten in initializeGame()
        
        Serial.println("RST"); // RST for Restart Acknowledged
        Serial.println("TORPEDO GAME RESTARTING...");
        Serial.println("STEP 1: Send five numbers: WIDTH HEIGHT S2_COUNT S3_COUNT S4_COUNT (e.g., 10 10 2 1 1)");
        return true; // Command was processed
    }
    
    // Add other global commands here if needed (e.g., "status")
    
    // If we read something that wasn't a recognized global command, 
    // we should let the main loop know that the input was consumed.
   // if (inputString.length() > 0) {
    //    return true;
    //}
    return false; // No command found
}


// --- SETUP ---

void setup() {
  Serial.begin(115200); 
  Serial.setTimeout(500);
 // ROBUST SEEDING: Read the floating pin multiple times
  long seed = 0;
  for (int i = 0; i < 10; i++) {
    seed += analogRead(A0);
    delay(1); // Introduce some micro-delay jitter
  }
  randomSeed(seed);
  
  Serial.println("TORPEDO GAME INITIALIZATION");
  Serial.println("---------------------------------");
  Serial.println("STEP 1: Send five numbers: WIDTH HEIGHT S2_COUNT S3_COUNT S4_COUNT (e.g., 10 10 2 1 1)");
}








// --- MAIN LOOP (CORRECTED) ---
void loop() {
  // Phase 1: Initialization (Only runs when the map hasn't been set up)
  if (MAP_WIDTH == 0) { 
    if (Serial.available() > 0) {
      // Attempt to initialize the game with the first set of input
      if (initializeGame()) {
          // If successful, proceed to placement
          placeShipsRandomly();
          Serial.println("--- Ship placement complete! ---");
          printMap();
          
          // Now that the game is initialized, we tell the user what to do next.
          Serial.println("GAME STARTED! Send coordinates (COLUMN ROW, e.g., 4 5) to fire a torpedo.");
      }
      // CRITICAL: If MAP_WIDTH is still 0 (due to error or failed read), 
      // the loop will try initialization again on the next cycle if input remains.
    }
  } 
  
  // Phase 2: Gameplay (Only runs AFTER the map has been set up)
  else { // MAP_WIDTH > 0
  if (Serial.available() > 0) {
    if (Serial.peek() == 'r' || Serial.peek() == 'R') {
                 if (checkGlobalCommand()) {
                    return; // Restart command handled. Return to loop for new setup.
                 }
            }
        if (hitsRemaining > 0) {
            handleAttack();
        } else {
            Serial.println("\n*** CONGRATULATIONS! ALL SHIPS SUNK! GAME OVER ***");
            // Clear input buffer to prevent repeating the message
            while(Serial.available() > 0) Serial.read(); 
        }
    }
  }
}





// --- CORE FUNCTION: READ INPUT AND CREATE MAP/INVENTORY ---
bool initializeGame() {
  // 1. READ THE ENTIRE LINE INTO A STRING
  String inputString = Serial.readStringUntil(TERMINATOR);
  inputString.trim(); // Clean up trailing \r
  
  // 2. PARSE THE STRING (5 NUMBERS TOTAL)
  int index = 0;
  int lastIndex = 0;
  int values[5] = {0}; // Temp array to hold W, H, C2, C3, C4
  int valueCount = 0;
  
  while (index != -1 && valueCount < 5) {
    index = inputString.indexOf(SEPARATOR, lastIndex);
    
    String numberString;
    if (index == -1) {
      numberString = inputString.substring(lastIndex); 
    } else {
      numberString = inputString.substring(lastIndex, index); 
    }
    
    numberString.trim();
    if (numberString.length() > 0) {
      values[valueCount] = numberString.toInt();
      valueCount++;
    }
    lastIndex = index + 1;
  }
  
  // 3. VALIDATE AND ASSIGN ALL VALUES
  if (valueCount == 5) {
    MAP_WIDTH = values[0];
    MAP_HEIGHT = values[1];
    shipCounts[2] = values[2]; // Count for size 2
    shipCounts[3] = values[3]; // Count for size 3
    shipCounts[4] = values[4]; // Count for size 4
    // Calculate total segments
    int totalShipSegments = (shipCounts[2] * 2) + (shipCounts[3] * 3) + (shipCounts[4] * 4);
    
    // **<- INITIALIZE IT HERE ->**
    hitsRemaining = totalShipSegments;
    if (MAP_WIDTH <= 0 || MAP_HEIGHT <= 0 || MAP_WIDTH > MAX_SIZE || MAP_HEIGHT > MAX_SIZE) {
        Serial.println("ERROR: Invalid map dimensions (must be 1-20).");
        MAP_WIDTH = 0; MAP_HEIGHT = 0; return false;
    }
    
    // Initialize the 2D array: Fill every cell with 'W' for Water.
    for (int r = 0; r < MAX_SIZE; r++) {
      for (int c = 0; c < MAX_SIZE; c++) {
        gameMap[r][c] = '~'; 
        publicMap[r][c] = '~'; 
      }
    }
    
    Serial.print("Game Initialized: Map "); Serial.print(MAP_WIDTH); Serial.print("x"); Serial.println(MAP_HEIGHT);
    Serial.print("Ships: Size 2 x"); Serial.print(shipCounts[2]);
    Serial.print(" | Size 3 x"); Serial.print(shipCounts[3]);
    Serial.print(" | Size 4 x"); Serial.println(shipCounts[4]);
    return true;
    
  } else {
    Serial.println("ERROR: Failed to read five valid numbers (W H S2 S3 S4).");
    return false;
  }
}

// --- CORE FUNCTION: RANDOM SHIP PLACEMENT ---
void placeShipsRandomly() {
  Serial.println("\n-- Starting Ship Placement --");
  
  // Iterate through the three ship sizes: 2, 3, and 4
  for (int sizeIndex = 2; sizeIndex <= 4; sizeIndex++) {
    int shipSize = sizeIndex;
    int count = shipCounts[shipSize];
    
    for (int i = 0; i < count; i++) {
      bool placed = false;
      int attempts = 0;

      // Precompute feasibility
      bool canPlaceHoriz = (shipSize <= MAP_WIDTH);
      bool canPlaceVert = (shipSize <= MAP_HEIGHT);
      if (!canPlaceHoriz && !canPlaceVert) {
        Serial.print("WARNING: Ship size-"); Serial.print(shipSize); Serial.println(" cannot fit on the map at all.");
        break; // skip placing these ships
      }
      
      // Try to place the ship up to 100 times before giving up
      while (!placed && attempts < 100) {
        attempts++;
        
        // Choose orientation, but respect feasibility
        long orientation;
        if (canPlaceHoriz && canPlaceVert) {
          orientation = random(0, 2); // 0=H,1=V
        } else if (canPlaceHoriz) {
          orientation = 0;
        } else {
          orientation = 1;
        }
        
        // Find a random starting position (r, c)
        int startR = 0, startC = 0;
        if (orientation == 0) { // Horizontal
          int maxStartC = MAP_WIDTH - shipSize + 1;
          if (maxStartC <= 0) { continue; } // defensive
          startR = random(MAP_HEIGHT);
          startC = random(maxStartC);
        } else { // Vertical
          int maxStartR = MAP_HEIGHT - shipSize + 1;
          if (maxStartR <= 0) { continue; } // defensive
          startR = random(maxStartR);
          startC = random(MAP_WIDTH);
        }
        
        // Check for conflicts (overlap with other ships)
        bool conflict = false;
        for (int k = 0; k < shipSize; k++) {
          int checkR = startR + (orientation == 1 ? k : 0);
          int checkC = startC + (orientation == 0 ? k : 0);
          if (checkR < 0 || checkC < 0 || checkR >= MAP_HEIGHT || checkC >= MAP_WIDTH || gameMap[checkR][checkC] != WATER) {
            conflict = true;
            break;
        }
        }
        
        // If no conflict, place the ship
        if (!conflict) {
          char shipChar = '0' + shipSize;
          for (int k = 0; k < shipSize;k++) {
            int placeR = startR + (orientation == 1 ? k : 0);
            int placeC = startC + (orientation == 0 ? k : 0);
            gameMap[placeR][placeC] = shipChar;
          }
          placed = true;
          Serial.print("  Placed Size-"); Serial.print(shipSize); Serial.print(" ship after "); Serial.print(attempts); Serial.println(" attempts.");
        }
      } // end while
      
      if (!placed) {
        Serial.print("WARNING: Could not place all Size-"); Serial.print(shipSize); Serial.println(" ships after 100 attempts.");
      }
      
    } // end for (count)
  } // end for (sizeIndex)
  
}


// --- HELPER FUNCTION: PRINT THE MAP TO SERIAL ---
void printMap() {
    Serial.println("\n--- CURRENT MAP STATE ---");
    // Print column numbers as a header
    Serial.print("   "); // Extra space for row index
    for (int c = 0; c < MAP_WIDTH; c++) {
        Serial.print(c % 10); // Print single digit for simplicity
    }
    Serial.println();
    
    // Print the map rows
    for (int r = 0; r < MAP_HEIGHT; r++) {
        Serial.print(r % 10); // Print row number
        Serial.print(" |");
        for (int c = 0; c < MAP_WIDTH; c++) {
            Serial.print(publicMap[r][c]);
        }
        Serial.println("|");
    }
    Serial.println("-------------------------");
}

// --- CORE FUNCTION: HANDLE ATTACK INPUT (UPDATED) ---
void handleAttack() {
    String inputString = Serial.readStringUntil(TERMINATOR);
    inputString.trim();
    
    // ... (Parsing logic for c, r remains the same) ...
    // Assuming parsing is successful, we have c and r.
// Array to store the two coordinate numbers (C and R)
    int coords[2] = {-1, -1}; 
    int count = 0;
    int index = 0;
    int lastIndex = 0;
    
    // --- PARSING LOOP: Reads exactly 2 numbers ---
    while (index != -1 && count < 2) {
      
      // 1. Find the separator (space)
      index = inputString.indexOf(SEPARATOR, lastIndex);
      
      // 2. Extract the substring for the current number (C or R)
      String numberString;
      if (index == -1) {
        // Last number in the string (the Row)
        numberString = inputString.substring(lastIndex);
      } else {
        // Number followed by a separator (the Column)
        numberString = inputString.substring(lastIndex, index);
      }
      
      numberString.trim(); // Ensures no extra spaces interfere
      
      // 3. Convert to integer and store
      if (numberString.length() > 0) {
        coords[count] = numberString.toInt();
        count++;
      }
      
      // Update the starting point for the next search
      lastIndex = index + 1;
    }
    
    // --- ASSIGNMENT & VALIDATION ---
    if (count != 2) {
        Serial.println("INVALID INPUT: Please send two numbers for Column and Row (e.g., 4 5).");
        return;
    }
    
    int c = coords[1]; // Column
    int r = coords[0]; // Row
    
    Serial.print("Firing at (C:"); Serial.print(c); Serial.print(", R:"); Serial.print(r); Serial.println(")...");
    
    // 1. VALIDATION
    if (r < 0 || r >= MAP_HEIGHT || c < 0 || c >= MAP_WIDTH) {
        Serial.println("ERROR: Coordinates are outside the map boundaries!");
        return;
    }
    
    // 2. CHECK CELL STATUS
    char cell = gameMap[r][c];
    
    if (cell == WATER) {
        Serial.println("RESULT: MISS!");
        gameMap[r][c] = MISS;
        publicMap[r][c] = MISS;
    } else if (cell == MISS || cell == HIT) {
        Serial.println("RESULT: Already targeted!");
    } else { // It's a ship segment ('2', '3', or '4')
        Serial.println("RESULT: HIT!");
        
        // Temporarily save the ship character before marking the hit
        char shipChar = cell; 
        
        // Mark the cell as Hit
        gameMap[r][c] = HIT;
        publicMap[r][c] = HIT;
        hitsRemaining--;
        
        // 3. CHECK FOR SUNK SHIP
        if (isShipSunk(r, c, shipChar)) {
             // This is triggered if ALL ships of that size (e.g., all size 3s) are now hit.
             Serial.print("!!! You sunk the last Size-"); Serial.print(shipChar); Serial.println(" ship! !!!");
        }

        // 4. CHECK FOR GAME OVER
        if (hitsRemaining == 0) {
            Serial.println("\n*** CONGRATULATIONS! ALL SHIPS SUNK! GAME OVER ***");
        }
    }
    
    printMap();
}

bool isShipSunk(int r, int c, char shipChar) {
    // 1. Get the ship size (e.g., '3' -> 3)
    int shipSize = shipChar - '0';
    
    // 2. Scan the entire map for any remaining segments of this ship type.
    // We check for the original '2', '3', or '4' character.
    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int col = 0; col < MAP_WIDTH; col++) {
            // If we find any cell that still contains the original ship character,
            // the ship is NOT sunk.
            if (gameMap[row][col] == shipChar) {
                return false;
            }
        }
    }
    
    // 3. If the loop completes without finding any original ship characters, 
    // it means every segment of that type has been converted to 'H', and the ship is sunk.
    
    // NOTE: This implementation assumes all ships of the same size (e.g., all size 3 ships) 
    // are sunk when the last segment is hit. If you wanted to check individual ships, 
    // you would need to give each ship a unique ID (e.g., 'A', 'B', 'C') during placement.
    // For this simple game, checking the type is the simplest approach.
    
    return true;

}