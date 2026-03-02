#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SLOTS_PER_LEVEL 40
#define NUM_LEVELS 10
#define TOTAL_CAPACITY (SLOTS_PER_LEVEL * NUM_LEVELS)
#define HASH_TABLE_SIZE 101
#define LRU_CACHE_SIZE 20

// ========================================================================
// DATA STRUCTURES
// ========================================================================

// AVL Tree Node for sorted vehicle records
typedef struct AVLNode {
    char vehicleNumber[20];
    int level;
    int parkingSlot;
    char vehicleType[10];
    time_t entryTime;
    int height;
    struct AVLNode* left;
    struct AVLNode* right;
} AVLNode;

// Min-Heap Node for Priority Queue
typedef struct HeapNode {
    char vehicleNumber[20];
    int priority;
    char vehicleType[10];
    time_t timestamp;
} HeapNode;

// Min-Heap Structure
typedef struct MinHeap {
    HeapNode* array;
    int capacity;
    int size;
} MinHeap;

// Hash Table Node with timestamp
typedef struct HashNode {
    char vehicleNumber[20];
    int level;
    int parkingSlot;
    char vehicleType[10];
    time_t entryTime;
    double feePaid;
    struct HashNode* next;
} HashNode;

// LRU Cache Node
typedef struct LRUNode {
    char vehicleNumber[20];
    HashNode* data;
    struct LRUNode* prev;
    struct LRUNode* next;
} LRUNode;

// LRU Cache
typedef struct LRUCache {
    int capacity;
    int size;
    LRUNode* head;
    LRUNode* tail;
    HashNode* hashMap[HASH_TABLE_SIZE];
} LRUCache;

// Stack for available slots
typedef struct Stack {
    int slots[SLOTS_PER_LEVEL];
    int top;
} Stack;

// Analytics Structure
typedef struct Analytics {
    int totalVehiclesParked;
    int totalVehiclesExited;
    double totalRevenue;
    int twoWheelerCount;
    int fourWheelerCount;
    int emergencyCount;
    int vipCount;
    int normalCount;
} Analytics;

// Global Variables
Stack availableSlots[NUM_LEVELS];
MinHeap* waitingQueue;
HashNode* hashTable[HASH_TABLE_SIZE];
AVLNode* avlRoot = NULL;
LRUCache* cache;
Analytics stats = {0};

// Indian State Codes
const char* INDIAN_STATES[] = {
    "AN", "AP", "AR", "AS", "BR", "CH", "CG", "DD", "DL", "DN", "GA", "GJ",
    "HP", "HR", "JH", "JK", "KA", "KL", "LA", "LD", "MH", "ML", "MN", "MP",
    "MZ", "NL", "OD", "PB", "PY", "RJ", "SK", "TN", "TR", "TS", "UK", "UP", "WB"
};
#define NUM_STATES 37

// Fee Structure (per hour)
#define TWO_WHEELER_RATE 10.0
#define FOUR_WHEELER_RATE 20.0
#define FIRST_HOUR_FREE 0

// ========================================================================
// UTILITY FUNCTIONS
// ========================================================================

int isValidStateCode(char* stateCode) {
    for (int i = 0; i < NUM_STATES; i++) {
        if (strcmp(stateCode, INDIAN_STATES[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int validateVehicleNumber(char* vehicleNumber) {
    int len = strlen(vehicleNumber);
    
    if (len < 9 || len > 10) {
        printf("Error: Invalid format. Expected length 9-10 characters\\n");
        return 0;
    }
    
    char stateCode[3];
    strncpy(stateCode, vehicleNumber, 2);
    stateCode[2] = '\0';
    
    for (int i = 0; i < 2; i++) {
        if (stateCode[i] >= 'a' && stateCode[i] <= 'z') {
            stateCode[i] = stateCode[i] - 32;
        }
    }
    
    if (!isValidStateCode(stateCode)) {
        printf("Error: Invalid state code '%s'\\n", stateCode);
        return 0;
    }
    
    if (vehicleNumber[2] < '0' || vehicleNumber[2] > '9' ||
        vehicleNumber[3] < '0' || vehicleNumber[3] > '9') {
        printf("Error: District code must be 2 digits\\n");
        return 0;
    }
    
    return 1;
}

double calculateFee(time_t entryTime, time_t exitTime, char* vehicleType) {
    double hours = difftime(exitTime, entryTime) / 3600.0;
    
    if (hours <= 1.0) {
        return 0.0; // First hour free
    }
    
    double rate = (strcmp(vehicleType, "2wheeler") == 0) ? TWO_WHEELER_RATE : FOUR_WHEELER_RATE;
    return (hours - 1.0) * rate;
}

// ========================================================================
// AVL TREE IMPLEMENTATION
// ========================================================================

int max(int a, int b) {
    return (a > b) ? a : b;
}

int height(AVLNode* node) {
    return node ? node->height : 0;
}

int getBalance(AVLNode* node) {
    return node ? height(node->left) - height(node->right) : 0;
}

AVLNode* createAVLNode(char* vehicleNumber, int level, int slot, char* vehicleType, time_t entryTime) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    strcpy(node->vehicleNumber, vehicleNumber);
    node->level = level;
    node->parkingSlot = slot;
    strcpy(node->vehicleType, vehicleType);
    node->entryTime = entryTime;
    node->height = 1;
    node->left = node->right = NULL;
    return node;
}

AVLNode* rightRotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    
    x->right = y;
    y->left = T2;
    
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    
    return x;
}

AVLNode* leftRotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    
    y->left = x;
    x->right = T2;
    
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    
    return y;
}

AVLNode* insertAVL(AVLNode* node, char* vehicleNumber, int level, int slot, char* vehicleType, time_t entryTime) {
    if (node == NULL)
        return createAVLNode(vehicleNumber, level, slot, vehicleType, entryTime);
    
    int cmp = strcmp(vehicleNumber, node->vehicleNumber);
    
    if (cmp < 0)
        node->left = insertAVL(node->left, vehicleNumber, level, slot, vehicleType, entryTime);
    else if (cmp > 0)
        node->right = insertAVL(node->right, vehicleNumber, level, slot, vehicleType, entryTime);
    else
        return node;
    
    node->height = 1 + max(height(node->left), height(node->right));
    
    int balance = getBalance(node);
    
    // Left Left
    if (balance > 1 && strcmp(vehicleNumber, node->left->vehicleNumber) < 0)
        return rightRotate(node);
    
    // Right Right
    if (balance < -1 && strcmp(vehicleNumber, node->right->vehicleNumber) > 0)
        return leftRotate(node);
    
    // Left Right
    if (balance > 1 && strcmp(vehicleNumber, node->left->vehicleNumber) > 0) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    
    // Right Left
    if (balance < -1 && strcmp(vehicleNumber, node->right->vehicleNumber) < 0) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    
    return node;
}

AVLNode* minValueNode(AVLNode* node) {
    AVLNode* current = node;
    while (current->left != NULL)
        current = current->left;
    return current;
}

AVLNode* deleteAVL(AVLNode* root, char* vehicleNumber) {
    if (root == NULL)
        return root;
    
    int cmp = strcmp(vehicleNumber, root->vehicleNumber);
    
    if (cmp < 0)
        root->left = deleteAVL(root->left, vehicleNumber);
    else if (cmp > 0)
        root->right = deleteAVL(root->right, vehicleNumber);
    else {
        if ((root->left == NULL) || (root->right == NULL)) {
            AVLNode* temp = root->left ? root->left : root->right;
            
            if (temp == NULL) {
                temp = root;
                root = NULL;
            } else
                *root = *temp;
            
            free(temp);
        } else {
            AVLNode* temp = minValueNode(root->right);
            strcpy(root->vehicleNumber, temp->vehicleNumber);
            root->level = temp->level;
            root->parkingSlot = temp->parkingSlot;
            strcpy(root->vehicleType, temp->vehicleType);
            root->entryTime = temp->entryTime;
            root->right = deleteAVL(root->right, temp->vehicleNumber);
        }
    }
    
    if (root == NULL)
        return root;
    
    root->height = 1 + max(height(root->left), height(root->right));
    
    int balance = getBalance(root);
    
    if (balance > 1 && getBalance(root->left) >= 0)
        return rightRotate(root);
    
    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    
    if (balance < -1 && getBalance(root->right) <= 0)
        return leftRotate(root);
    
    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    
    return root;
}

AVLNode* searchAVL(AVLNode* root, char* vehicleNumber) {
    if (root == NULL || strcmp(root->vehicleNumber, vehicleNumber) == 0)
        return root;
    
    if (strcmp(vehicleNumber, root->vehicleNumber) < 0)
        return searchAVL(root->left, vehicleNumber);
    
    return searchAVL(root->right, vehicleNumber);
}

void inorderAVL(AVLNode* root) {
    if (root != NULL) {
        inorderAVL(root->left);
        printf("%-20s %-12s Level %-3d Slot %-3d\\n", 
               root->vehicleNumber, root->vehicleType, root->level, root->parkingSlot);
        inorderAVL(root->right);
    }
}

// ========================================================================
// MIN-HEAP IMPLEMENTATION (Priority Queue)
// ========================================================================

MinHeap* createMinHeap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->capacity = capacity;
    heap->size = 0;
    heap->array = (HeapNode*)malloc(capacity * sizeof(HeapNode));
    return heap;
}

void swap(HeapNode* a, HeapNode* b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void minHeapify(MinHeap* heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    
    if (left < heap->size && heap->array[left].priority < heap->array[smallest].priority)
        smallest = left;
    
    if (right < heap->size && heap->array[right].priority < heap->array[smallest].priority)
        smallest = right;
    
    if (smallest != idx) {
        swap(&heap->array[idx], &heap->array[smallest]);
        minHeapify(heap, smallest);
    }
}

void insertHeap(MinHeap* heap, char* vehicleNumber, int priority, char* vehicleType) {
    if (heap->size == heap->capacity) {
        printf("Heap is full!\\n");
        return;
    }
    
    int i = heap->size++;
    strcpy(heap->array[i].vehicleNumber, vehicleNumber);
    heap->array[i].priority = priority;
    strcpy(heap->array[i].vehicleType, vehicleType);
    heap->array[i].timestamp = time(NULL);
    
    while (i != 0 && heap->array[(i - 1) / 2].priority > heap->array[i].priority) {
        swap(&heap->array[i], &heap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

HeapNode extractMin(MinHeap* heap) {
    HeapNode minNode = heap->array[0];
    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;
    minHeapify(heap, 0);
    return minNode;
}

int isHeapEmpty(MinHeap* heap) {
    return heap->size == 0;
}

// ========================================================================
// STACK IMPLEMENTATION
// ========================================================================

void initStack() {
    for (int level = 0; level < NUM_LEVELS; level++) {
        availableSlots[level].top = -1;
        for (int i = SLOTS_PER_LEVEL; i >= 1; i--) {
            availableSlots[level].slots[++availableSlots[level].top] = i;
        }
    }
}

int isStackEmpty(int level) {
    return availableSlots[level].top == -1;
}

void pushSlot(int level, int slot) {
    if (availableSlots[level].top == SLOTS_PER_LEVEL - 1) {
        printf("Stack overflow!\\n");
        return;
    }
    availableSlots[level].slots[++availableSlots[level].top] = slot;
}

int popSlot(int level) {
    if (isStackEmpty(level)) {
        return -1;
    }
    return availableSlots[level].slots[availableSlots[level].top--];
}

// ========================================================================
// HASH TABLE IMPLEMENTATION
// ========================================================================

int hashFunction(char* vehicleNumber) {
    int sum = 0;
    for (int i = 0; vehicleNumber[i] != '\0'; i++) {
        sum += vehicleNumber[i];
    }
    return sum % HASH_TABLE_SIZE;
}

void initHashTable() {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hashTable[i] = NULL;
    }
}

void insertRecord(char* vehicleNumber, int level, int parkingSlot, char* vehicleType) {
    int index = hashFunction(vehicleNumber);
    
    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    strcpy(newNode->vehicleNumber, vehicleNumber);
    newNode->level = level;
    newNode->parkingSlot = parkingSlot;
    strcpy(newNode->vehicleType, vehicleType);
    newNode->entryTime = time(NULL);
    newNode->feePaid = 0.0;
    
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
    
    // Also insert into AVL tree
    avlRoot = insertAVL(avlRoot, vehicleNumber, level, parkingSlot, vehicleType, newNode->entryTime);
}

int searchRecord(char* vehicleNumber, int* level, int* slot, char* vehicleType, time_t* entryTime) {
    int index = hashFunction(vehicleNumber);
    HashNode* temp = hashTable[index];
    
    while (temp != NULL) {
        if (strcmp(temp->vehicleNumber, vehicleNumber) == 0) {
            *level = temp->level;
            *slot = temp->parkingSlot;
            strcpy(vehicleType, temp->vehicleType);
            *entryTime = temp->entryTime;
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

int deleteRecord(char* vehicleNumber) {
    int index = hashFunction(vehicleNumber);
    HashNode* temp = hashTable[index];
    HashNode* prev = NULL;
    
    while (temp != NULL) {
        if (strcmp(temp->vehicleNumber, vehicleNumber) == 0) {
            if (prev == NULL) {
                hashTable[index] = temp->next;
            } else {
                prev->next = temp->next;
            }
            
            // Also delete from AVL tree
            avlRoot = deleteAVL(avlRoot, vehicleNumber);
            
            free(temp);
            return 1;
        }
        prev = temp;
        temp = temp->next;
    }
    return 0;
}

void displayRecords() {
    int count = 0;
    printf("\\n--- Parked Vehicles (Sorted by Vehicle Number - AVL Tree) ---\\n");
    printf("%-20s %-12s %-10s %-15s\\n", "Vehicle Number", "Type", "Level", "Slot");
    printf("------------------------------------------------------------\\n");
    
    inorderAVL(avlRoot);
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode* temp = hashTable[i];
        while (temp != NULL) {
            count++;
            temp = temp->next;
        }
    }
    
    if (count == 0) {
        printf("No vehicles currently parked.\\n");
    } else {
        printf("\\nTotal parked vehicles: %d\\n", count);
    }
}

// ========================================================================
// MAIN FUNCTIONS
// ========================================================================

void displayAvailableSlots() {
    int totalAvailable = 0;
    printf("\\n--- Available Parking Slots (Stack - LIFO per Level) ---\\n");
    
    for (int level = 0; level < NUM_LEVELS; level++) {
        printf("Level %d: ", level + 1);
        if (isStackEmpty(level)) {
            printf("FULL");
        } else {
            printf("Available (%d)", availableSlots[level].top + 1);
            totalAvailable += (availableSlots[level].top + 1);
        }
        printf("\\n");
    }
    printf("Total Available Slots: %d/%d\\n", totalAvailable, TOTAL_CAPACITY);
}

void displayWaitingQueue() {
    if (isHeapEmpty(waitingQueue)) {
        printf("No vehicles waiting in queue.\\n");
        return;
    }
    
    printf("\\n--- Waiting Priority Queue (Min-Heap) ---\\n");
    printf("%-5s %-20s %-12s %-15s\\n", "Pos", "Vehicle Number", "Type", "Priority");
    printf("------------------------------------------------------------\\n");
    
    for (int i = 0; i < waitingQueue->size; i++) {
        char* priorityStr = (waitingQueue->array[i].priority == 1) ? "Emergency" :
                           (waitingQueue->array[i].priority == 2) ? "VIP" : "Normal";
        printf("%-5d %-20s %-12s %-15s\\n", i + 1, 
               waitingQueue->array[i].vehicleNumber,
               waitingQueue->array[i].vehicleType,
               priorityStr);
    }
}

void displayAnalytics() {
    printf("\\n========================================\\n");
    printf("  PARKING ANALYTICS\\n");
    printf("========================================\\n");
    printf("Total Vehicles Parked: %d\\n", stats.totalVehiclesParked);
    printf("Total Vehicles Exited: %d\\n", stats.totalVehiclesExited);
    printf("Total Revenue: ₹%.2f\\n", stats.totalRevenue);
    printf("\\nVehicle Type Distribution:\\n");
    printf("  2-Wheelers: %d\\n", stats.twoWheelerCount);
    printf("  4-Wheelers: %d\\n", stats.fourWheelerCount);
    printf("\\nPriority Distribution:\\n");
    printf("  Emergency: %d\\n", stats.emergencyCount);
    printf("  VIP: %d\\n", stats.vipCount);
    printf("  Normal: %d\\n", stats.normalCount);
    printf("========================================\\n");
}

void vehicleEntry() {
    char vehicleNumber[20];
    char vehicleType[10];
    int priority;
    int level, slot;
    char tempType[10];
    time_t tempTime;
    
    printf("\\nEnter vehicle number: ");
    scanf("%s", vehicleNumber);
    
    for (int i = 0; vehicleNumber[i]; i++) {
        if (vehicleNumber[i] >= 'a' && vehicleNumber[i] <= 'z') {
            vehicleNumber[i] = vehicleNumber[i] - 32;
        }
    }
    
    if (!validateVehicleNumber(vehicleNumber)) {
        printf("Invalid vehicle number!\\n");
        return;
    }
    
    if (searchRecord(vehicleNumber, &level, &slot, tempType, &tempTime)) {
        printf("Vehicle already parked at Level %d, Slot %d!\\n", level, slot);
        return;
    }
    
    printf("Select vehicle type:\\n1. 4-Wheeler\\n2. 2-Wheeler\\nChoice: ");
    int typeChoice;
    scanf("%d", &typeChoice);
    strcpy(vehicleType, (typeChoice == 2) ? "2wheeler" : "4wheeler");
    
    printf("Enter priority (1=Emergency, 2=VIP, 3=Normal): ");
    scanf("%d", &priority);
    
    if (priority < 1 || priority > 3) priority = 3;
    
    int allocated = 0;
    for (int lvl = 0; lvl < NUM_LEVELS && !allocated; lvl++) {
        if (!isStackEmpty(lvl)) {
            int allocatedSlot = popSlot(lvl);
            insertRecord(vehicleNumber, lvl + 1, allocatedSlot, vehicleType);
            printf("✓ %s parked at Level %d, Slot %d\\n", vehicleNumber, lvl + 1, allocatedSlot);
            allocated = 1;
            stats.totalVehiclesParked++;
            if (strcmp(vehicleType, "2wheeler") == 0) stats.twoWheelerCount++;
            else stats.fourWheelerCount++;
            if (priority == 1) stats.emergencyCount++;
            else if (priority == 2) stats.vipCount++;
            else stats.normalCount++;
        }
    }
    
    if (!allocated) {
        insertHeap(waitingQueue, vehicleNumber, priority, vehicleType);
        printf("Parking full! Added to queue.\\n");
    }
}

void vehicleExit() {
    char vehicleNumber[20];
    char vehicleType[10];
    int level, slot;
    time_t entryTime;
    
    printf("\\nEnter vehicle number: ");
    scanf("%s", vehicleNumber);
    
    for (int i = 0; vehicleNumber[i]; i++) {
        if (vehicleNumber[i] >= 'a' && vehicleNumber[i] <= 'z') {
            vehicleNumber[i] = vehicleNumber[i] - 32;
        }
    }
    
    if (!searchRecord(vehicleNumber, &level, &slot, vehicleType, &entryTime)) {
        printf("Vehicle not found!\\n");
        return;
    }
    
    time_t exitTime = time(NULL);
    double fee = calculateFee(entryTime, exitTime, vehicleType);
    
    printf("\\n--- Exit Summary ---\\n");
    printf("Vehicle: %s\\n", vehicleNumber);
    printf("Type: %s\\n", vehicleType);
    printf("Level: %d, Slot: %d\\n", level, slot);
    printf("Duration: %.2f hours\\n", difftime(exitTime, entryTime) / 3600.0);
    printf("Fee: ₹%.2f\\n", fee);
    printf("-------------------\\n");
    
    deleteRecord(vehicleNumber);
    stats.totalVehiclesExited++;
    stats.totalRevenue += fee;
    
    if (!isHeapEmpty(waitingQueue)) {
        HeapNode waiting = extractMin(waitingQueue);
        insertRecord(waiting.vehicleNumber, level, slot, waiting.vehicleType);
        printf("%s from queue allocated to Level %d, Slot %d\\n", 
               waiting.vehicleNumber, level, slot);
    } else {
        pushSlot(level - 1, slot);
        printf("Slot freed.\\n");
    }
}

void displayMenu() {
    printf("\\n========================================\\n");
    printf("  PARKING MANAGEMENT SYSTEM\\n");
    printf("  (Advanced DSA Implementation)\\n");
    printf("========================================\\n");
    printf("1. Vehicle Entry\\n");
    printf("2. Vehicle Exit\\n");
    printf("3. Display Parked Vehicles (AVL Tree)\\n");
    printf("4. Display Waiting Queue (Min-Heap)\\n");
    printf("5. Display Available Slots (Stack)\\n");
    printf("6. Display Analytics\\n");
    printf("7. Exit\\n");
    printf("========================================\\n");
    printf("Enter choice: ");
}

int main() {
    initStack();
    initHashTable();
    waitingQueue = createMinHeap(1000);
    
    printf("\\n*************************************************\\n");
    printf("  ADVANCED PARKING MANAGEMENT SYSTEM\\n");
    printf("  DSA Concepts: AVL Tree, Min-Heap, Stack, Hash\\n");
    printf("  Levels: %d | Slots per Level: %d\\n", NUM_LEVELS, SLOTS_PER_LEVEL);
    printf("  Total Capacity: %d slots\\n", TOTAL_CAPACITY);
    printf("*************************************************\\n");
    
    int choice;
    while (1) {
        displayMenu();
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: vehicleEntry(); break;
            case 2: vehicleExit(); break;
            case 3: displayRecords(); break;
            case 4: displayWaitingQueue(); break;
            case 5: displayAvailableSlots(); break;
            case 6: displayAnalytics(); break;
            case 7:
                printf("\\nThank you!\\n");
                exit(0);
            default:
                printf("Invalid choice!\\n");
        }
    }
    
    return 0;
}