# Robust Visual Similarity Algorithm

This document describes the "Robust" algorithm used for finding visually similar items in the Replace Tool. It is a 1:1 implementation of the logic defined in `mappingtool/FIND_SIMILAR_ALGORITHM.md`.

## Core Logic

The algorithm distinguishes between two types of items:
1. **Opaque Items**: Items with no transparent pixels (e.g., walls, grounds).
2. **Transparent Items**: Items with transparency (e.g., vegetation, furniture).

### 1. Transparent Items (Dice Coefficient)

For items that are not fully opaque, the similarity is calculated using the **Dice Coefficient** on their binary masks.

#### Step 1: Extract Binary Mask
Convert the RGBA image to a binary mask.
- A pixel is **1 (Foreground)** if its Alpha channel is **> 10**.
- A pixel is **0 (Background)** otherwise.

#### Step 2: Resize (Normalization)
If the two masks being compared have different dimensions:
- Determine the target size: `width = max(w1, w2)`, `height = max(h1, h2)`.
- Resize both masks to this target size using **Nearest Neighbor** interpolation.

#### Step 3: Compare Masks (Confusion Matrix)
Count the pixel matches between the two (potentially resized) masks:
- **True Positive (TP)**: Both masks have 1 at the same position.
- **False Positive (FP)**: Source has 0, Candidate has 1.
- **False Negative (FN)**: Source has 1, Candidate has 0.
- **True Negative (TN)**: Both masks have 0.

#### Step 4: Calculate Score
The Primary Similarity Score is the **Dice Coefficient**:
```cpp
Similarity = (2.0 * TP) / (2.0 * TP + FP + FN)
```

### 2. Opaque Items (aHash)

For items that are fully opaque (all pixels have Alpha > 10), we use **Perceptual Hashing (aHash)** to compare visual structure, as binary masks would be identical rectangles for all full-tile items.

#### Step 1: Calculate aHash
1. Resize the image to **8x8** pixels.
2. Convert to **Grayscale** (`0.299*R + 0.587*G + 0.114*B`).
3. Calculate the **average brightness** of the 64 pixels.
4. Generate a 64-bit hash:
   - Bit `i` is **1** if pixel `i` brightness >= average.
   - Bit `i` is **0** otherwise.

#### Step 2: Compare Hashes
Calculate the **Hamming Distance** (number of differing bits) between the two hashes.

#### Step 3: Calculate Score
```cpp
Similarity = 1.0 - (HammingDistance / 64.0)
```

## Implementation Details for RME

To ensure performance while maintaining 1:1 accuracy:

1. **Pre-calculation**:
   - During the "Indexing" phase, we calculate and cache:
     - The **Binary Mask** (as a bitset or vector).
     - The **aHash** (for potential fallback/opaque checks).
     - IsOpaque flag.
   
2. **Comparison**:
   - `FindSimilar(itemId)` determines if the source item is Opaque.
   - It then iterates through all cached items.
   - It selects the comparison method (Dice or aHash) based on the source item type.
   - It performs the comparison strictly according to the steps above.

3. **Performance Optimization for Standard Size**:
   - Since >99% of items are 32x32, the "Resize" step is skipped for these comparisons, allowing for highly optimized bitwise operations.
   - Resize is only performed when dimensions differ.
