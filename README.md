# 🅿️ Advanced Parking Management System

A multi-level parking management system implementing **15+ data structures and algorithms** in both a C backend and a modern web-based frontend.

---

## 📋 Project Overview

This system efficiently manages vehicle parking, queuing, searching, and analytics across **10 levels with 40 slots each** (400 total capacity).

### Key Features

- ✅ **Priority-based queuing** — Emergency, VIP, Normal
- ✅ **Real-time analytics** with charts and graphs
- ✅ **Smart search** with Trie-based autocomplete
- ✅ **Fee calculation** based on parking duration (first hour free)
- ✅ **Dark mode** support
- ✅ **Data persistence** with localStorage
- ✅ **Indian vehicle number validation**

---

## 🎯 DSA Concepts Implemented

| # | Data Structure / Algorithm | Time Complexity | Use Case |
|---|---------------------------|-----------------|----------|
| 1 | AVL Tree | O(log n) | Sorted vehicle records |
| 2 | Min-Heap | O(log n) | Priority queue management |
| 3 | Hash Table (Chaining) | O(1) avg | Fast vehicle lookup |
| 4 | Trie | O(k) | Autocomplete search |
| 5 | Stack | O(1) | Available slots per level |
| 6 | Linked List | O(n) | Hash table chaining |
| 7 | Binary Search | O(log n) | Sorted data queries |
| 8 | Queue | O(1) | FIFO operations |
| 9 | Tree Rotations (AVL) | O(1) | Tree rebalancing |
| 10 | Heap Operations | O(log n) | Priority management |

---

## 🚀 How to Run

### C Backend

```bash
# Compile
gcc parking_lot_upgraded.c -o parking

# Run
./parking
```

### Web Frontend

```bash
# Open the main GUI (Windows)
start parking_gui_professional.html

# Or open with login page first
start login.html
```

**Default Login Credentials:**
- User ID: `USER1`
- Password: `1234`

---

## 📁 Project Structure

```
parkingmanagementsystem/
├── parking_lot_upgraded.c          # C backend with 15+ DSA implementations
├── parking_gui_professional.html   # Main professional web frontend
├── login.html                      # Login/authentication page
├── index.html                      # Landing page
├── .gitignore                      # Git ignore rules
└── README.md                       # This file
```

---

## 💡 Key Algorithms

### AVL Tree Balancing
```
Balance Factor = height(left) - height(right)

BF > 1  → Left-heavy  → Right rotation
BF < -1 → Right-heavy → Left rotation
BF = 0  → Balanced
```

### Min-Heap Priority
```
Priority: 1 (Emergency) > 2 (VIP) > 3 (Normal)

Parent: (i-1)/2 | Left child: 2i+1 | Right child: 2i+2
```

### Fee Calculation
```
Fee = (Duration - 1) × Rate   // First hour is free
Rate: 2-wheeler ₹10/hr | 4-wheeler ₹20/hr
```

---

## 🔮 Future Enhancements

- [ ] Graph-based pathfinding (Dijkstra's algorithm)
- [ ] Vehicle reservation system
- [ ] Mobile app integration
- [ ] Payment gateway integration
- [ ] Multi-location support

---

## 📄 License

This project is created for educational purposes as part of DSA coursework.
