# M25-CS1.304 – Assignment 1 (README)
**Name: Souradeep Das**

**Roll No.: 2025201004**  

> This README briefly explains the approach (data structures, algorithms, optimizations) for each question and gives step-by-step compile & run instructions.

---


## Question 1 — Seam Carving (Content‑Aware Resizing)

### Approach (Data structures, algorithms, optimizations)
- **Energy function:** Dual‑gradient magnitude per pixel on BGR channels. For each pixel `(r,c)`, compute
  - `dx = (R-L)^2` over channels, `dy = (D-U)^2`, then `energy = sqrt(dx + dy)`.
  - **Border handling:** wrap‑around indices (top uses last row, left uses last column, etc.) to avoid artificially high edges and match common seam‑carving references.
- **Seam identification (vertical):** Dynamic Programming (DP) over the energy grid:
  - `dp[r][c] = energy[r][c] + min(dp[r-1][c-1], dp[r-1][c], dp[r-1][c+1])` with parent pointers for backtracking.
  - Store parents in an `int**` and backtrack from the minimum in the last row to get the seam.
- **Seam removal (vertical):** Allocate a new image with width `W-1` and copy every row while skipping the seam column.
- **Horizontal seams:** Use the **transpose trick**: transpose the image, run the same vertical‑seam pipeline, transpose back.
- **I/O niceties:** Progress bar while removing multiple seams; output image name is auto‑derived as `<input_name>_resized.<ext>` beside the input.
- **Complexity:** Removing one seam is `O(H·W)`; removing `k_w` vertical seams and `k_h` horizontal seams is `O((k_w + k_h)·H·W)`.

### Build
Linux (pkg-config + OpenCV4):
```bash
g++ 2025201004_A1_Q1.cpp -o A1_Q1 `pkg-config --cflags --libs opencv4`
```
If your system uses `opencv` instead of `opencv4` in pkg-config:
```bash
g++ 2025201004_A1_Q1.cpp -o A1_Q1 `pkg-config --cflags --libs opencv`
```

### Run
```bash
./A1_Q1
# Prompts:
# Enter image path: <path/to/input_image>
# Enter the reduced width and height: <new_width> <new_height>
# Output: saves "<input_name>_resized.<ext>" next to the input file
```

---

## Question 2 — “Trie Harder” (Spell‑check, Autocomplete, Autocorrect)

### Approach (Data structures, algorithms, optimizations)
- **Data structure:** A classic **Trie** node with:
  - `int next[26]` (array of child indices, initialized to `-1`) and `bool flag` to mark word termination.
  - Trie stored as a dynamic `vector<Node>`; root is index 0.
- **Insert:** Walk/extend nodes for each character, set `flag=true` at word end.
- **Spell check (Type=1):** Traverse the path; return `1` only if the terminal node’s `flag` is set; else `0`.
- **Autocomplete (Type=2):**
  - Traverse to the prefix node; **DFS** over children `a..z` to collect words.
  - Iterating children in sorted order guarantees **lexicographic order** in the results without extra sorting.
- **Autocorrect (Type=3):**
  - **Trie‑guided Levenshtein**: Maintain a dynamic‑programming row for edit distance between the query and the current trie path.
  - For each node/edge, compute a new DP row using insert/delete/replace costs; **prune** subtrees when the minimal value in the row exceeds **3** (the allowed threshold).
  - Add a word to results when at a terminal node with edit distance `≤ 3`.
- **Complexity:**  
  - Insert/Spell‑check `O(L)` per word/query.  
  - Autocomplete worst‑case `O(number_of_words_under_prefix)`.  
  - Autocorrect worst‑case `O(|Trie| · |query|)` but heavily pruned by the distance threshold.

### Build
```bash
g++ 2025201004_A1_Q2.cpp -o A1_Q2
```

### Run (stdin -> stdout)
```
<n> <q>
<dictionary word 1>
...
<dictionary word n>
<type> <string>
...
```
- **Type 1:** Spell‑check → prints `1` or `0`.
- **Type 2:** Autocomplete → prints count on one line, then the words (lexicographic).
- **Type 3:** Autocorrect (Levenshtein ≤ 3) → prints count, then the words (lexicographic).

---

## Question 3 — Battle of the Banners (Dynamic Skyline)

### Approach (Data structures, algorithms, optimizations)
- **Coordinate Compression:**
  - Collect all `l` and `r` from update queries up front, sort, and deduplicate to compress the line into `M` atomic segments.
- **Segment Tree with Lazy Propagation (range assignment):**
  - Each node stores the **maximum height** over its segment.
  - **Update (Type=0)** assigns a new height over an interval `[l, r)`:
    - Convert to indices in the compressed array, then **range-assign** with lazy propagation.
  - **Silhouette print (Type=1):**
    - Materialize the height per atomic segment by traversing the segment tree (pushing lazy tags).
    - Scan left->right and print a key point every time the height changes; finally print the last coordinate with height `0`.
- **Complexity:**
  - Update: `O(log M)` per range assignment.
  - Print: `O(M)` to traverse/build plus `O(K)` to output `K` key points (output‑sensitive; printing necessarily scales with the number of breakpoints).

### Build
```bash
g++ 2025201004_A1_Q3.cpp -o A1_Q3
```

### Run (stdin → stdout)
```
<q>
t1 [l1 r1 h1]
t2 [l2 r2 h2]
...
```
- **Type 0:** `0 l r h` → set height `h` over `[l, r)`.
- **Type 1:** `1` -> print the skyline as lines `x height`, emitting a point whenever the height changes, ending with `<last_x> 0`.

---

## Troubleshooting / Notes
- **OpenCV linkage (Q1):** If `pkg-config` is missing or reports no package, ensure OpenCV C++ dev files are installed and change the back‑ticked part accordingly to your platform’s flags.
- **Paths (Q1):** The program asks for an **image path**. The output is placed beside the input as `<name>_resized.<ext>`.

---
