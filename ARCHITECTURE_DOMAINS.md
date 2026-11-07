# WAVEWATCH IV Domain Decomposition Architecture

*Last updated: November 6, 2026*

This document describes the domain decomposition architecture, initial partitioning, integrated iterative refinement loop, and convergence criteria for regular structured (UQ) grids in WAVEWATCH IV (`src/ww4_core/solver_UQ/UQ_domains.cpp`).

---

## 1. Domain Decomposition Pipeline Overview

Domain decomposition divides the active computational wet (sea) points of a full model grid ($N_{\text{wet}}$) into $K$ balanced computational domains (where $K = \text{num\_domains}$ or $\text{num\_ranks\_compute}$) to enable efficient parallel execution.

The decomposition pipeline consists of two primary stages:
1. **Initial Domain Decomposition**: Recursive Orthogonal Bisection partitions the grid into $K$ initial domains. Following bisection, domains are sorted by their active wet cell count, spatial neighbor relationships are determined, and baseline test diagnostics are written to stdout.
2. **Integrated Refinement Loop**: A single unified refinement loop iteratively balances the domains across seven sequential operational steps per iteration until convergence criteria are met or the maximum iteration limit is reached.

```mermaid
flowchart TD
    Start([Start Domain Decomposition]) --> Bisect[1. Recursive Orthogonal Bisection<br/>Divide computational domain into K initial domains]
    Bisect --> SortInit[Post-Bisection Initialization:<br/>- Sort all domains by number of wet points<br/>- Determine 4-connected spatial neighbors of each domain]
    SortInit --> StdoutInit[Output Initial Test Diagnostics to stdout:<br/>- Highest and lowest number of grid points<br/>- Largest domain size as % of target domain size]

    StdoutInit --> LoopStart([Enter Integrated Refinement Loop<br/>iter = 1 ... max_iterations])

    subgraph IntegratedRefinement [Integrated Refinement Loop]
        Step1[Step 1: Check Smallest Domain Neighbors<br/>If no neighbors & size < 70% target:<br/>Split coherent part of largest domain with neighbors<br/>Resort domains & adjust neighbor information]
        Step2[Step 2: Check Largest Domain Neighbors<br/>If no neighbors:<br/>Split largest domain into 2 near-equal parts<br/>Merge smallest adjacent pair with external neighbors<br/>Resort domains & adjust neighbor information]
        Step3[Step 3: Sequential Full Boundary Exchanges<br/>Pairwise: 1st-2nd, 2nd-3rd, ..., (K-1)th-Kth largest<br/>Reject swap if furthest domain from target gets further away]
        Step4[Step 4: Deal with Disconnected Pieces<br/>Identify components via 4-connectivity BFS<br/>Retain largest bulk; reassign small pieces to smallest neighbor]
        Step5[Step 5: Deal with Small Grid Extensions<br/>Row/col boundary point smoothing & voting<br/>Alternating row/col order per iteration]
        Step6[Step 6: Resort Domains & Re-establish Neighbors<br/>Resort domains by size and update neighbor tables<br/>Write diagnostic test info to stdout]
        Step7{Step 7: Convergence Check<br/>Largest domain < 5% above target OR<br/>No domains changed in current iteration?}

        Step1 --> Step2
        Step2 --> Step3
        Step3 --> Step4
        Step4 --> Step5
        Step5 --> Step6
        Step6 --> Step7
    end

    LoopStart --> Step1
    Step7 -- Yes: Converged --> Done([Decomposition Complete<br/>Return Final Domain Map])
    Step7 -- No: Continue --> MaxCheck{iter < max_iterations?}
    MaxCheck -- Yes --> Step1
    MaxCheck -- No: Iteration Limit --> Done
```

---

## 2. Initial Domain Decomposition & Post-Bisection Diagnostics

### 2.1 Recursive Orthogonal Bisection
Initial domain partitioning is performed using recursive orthogonal bisection:
- The bounding box containing active sea cells is bisected along its longest spatial dimension ($X$ or $Y$).
- The partition cut divides the active sea cells into two balanced subsets based on wet cell counts, allocating $\lfloor K / 2 \rfloor$ domains to one side and $\lceil K / 2 \rceil$ domains to the other.
- Bisection recurses until each sub-domain corresponds to a single domain ($k = 1$).

### 2.2 Domain Sorting and Neighbor Determination
Immediately following the bisection phase:
1. **Domain Sorting**: All $K$ domains are sorted in rank order according to their total count of active wet points ($N_i$).
2. **Neighbor Determination**: The 4-connected spatial neighbors (North, South, East, West) are identified for every domain. When longitudinal 360° periodic wrapping is active (`is_global == true`), horizontal neighbor connectivity wraps across the closure boundary ($X = 0 \leftrightarrow X = N_X - 1$).

### 2.3 Initial Test Diagnostics Output
Following sorting and neighbor identification, initial diagnostic test output is printed to `stdout`:
- **Highest number of grid points**: $\max_{1 \le i \le K} N_i$
- **Lowest number of grid points**: $\min_{1 \le i \le K} N_i$
- **Largest domain percentage of target size**: $\left( \frac{\max_i N_i}{T} \right) \times 100\%$

where the target domain size $T$ is defined as the total number of wet points divided by the total number of domains, rounded up:
$$T = \left\lceil \frac{N_{\text{wet}}}{K} \right\rceil$$

---

## 3. The Integrated Refinement Loop

The refinement phase consists of a single integrated loop executing seven sequential steps during each iteration (`iter = 1, 2, \dots, \text{max\_iterations}`):

```mermaid
flowchart TD
    IterStart([Start Refinement Iteration]) --> S1Check{Step 1: Smallest domain<br/>has neighbors?}
    S1Check -- Has Neighbors --> S2Check{Step 2: Largest domain<br/>has neighbors?}
    S1Check -- No Neighbors --> S1Size{Size < 70% of target?<br/>N_min < 0.70 * T}
    S1Size -- Yes --> S1Split[Split off coherent part of largest domain with neighbors<br/>Add to smallest domain so resulting size ~ target<br/>Resort domains & adjust neighbor information]
    S1Size -- No --> S2Check
    S1Split --> S2Check

    S2Check -- Has Neighbors --> S3Loop[Step 3: Sequential Full Boundary Exchanges<br/>Iterate pairs i = 1 ... K-1 from largest to smallest]
    S2Check -- No Neighbors --> S2Action[Split largest domain into 2 near-equal parts<br/>Merge two adjacent domains with smallest combined size<br/>that have additional external neighbors<br/>Resort domains & adjust neighbor information]
    S2Action --> S3Loop

    S3Loop --> S3Swap{Furthest domain from target<br/>gets even further away<br/>in absolute value?}
    S3Swap -- Yes --> S3Skip[Skip exchange for this pair]
    S3Swap -- No --> S3Exec[Exchange full boundary data between pair]
    S3Skip --> S3Next{More pairs?}
    S3Exec --> S3Next
    S3Next -- Yes --> S3Loop
    S3Next -- No --> S4Disconn[Step 4: Reassign Disconnected Pieces<br/>Identify components via 4-connectivity BFS<br/>Retain primary bulk; reassign small pieces to smallest neighbor]

    S4Disconn --> S5Ext[Step 5: Deal with Small Grid Extensions<br/>Row/col boundary point smoothing & voting<br/>Alternating row/col order per iteration]

    S5Ext --> S6Resort[Step 6: Resort Domains & Update Neighbors<br/>Resort all domains by active cell count<br/>Re-establish spatial neighbor tables<br/>Write test diagnostics to stdout]

    S6Resort --> S7Conv{Step 7: Convergence Reached?<br/>- Largest domain < 5% larger than target (N_max < 1.05 * T)<br/>OR<br/>- No domain assignments changed in current iteration}

    S7Conv -- Yes --> ExitLoop([Exit Refinement Loop: Converged])
    S7Conv -- No --> S8Limit{iter >= max_iterations?}
    S8Limit -- Yes --> ExitLoop
    S8Limit -- No --> NextIter([Proceed to Next Iteration])
```

### Step 1: Smallest Domain Isolation and Underload Handling
- Check if the smallest domain has neighbors.
- If the smallest domain has **no neighbors** AND its size is smaller than 70% of the target domain size:
  $$N_{\text{smallest}} < 0.70 \times T$$
  then split off a spatially coherent part of the largest domain that has neighbors and add it to the smallest domain such that the resulting domain size is close to the target size $T$.
- Resort all domains by active cell count and update all domain neighbor connectivity tables.

### Step 2: Largest Domain Isolation and Split/Merge Rebalancing
- Check if the largest domain has neighbors.
- If the largest domain has **no neighbors**:
  1. Split the largest domain into two near-equal sub-domains.
  2. To preserve the total domain count $K$, two other domains must be merged. Select the two adjacent domains that:
     - Share an active spatial boundary,
     - Produce the **smallest combined cell count** ($N_a + N_b$ is minimized), and
     - Possess additional neighbors beyond each other (ensuring the merged domain remains connected to the broader grid topology).
  3. Merge the selected adjacent domain pair.
- Resort all domains by active cell count and update all domain neighbor connectivity tables.

### Step 3: Sequential Full Boundary Exchanges Between Ranked Domains
- Boundary data exchanges are evaluated sequentially across size-ranked adjacent domains:
  - Exchange full boundary data between the 1st and 2nd largest domains.
  - Repeat between the 2nd and 3rd largest domains.
  - Continue sequentially through all ranked pairs down to the two smallest domains ($i = 1, 2, \dots, K-1$).
  - For each domain, if cell count is above target, boundary grid points are transferred at the longest contiguous line of points on their border to its smallest neighbor; if below target, the longest contiguous line of cells is taken from its largest neighbor.
  - Cell counts of exchanging domains are adjusted on the fly, without re-sorting the list while in the loop over all domains.
- **Rejection Criterion**: Do not exchange boundary data if the exchange results in the domain that is furthest away from the target size becoming even further away (in absolute value) after the exchange:
  $$\Delta_{\max}^{\text{after}} > \Delta_{\max}^{\text{before}}$$
  where:
  $$\Delta_{\max}^{\text{before}} = \max\left(|N_i - T|, |N_{i+1} - T|\right)$$
  $$\Delta_{\max}^{\text{after}} = \max\left(|N_i^{\prime} - T|, |N_{i+1}^{\prime} - T|\right)$$
  If this condition is violated, the boundary exchange is rejected and the boundary remains unchanged.

### Step 4: Disconnected Piece Reassignment
- Handle disconnected fragments using connected component analysis:
  1. Identify all 4-connected components for each domain using a breadth-first search (BFS).
  2. The component containing the largest number of wet cells is designated as the primary bulk of the domain.
  3. All smaller disconnected components are reassigned to their adjacent neighboring domain that has the smallest total active wet cell count.

### Step 5: Small Grid Extensions
- Clean narrow grid extensions and single protruding points on domain boundaries:
  1. **Row sweep**: If a point in a domain does not have points in the same domain next to it in a row, and one of these adjacent points is in another domain, assign the point to the other domain. If all three points in a row are in different domains, move the point to the domain most common in its four direct neighbors, or if that is not decisive, within the nine-point stencil directly surrounding the point.
  2. **Column sweep**: Repeat for points adjacent in a column: if a point does not have points in the same domain next to it in a column, and one of these adjacent points is in another domain, assign the point to the other domain. If all three points in a column are in different domains, move the point to the domain most common in its four direct neighbors, or if not decisive, within the nine-point stencil surrounding the point.
  3. **Alternation**: Alternate the row-column processing order per iteration (even iterations evaluate rows then columns; odd iterations evaluate columns then rows).

### Step 6: Domain Resorting, Neighbor Updating, and Iteration Diagnostics
- Resort all domains in rank order based on their active wet cell count.
- Re-establish 4-connected spatial neighbor relationships for all domains.
- Write the diagnostic test output to `stdout` in the same format as written after the initialization:
  - Highest number of grid points across all domains,
  - Lowest number of grid points across all domains,
  - Size of the largest domain as a percentage of the target domain size:
    $$\text{Percentage} = \left( \frac{\max_i N_i}{T} \right) \times 100\%$$

### Step 7: Convergence Evaluation
- Evaluate whether the iterative refinement has converged:
  1. **Load balance criterion**: The largest domain is less than 5% larger than the target domain size:
     $$\max_{1 \le i \le K} N_i < 1.05 \times T$$
  2. **Stability criterion**: No domain cell assignments have changed in the present iteration ($\Delta_{\text{domains}} == 0$).
- If either condition is met, convergence is reached and the refinement loop terminates.

### Loop Continuation
- If convergence has not been reached and the iteration counter has not reached `max_iterations`, advance to the next iteration.
- If `max_iterations` is reached without meeting the convergence criteria, the refinement loop terminates and returns the current domain decomposition.

---

## 4. Mathematical Formulation and Operational Rules

### 4.1 Target Domain Size
Given total active wet sea points $N_{\text{wet}}$ and number of domains $K$:
$$T = \left\lceil \frac{N_{\text{wet}}}{K} \right\rceil$$

### 4.2 Test Diagnostic Output Metrics
- Maximum domain load: $N_{\max} = \max_{1 \le i \le K} N_i$
- Minimum domain load: $N_{\min} = \min_{1 \le i \le K} N_i$
- Load percentage of target:
  $$P_{\max} = \frac{N_{\max}}{T} \times 100\%$$

### 4.3 Step 1: Smallest Domain Isolation Criteria
- Isolation condition: $\text{Neighbors}(D_{\text{smallest}}) = \emptyset$
- Underload threshold: $N_{\text{smallest}} < 0.70 \times T$
- Action: Identify largest domain with $\text{Neighbors}(D) \ne \emptyset$, extract a coherent sub-region of size $\Delta N \approx T - N_{\text{smallest}}$, and assign to $D_{\text{smallest}}$.

### 4.4 Step 2: Largest Domain Isolation Criteria
- Isolation condition: $\text{Neighbors}(D_{\text{largest}}) = \emptyset$
- Action:
  1. Bisect $D_{\text{largest}}$ into parts $A$ and $B$ with $|N_A - N_B| \le 1$.
  2. Select adjacent merge pair $(p_a, p_b)$ satisfying:
     $$(p_a, p_b) = \arg\min_{(i, j) \in \text{Adjacent}} (N_i + N_j)$$
     subject to:
     $$\left( \text{Neighbors}(i) \cup \text{Neighbors}(j) \right) \setminus \{i, j\} \ne \emptyset$$
  3. Merge $p_a$ and $p_b$ into a single domain.

### 4.5 Step 3: Full Boundary Exchange Rejection Rule
For ranked domains $D_i$ and $D_{i+1}$ sharing a spatial interface:
- Let candidate post-exchange sizes be $N_i^{\prime}$ and $N_{i+1}^{\prime}$.
- Let $\text{dev}(N) = |N - T|$.
- The boundary exchange is rejected if:
  $$\max\left(\text{dev}(N_i^{\prime}), \text{dev}(N_{i+1}^{\prime})\right) > \max\left(\text{dev}(N_i), \text{dev}(N_{i+1})\right)$$

### 4.6 Step 7: Convergence Criteria
Refinement completes successfully when:
$$\frac{N_{\max}}{T} < 1.05 \quad \lor \quad \sum_{c \in \text{Cells}} |\text{domain}_{\text{new}}(c) - \text{domain}_{\text{old}}(c)| = 0$$

---

## 5. Architectural Analysis: Overcoming Geographic and Topological Imbalance

The systematic refinement pipeline directly resolves key architectural issues observed in structured domain decomposition:

1. **Resolution of Isolated and Semi-Enclosed Basins (Steps 1 & 2)**:
   - Land-locked seas and isolated water bodies lack direct 4-connected spatial neighbors across land barriers.
   - When an isolated basin is underloaded ($< 70\%$ of target), Step 1 offloads points from the largest connected basin to bring it near target capacity.
   - When an isolated basin is excessively large, Step 2 splits it into balanced halves while merging the two smallest connected neighboring domains, preserving the total domain count $K$ without stalling boundary exchanges.

2. **Mitigation of Pairwise Overshoot (Step 3)**:
   - Sorting domains and performing boundary exchanges at the longest contiguous line of points on their border focuses rebalancing on the largest disparities first.
   - The absolute distance-from-target rejection criterion ($\Delta_{\max}^{\text{after}} \le \Delta_{\max}^{\text{before}}$) guarantees that exchanges never worsen the imbalance of the most deviant domain, eliminating limit-cycle oscillations between neighboring domains.

3. **Prevention of Domain Fragmentation & Protrusions (Steps 4 & 5)**:
   - Connected component analysis preserves a single primary bulk per domain and continuously absorbs orphaned fragments into neighboring domains with the lowest workload.
   - Step 5 alternates row and column sweeps to reassign isolated 1-point protrusions and bridge points to adjacent domains, using 4-neighbor voting with 9-point stencil tie-breaking.

4. **Rigorous Convergence Bounds (Step 7)**:
   - Defining convergence as $< 5\%$ deviation above target domain size or complete state stability ensures deterministic termination and optimal computational load balance across all MPI ranks.
