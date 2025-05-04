# 📞 Scam Calls Analyzer (SCA)

> **CPE112 - Data Structures Final Project**  
>**Team Members:**
- "**Grittapob Chutitas** (67070503402) - Admin Module"         
- "**Phoo Phoo Thit** (67070503455) - User Module"
- "**Kittiphat Noikate** (67070503459) - Structures & Storage Module"

---

## 📌 Project Overview

The Scam Calls Analyzer (SCA) is a phone number risk evaluation system built with C, designed to detect and manage suspicious numbers. The system integrates user reporting, admin database management, and graph based relationship analysis to help mitigate phone scam threats.

**Key Features:**

- Lookup phone numbers and show suspicious score (0–100%).
- Display graph-based relationships among scam numbers.
- Users can report unlisted suspicious numbers.
- Admins can update records and relationships.

---

## 🌍 Real World Problem Inspiration

- The system is inspired by the growing number of scam calls in Southeast Asia. 
- Users often receive calls from unfamiliar numbers, and there is no easy way to verify if a number is suspicious.
- This project models how telcos or users can crowdsource suspicious number reports and visualize scam networks using graph relationships.

---

## ⚙️ Technologies Used

- **Language:** C Programming Language
- **Data Structures:** Hash Table, Graph (Adjacency List), Queue (BFS)
- **Data Storage:** CSV (Record phone, Edge relation, Pending report)
- **CLI Interface:** Command-line menu for both user/admin
- **Logging:** Custom logging module (app.log)

---

## 🚀 Program Workflow

### 👤 User Mode

- **Input phone number**
    - **If found:** display score, report count, and connected numbers.
    - **If not found:**
         - **If outside SEA region:** flagged Danger risk.
         - **If SEA:** explore relationships via graph traversal.

- **Report option:** Send number to admin for review.

### 👑 Admin Mode

- **Add/Edit Number:** Add new scammer or edit increment data.
- **Delete Record:** Remove scam number from database.
- **Analyze Number:** Check report stats and connections.
- **Link Numbers:** Build bidirectional scam relationships.
- **Accept Reports:** View and approve entries from pending report.

---

## 🗃️ Data Structures Explanation

1. **Hash Table**
- Used to store scam numbers and their metadata.
- Provides fast lookup (O(1) avg-case).
- Handles collisions via chaining (linked list).
2. **Graph (Adjacency List)**
- Represents scam relationships between phone numbers.
- Enables graph traversal (BFS/DFS) for network exploration.
3. **Queue**
- Used in BFS for graph traversal.
- Dynamic queue implementation with linked list.

---

## ⏱️ Time & Space Complexity

| Operation   | Time Complexity | Notes       |
|---------------------------|-----------------|-------------------------------------|
| Hash Table Insert/Lookup| O(1) avg-case|O(n) worst-case (chaining collision)|
| Graph Add Edge | O(1) | Simple bidirectional edge insert |
| BFS/DFS Traversal | O(V + E) | V = number of nodes, E = edges |
| CSV Read/Write | O(n) | n = number of records |

---

## 📚 Scam Detection Algorithm

The system identifies scam numbers using a **risk score algorithm** based on the following logic:

### 📌 Inputs:
- `report_count` — how many times a number has been reported
- `region` — whether the number is from a Southeast Asian (SEA) country
- `neighbor_count` — how many connected scam numbers it is linked to in the relationship graph

### 🧮 Risk Score Formula:

1. **Base Score** is assigned by region:
   - Thai landlines (e.g., `+662...`): starts at 0.5
   - Thai mobiles (e.g., `+668...`): starts at 0.1
   - Other SEA countries (e.g., `+855`, `+95`): starts at 0.7+
   - Non-SEA countries: always 1.0 (max risk)
2. **Report Contribution**:
- Each report increases the base score by `+0.05`
3. **Neighbor Contribution**: 
- Each connected node in the scam graph adds `+0.05` (capped at `+0.2` total)
4. **Final Score**:
- `score = min(1.0, base + report_bonus + neighbor_bonus)`
If a number reaches a score above 0.81, it is classified as `SEVERE` risk.

### ✅ Example:
```
+66812345678 Reports: 3
Neighbors: 2
→ Base: 0.1
→ +0.15 (3 reports)
→ +0.10 (2 neighbors)
→ Final Score = 0.35 → LOW RISK
```

---

## 🎓 Learning Reflections

> “I learned how data structures like Hash Tables and Graphs can power real-world applications. It helped me understand C more deeply.”  
> — *Kittiphat Noikate*

> “Designing user interfaces in CLI taught me the importance of usability, even in simple terminal programs.”  
> — *Phoo Phoo Thit*

> “Admin feature logic and validation gave me practical insights into how backend logic works.”  
> — *Grittapob Chutitas*

---

## 🙏 Acknowledgements

- Special thanks to Dr. Aye Hninn Khine, Mr. Naveed Sultan for guiding us through C and data structures.
- Project inspired by real scam call challenges in SEA.
- Developed as part of CPE112 at KMUTT.

---

## 🧪 Sample Test Case

### scam_numbers.csv
R,+66812345678,0.60,3                      
R,+66898765432,0.85,5
### scam_edges.csv
E,+66812345678,+66898765432
### User Test:
**Input:** `+66812345678`  
**Output:**
- Score: 60%                     
- Reports: 3
- Linked to: +66898765432
### Admin Test:
Accept report for `+66999999999`  
**Output:**                         
New record added with 1 report and score ~0.75.

---

## ⚙️ How to Run

### Requirements:
- C compiler (e.g., GCC)
- `data/` directory with:
  - scam_numbers.csv
  - scam_edges.csv
  - pending_reports.csv (optional)

### Compile:
```
gcc -o sca main.c csv_manage.c hash_table.c graph.c logging.c phone_format.c queue.c cli_user.c cli_admin.c
```
### Run:
```
./sca
```
---

```
___________________________________________________________________
📁 SCA_Project
├── 📁 src                         # Source Code
│   ├── main.c                    # Main entry point
│   ├── cli_user.c                # User Module
│   ├── cli_admin.c               # Admin Module
│   ├── hash_table.c              # Hash Table Implementation
│   ├── graph.c                   # Graph & BFS/DFS
│   ├── csv_manage.c              # CSV File I/O
│   ├── logging.c                 # Logging System
│   ├── phone_format.c            # Phone Normalization & Scoring
│   ├── queue.c                   # BFS Queue Implementation
├── 📁 include                     # Header Files
│   ├── cli_user.h                # User Module Header
│   ├── cli_admin.h               # Admin Module Header
│   ├── hash_table.h              # Hash Table Header
│   ├── graph.h                   # Graph Header
│   ├── csv_manage.h              # CSV Handler Header
│   ├── logging.h                 # Logger Header
│   ├── phone_format.h            # Phone Format Header
│   ├── queue.h                   # Queue Header
├── 📁 data                         # CSV Data & Logs
│   ├── scam_numbers.csv          # Scam Number Database
│   ├── scam_edges.csv            # Number Relationships
│   ├── pending_reports.csv       # Pending User Reports
│   ├── app.log                   # Application Logs
├── 📁 docs                         # Documentation
|   ├── Scam Calls Analyzer.pdf   # Full Project Report
│   ├── Code Standard.pdf         # Define Coding Style
│   ├── Test Cases.pdf            # Test Case Combination
├── 📁 stress                       # Stress Data Sets
|   ├── scam_numbers.csv          # Stress Scam Number Database
|   ├── scam_edges.csv            # Stress Number Relationships
|   ├── pending_reports.csv       # Stress Pending User Reports
|   
├────── README.md                 # Project Overview & Guide
└────── sca.exe                   # Program Execution
___________________________________________________________________