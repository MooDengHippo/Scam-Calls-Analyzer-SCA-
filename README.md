# 📞 Scam Calls Analyzer (SCA)

> **CPE112 - Data Structures Final Project**  
> Team Members:
    Member 1: Grittapob Chutitas ID 67070503402 (Admin Module)
    Member 2: Phoo Phoo Thit     ID 67070503455 (User Module)
    Member 3: Kittiphat Noikate  ID 67070503459 (Stuctures & Storage Module)

---

## 📌 Project Overview

The **Scam Calls Analyzer (SCA)** is a phone number risk assessment tool designed to identify potential scam callers.  
It uses a combination of **Hash Table**, **Graph (BFS/DFS)**, and **File Storage** to analyze and visualize scammer relationships.

Key Features:
- Lookup phone numbers and display their suspicious score.
- Show network relationships between suspicious numbers.
- Admin can update and manage scammer database via CLI.
- Easy reporting system for users to report suspicious numbers.

---

## ⚙️ Technologies Used

- **Language:** C Programming Language
- **Data Storage:** CSV files (`data/scam_numbers.csv`)
- **Data Structures:** Hash Table, Graph (Adjacency List), Queue (for BFS)
- **Interface:** Command-Line Interface (CLI)

---

## 🚀 Program Workflow

### 👤 User Mode

    **Enter a phone number to check:**
        **If found:** shows risk score and number of reports.
        **If not found:**
            If non-SEA number: automatically considered high risk. 
            If SEA number: explores scam network graph.
    Optionally report unknown numbers for admin review.

### 👑 Admin Mode

    Add new suspicious phone numbers (phone, score, reports).
    Link relationships between scam numbers (builds the network graph).

---

