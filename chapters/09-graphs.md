# Graphs {#ch:graphs}

::: goals
By the end of this week you should be able to:

- model a problem as a graph, saying what the vertices and edges are;
- represent a graph with adjacency lists or an adjacency matrix, and choose between them;
- implement breadth-first and depth-first search, and say what each is good for;
- find a topological order of a directed acyclic graph;
- implement Dijkstra's algorithm with a priority queue, and explain why it requires non-negative weights;
- implement union-find with union by size and path compression.
:::

In a linked list, every node points to at most one other. In a tree, a node may point to several, but no node is pointed to by more than one, and there is a root from which everything hangs. Take away those restrictions, and let any node be connected to any others, and you have a **graph**: the most general linked structure, and the one that describes the world best.

A road map is a graph: towns, and the roads between them. So is a social network (people, and who knows whom), the web (pages and links), an airline's route map, the wiring of a circuit, the prerequisite structure of a curriculum, the dependency structure of a large program, the atoms and bonds of a molecule. Whenever the interesting thing about some objects is not the objects but the *relationships* between pairs of them, the right picture is a graph.

::: check
Six people are at a party. Some pairs are friends. You want to know the smallest number of introductions needed to get from Ana to Farid, where each introduction is made by a mutual friend. You start by listing Ana's friends, then *their* friends, and so on. As you work outward, you need to keep track of people you have discovered but whose friends you have not yet listed. In what order should you process them: most recently discovered first, or earliest discovered first? Which data structure is that?

::: answer
Earliest discovered first. You want to finish with everyone one introduction away before moving on to anyone two away, or you might reach Farid by a roundabout route and report too large a number. First discovered, first processed is a queue. The procedure is called breadth-first search, and it is the first algorithm of this chapter.
:::
:::

This chapter is different in character from the ones before it. A graph is less a single data structure than a *setting*, in which the structures you already have (vectors, queues, stacks, priority queues, hash tables) are put to work together. Most of what follows is algorithms. Pay attention to which earlier structure sits at the heart of each one. It is rarely an accident.

## Graphs and how to store them {#sec:graph-rep}

A graph consists of a set of **vertices** (also called nodes) and a set of **edges**, each of which connects two vertices.

- In an **undirected** graph, an edge is a two-way connection: if $u$ is joined to $v$, then $v$ is joined to $u$. Friendship is like this.
- In a **directed** graph, each edge has a direction, from one vertex to another. "Page $u$ links to page $v$" and "course $u$ is a prerequisite of course $v$" are like this.
- In a **weighted** graph, each edge carries a number: a distance, a cost, a time.

Two vertices joined by an edge are **adjacent**, or **neighbors**. A **path** is a sequence of vertices in which each is adjacent to the next. A **cycle** is a path that returns to where it began. An undirected graph is **connected** if there is a path between every pair of vertices; if not, it falls into several **connected components**.

It is conventional to write $V$ for the number of vertices and $E$ for the number of edges, and to state costs in terms of both. A graph with $V$ vertices can have at most about $V^2$ edges (every pair). Most graphs that arise in practice have far fewer: a road junction joins three or four roads however large the country, and a person has a few hundred friends however large the network. Such graphs, where $E$ is much nearer to $V$ than to $V^2$, are called **sparse**.

How should a graph be stored? There are two standard answers, and [[fig:graph-rep]] shows both for one small graph.

::: {.figure #fig:graph-rep}
An undirected graph with six vertices and six edges, and its two standard representations. The adjacency lists give, for each vertex, the vertices it is joined to. The adjacency matrix has a 1 in row $u$, column $v$ exactly when there is an edge between $u$ and $v$.
:::

An **adjacency matrix** is a $V \times V$ table in which the entry in row $u$ and column $v$ says whether there is an edge from $u$ to $v$ (or gives its weight). Testing for a particular edge is a single array access. But the matrix occupies $\Theta(V^2)$ space no matter how few edges there are, and finding all the neighbors of a vertex means scanning a whole row. For a sparse graph with a million vertices, the matrix would have a trillion entries, almost all of them zero.

An **adjacency list** representation stores, for each vertex, a list of its neighbors. The space is $\Theta(V + E)$: proportional to the actual size of the graph. Finding the neighbors of a vertex, which turns out to be the operation that graph algorithms perform constantly, takes time proportional to how many there are.

| | Adjacency matrix | Adjacency lists |
|:--|:--|:--|
| Space | $\Theta(V^2)$ | $\Theta(V + E)$ |
| Is there an edge from $u$ to $v$? | $\Theta(1)$ | $\Theta(\text{number of neighbors of } u)$ |
| Visit all neighbors of $u$ | $\Theta(V)$ | $\Theta(\text{number of neighbors of } u)$ |
| Visit every edge | $\Theta(V^2)$ | $\Theta(V + E)$ |

For sparse graphs, which is to say nearly always, adjacency lists are the right choice, and they are what we will use. The representation is a vector of vectors.

```{.cpp include="ch09/graph.hpp" region="graph"}
```

Vertices are identified by the integers 0 through $V - 1$, so that a vertex can index directly into a vector. This is simple and fast, and it is how nearly all graph code is written. When the vertices of your problem are really names, or coordinates, or web addresses, use a hash map to translate each one to an integer once, at the door.

```{.cpp include="ch09/graph_test.cpp" region="build"}
```

::: wild
At very large scale even a vector of vectors is too scattered, with one heap allocation per vertex. Graph-processing systems and scientific libraries pack the whole thing into two flat arrays, a format called *compressed sparse row*: one long array holding every vertex's neighbor list end to end, and a second array giving the position where each vertex's list starts. It is the adjacency-list idea with the contiguity lesson of [[sec:cache]] applied.
:::

## Breadth-first search {#sec:bfs}

The most basic question about a graph is: starting here, what can I reach, and how far away is it? **Breadth-first search**, or BFS, answers it by exploring outward from a source vertex in waves: first the source, then all its neighbors, then all of *their* neighbors that have not been seen, and so on. [[fig:bfs-layers]] shows the waves on our example graph.

::: {.figure #fig:bfs-layers}
Breadth-first search from vertex 0. The number beside each vertex is its distance from the source, in edges. The search discovers vertices in order of distance: everything at distance 1, then everything at distance 2, and so on. The heavy edges are the ones along which each vertex was first discovered; they form a tree of shortest paths.
:::

Two pieces of bookkeeping are needed. First, a record of which vertices have already been discovered, so that the search does not go around a cycle forever. Second, the collection of vertices that have been discovered but not yet processed, called the **frontier**. As the opening question established, the frontier must be a queue.

```{.cpp include="ch09/algorithms.hpp" region="bfs"}
```

The `dist` vector does double duty. A value of $-1$ means "not yet discovered"; any other value is the vertex's distance from the source. The algorithm takes a vertex `u` from the front of the queue and examines each of its neighbors. Any neighbor seen for the first time is one edge further from the source than `u` is; it is given its distance and added to the back of the queue.

**Why the distances are right.** The queue always holds vertices in order of distance: first some at distance $d$, then some at distance $d + 1$, and never anything else. So vertices are taken from it in order of distance, and a vertex is always discovered from a neighbor that is as close to the source as any of its neighbors. BFS computes the *shortest* path, in number of edges, from the source to every reachable vertex.

**What it costs.** Each vertex enters the queue at most once, because it is marked when first discovered. When a vertex leaves the queue, its adjacency list is scanned once. So over the whole run, every vertex is handled once and every edge is examined once (twice, in an undirected graph). The running time is $\Theta(V + E)$: linear in the size of the graph. You cannot do better and still look at everything.

### Recovering the path {#sec:bfs-path .unnumbered}

Knowing that Farid is three introductions away is less useful than knowing who should make them. The `parent` vector records, for each vertex, the neighbor from which it was discovered. Following parents from any vertex leads back, step by step, to the source, along a shortest path in reverse.

```{.cpp include="ch09/algorithms.hpp" region="path"}
```

The parent links form a tree, rooted at the source, containing one shortest path to every reachable vertex: the heavy edges in [[fig:bfs-layers]]. Storing one parent per vertex is a compact way to store all those paths at once.

### Graphs that are never built {#sec:implicit .unnumbered}

Sometimes the graph is too large to construct, or there is simply no need. A maze drawn on a grid is a graph, with a vertex for each open cell and an edge between adjacent open cells, but the neighbors of a cell can be computed from its coordinates whenever they are wanted.

```{.cpp include="ch09/graph_test.cpp" region="maze"}
```

This is the same BFS, with "for each neighbor" replaced by "for each of the four directions that stays on the grid and is not a wall." A graph like this is called **implicit**. The technique applies to any puzzle or game: a vertex is a configuration (the arrangement of a sliding puzzle, the state of a Rubik's cube), an edge is a legal move, and BFS finds the solution with the fewest moves. The graph of all configurations may be astronomically large. The search builds only the part it visits.

## Depth-first search {#sec:dfs}

Breadth-first search is cautious: it explores everything nearby before venturing further. **Depth-first search**, or DFS, is the opposite. It follows a path as far as it can go, and only when it reaches a dead end does it back up to the most recent vertex with an unexplored neighbor and try another way. It is how you would explore a maze on foot, with chalk to mark where you had been.

"Back up to the most recent" is the behavior of a stack, and the simplest way to get one is to use the call stack, by writing the search recursively.

```{.cpp include="ch09/algorithms.hpp" region="dfs"}
```

Mark the vertex; then, for each neighbor not yet visited, search from *there* before looking at the next neighbor. When the recursive call returns, everything reachable through that neighbor has been visited.

Like BFS, DFS visits each vertex once and examines each edge once: $\Theta(V + E)$. It does not find shortest paths. What it offers instead is that it finishes exploring everything below a vertex before it leaves that vertex, and several problems are naturally solved by that structure.

::: check
Compare `bfs` and `dfs` with the `level_order` and `pre_order` tree traversals of [[sec:trees]]. What corresponds to what? What did the tree traversals not need that the graph searches do?

::: answer
Level-order is BFS: a queue of discovered-but-unprocessed nodes. Pre-order is DFS: recursion, going deep first. A tree is a graph with no cycles and exactly one path to each node, so a tree traversal can never arrive at a node twice. A graph search can, which is why it needs the visited marks. A tree traversal is a graph search that gets to skip the bookkeeping.
:::
:::

### Connected components {#sec:components .unnumbered}

A search from one vertex of an undirected graph visits exactly the vertices in that vertex's connected component. To find all the components, start a search from each vertex that no earlier search has reached.

```{.cpp include="ch09/algorithms.hpp" region="components"}
```

The outer loop looks as though it might make this quadratic, but it does not: the visited marks persist across the searches, so each vertex is still visited once and each edge examined once in total.

### Ordering tasks: topological sort {#sec:topo .unnumbered}

Suppose the vertices are tasks and a directed edge from $u$ to $v$ means "$u$ must be done before $v$." Courses and their prerequisites are the classic example. So are the steps of a recipe, the source files of a program that must be compiled before the files that include them, and the cells of a spreadsheet that must be recalculated before the cells that refer to them.

::: {.figure #fig:topo}
A prerequisite graph and one topological order of it. Every edge points from left to right, so taking the courses in this order never violates a prerequisite. There are usually several valid orders.
:::

A **topological order** of a directed graph is a listing of its vertices in which every edge points from an earlier vertex to a later one. Such an order exists exactly when the graph has no directed cycle. (If $u$ must precede $v$ and $v$ must precede $u$, nothing can be done.) A directed graph with no cycles is called a **directed acyclic graph**, or DAG, and DAGs are everywhere that dependencies are.

One algorithm follows the way you would actually plan your courses. Find a course with no prerequisites, and take it. That frees up any course for which it was the last unmet prerequisite. Repeat.

```{.cpp include="ch09/algorithms.hpp" region="topo"}
```

The algorithm first counts, for each vertex, how many edges point into it: its **in-degree**. The vertices with in-degree zero are ready. Each time a vertex is output, the in-degrees of its successors are decremented, and any that reach zero become ready. If the graph has a cycle, the vertices on the cycle never become ready, the queue runs dry early, and the function reports failure by returning an empty order. So this algorithm also *detects* cycles. Its cost is again $\Theta(V + E)$.

```{.cpp include="ch09/graph_test.cpp" region="courses"}
```

::: wild
Topological ordering runs constantly in the software you use. Build tools such as `make` and package managers use it to decide what to compile or install first. Spreadsheets use it to recalculate cells. And it is at the center of how neural networks are trained: the arithmetic that a model performs is recorded as a DAG of operations, and the training algorithm, backpropagation, computes its results by walking that graph in *reverse* topological order, so that each operation's contribution is known before it is needed by the operations that fed into it.
:::

## Shortest paths with weights {#sec:dijkstra}

Breadth-first search finds the path with the fewest edges. On a road map that is the route with the fewest road segments, which is not what anyone wants. When edges have weights, the length of a path is the *sum* of its weights, and the shortest path may well use more edges.

The classic algorithm for this was devised by Edsger Dijkstra in 1956, by his own account in about twenty minutes at a café table [@dijkstra1959]. It works whenever **no edge has a negative weight**.

The idea is to generalize BFS's waves. BFS processes vertices in order of their distance from the source, and that worked because the next-closest vertex was always at the front of a queue. With weights, the vertices still need to be processed in order of distance, but the next-closest one is no longer simply the oldest. We need a structure that hands over the *closest* unprocessed vertex each time: a priority queue.

Keep, for every vertex, the length of the shortest path to it found *so far*, initially infinity. Repeatedly take the unprocessed vertex with the smallest such distance. That distance is now final; we say the vertex is **settled**. Then look at each edge leaving it: if going through this vertex and along that edge gives a shorter route to the neighbor than any found before, record the improvement. This update is called **relaxing** the edge.

```{.cpp include="ch09/algorithms.hpp" region="dijkstra"}
```

The priority queue holds pairs of (distance, vertex), ordered by distance, smallest first. (Recall from [[sec:heap]] that `std::priority_queue` needs `std::greater` to behave as a min-heap.)

One practical detail deserves a comment. When a shorter route to a vertex is found, the ideal would be to lower that vertex's priority where it sits in the heap. The standard priority queue has no such operation. So the code simply pushes a *second* entry for the vertex, with the better distance, and leaves the old one in place. The better entry will come out first. When the outdated one eventually surfaces, it is recognized, because its distance is greater than the one now on record, and skipped. That is the meaning of the line marked "a stale entry." This trick is known as *lazy deletion*, and it is the usual way to write Dijkstra's algorithm in practice.

```{.cpp include="ch09/graph_test.cpp" region="weighted"}
```

::: {.viz #fig:graph name="graph"}
Dijkstra's algorithm on a small road network, complete. Each vertex shows its final distance from vertex 0, and the heavy edges form the tree of shortest paths. Note that the best route from 0 to 1 goes through 2, with total weight 3, and not along the direct edge of weight 4. In the web edition, step through breadth-first search, depth-first search, and Dijkstra's algorithm on this and other graphs, and watch the queue, stack, or priority queue as it changes.
:::

Here is the run on that graph, step by step.

| Step | Popped | Action | Distances to 0, 1, 2, 3, 4 |
|:-:|:--|:--|:--|
| | | start | 0, ∞, ∞, ∞, ∞ |
| 1 | (0, vertex 0) | relax 0→1 to 4; relax 0→2 to 1 | 0, 4, 1, ∞, ∞ |
| 2 | (1, vertex 2) | relax 2→1 to 3; relax 2→3 to 9 | 0, 3, 1, 9, ∞ |
| 3 | (3, vertex 1) | relax 1→3 to 8 | 0, 3, 1, 8, ∞ |
| 4 | (4, vertex 1) | stale: skip | |
| 5 | (8, vertex 3) | relax 3→4 to 11 | 0, 3, 1, 8, 11 |
| 6 | (9, vertex 3) | stale: skip | |
| 7 | (11, vertex 4) | nothing to improve | 0, 3, 1, 8, 11 |

### Why it works, and when it does not {#sec:dijkstra-why .unnumbered}

The crucial claim is that when a vertex is taken from the priority queue (and is not stale), its recorded distance is truly the shortest. Why can no shorter route turn up later?

Suppose vertex $u$ is popped with distance $d$. Any other route to $u$ would have to leave the set of already-settled vertices at some point, passing through some vertex $x$ that is still in the queue. But $u$ was the *closest* thing in the queue, so the distance to $x$ is at least $d$. The rest of the route, from $x$ on to $u$, can only add to that. So no route through $x$ is shorter than $d$.

Look at the last step of that argument: "can only add." It assumes that edge weights are not negative. With a negative edge, a route that wanders further away could come back shorter, and a vertex could be settled too soon.

::: check
Consider a directed graph with edges A→B of weight 1, A→C of weight 3, and C→B of weight $-5$. What distance does Dijkstra's algorithm, as written, settle for B on first popping it? What is the true shortest distance?

::: answer
Dijkstra's algorithm pops B first, with distance 1, since it is the closest. The true shortest path is A→C→B, with length $3 + (-5) = -2$. With this lazy-deletion implementation the later relaxation would push a new entry for B and the final number would happen to come out right in this tiny case, but anything computed *from* B in the meantime would have used the wrong value, and in general the algorithm's guarantee is gone. Graphs with negative edges need a different algorithm (Bellman–Ford), which you will meet in an algorithms course.
:::
:::

### What it costs {#sec:dijkstra-cost .unnumbered}

Every edge can cause at most one push onto the priority queue, so the queue never holds more than $E$ entries, and each push or pop costs $O(\log E)$, which is $O(\log V)$. The total is

$$O\big((V + E) \log V\big).$$

Before heaps were in common use, the algorithm was implemented by scanning an array of all the vertices to find the closest unsettled one, which takes $\Theta(V^2)$ time in total. On sparse graphs the difference is dramatic.

| Vertices (5 edges each) | Array scan | Priority queue |
|--:|--:|--:|
| 2,000 | 0.005 s | 0.0002 s |
| 8,000 | 0.086 s | 0.0011 s |
| 32,000 | 2.2 s | 0.0058 s |
| 128,000 | | 0.027 s |

Quadrupling the graph multiplies the array-scan time by about twenty and the heap time by about five. The same algorithm, with a better data structure inside it, goes from unusable to instantaneous. That is the reason this is a course in data structures.

::: byhand
On paper, run Dijkstra's algorithm on a weighted graph of six or seven vertices that you draw yourself, from a source of your choosing. Keep a table like the one above: what was popped, what was relaxed, the distance array, and the contents of the priority queue after each step, including stale entries. Then run BFS on the same graph, ignoring weights, and compare the two shortest-path trees.

Then check your work by running the code, not by asking a model to "trace Dijkstra on this graph." A language model produces a trace by predicting what such a trace looks like, and over a dozen steps of carefully interdependent state it is quite capable of relaxing an edge that does not exist or losing an entry from the queue, while the prose around the error remains perfectly confident. For anything beyond a toy, the reliable way to get an algorithm's output from a model is to have it write the program and then run the program yourself.
:::

## Union-find {#sec:union-find}

The last structure of the chapter solves a narrower problem than the searches do, and solves it almost unbelievably well.

Suppose a graph is being built up one edge at a time, and at any moment you may be asked: are these two vertices connected yet? This is the **dynamic connectivity** problem. Running a new search for every question would take linear time per question. But we do not need paths. We need only to know which vertices are in the same component, and to be able to merge two components when an edge joins them.

The abstract data type is called **disjoint sets**, or **union-find** after its two operations.

| Operation | Meaning |
|:--|:--|
| `find(x)` | return a representative element of the set containing `x` |
| `unite(a, b)` | merge the sets containing `a` and `b` |

Two elements are in the same set exactly when `find` returns the same representative for both.

The representation is a forest: each set is a tree, in which every element points to a **parent**, and the root of the tree, which is its own parent, is the set's representative. These trees are unusual in two ways. The pointers go *up*, from child to parent, since the only thing we ever do is climb to the root. And the trees do not need nodes at all. Since elements are numbered 0 through $n - 1$, the entire forest is one array: `parent[x]` is the parent of `x`.

::: {.viz #fig:unionfind name="unionfind"}
A union-find structure over eight elements, shown as a forest and as the `parent` array that actually stores it. There are three sets, with representatives 0, 4, and 7. In the web edition, unite sets and run `find`, and watch path compression flatten the trees.
:::

`find` climbs parent links to the root. `unite` finds the two roots and makes one the parent of the other. That is the whole idea, and in its naive form it has the same flaw as the binary search tree: nothing stops the trees from growing tall and thin, in which case `find` takes linear time. Two small refinements fix it.

**Union by size.** When merging two trees, always attach the *smaller* one beneath the root of the larger. An element's depth increases only when its tree is absorbed into one at least as big, so each time an element gets one level deeper, the size of its tree has at least doubled. That can happen at most $\log_2 n$ times. The trees have logarithmic height.

**Path compression.** After `find` has climbed from `x` to the root, it goes back over the path and points every element on it *directly* at the root. The next `find` for any of them takes one step. The tree gets flatter every time it is used.

```{.cpp include="ch09/union_find.hpp" region="union_find"}
```

With both refinements, the cost of a sequence of operations was determined by Robert Tarjan in 1975, in one of the most celebrated analyses in the field [@tarjan1975]. The amortized cost per operation is $O(\alpha(n))$, where $\alpha$ is the *inverse Ackermann function*, a function that grows so slowly that $\alpha(n) \le 4$ for any $n$ up to far beyond the number of atoms in the universe. It is not constant. But no computation that could ever be carried out will see it exceed four.

Here is the difference the two refinements make. The test unites $n$ elements into one long chain, the worst case for the naive version, and then calls `find` on each.

| $n$ | naive | union by size + path compression |
|--:|--:|--:|
| 10,000 | 0.078 s | 0.00006 s |
| 20,000 | 0.28 s | 0.00011 s |
| 40,000 | 1.1 s | 0.00021 s |
| 80,000 | 4.4 s | 0.00046 s |

The naive version is quadratic. The refined one is, to the eye, linear: about ten lines of code separate a structure that chokes on eighty thousand elements from one that handles twenty million operations on ten million elements at around 36 nanoseconds apiece.

### An application: the cheapest network {#sec:kruskal .unnumbered}

Suppose the vertices are buildings, and each weighted edge is the cost of laying cable between two of them. We want to connect every building, directly or indirectly, as cheaply as possible. The solution is a set of edges that connects all the vertices with no cycles (a cycle would mean a redundant cable) and with the smallest possible total weight: a **minimum spanning tree**.

Kruskal's algorithm [@kruskal1956] finds one with a strategy of pure greed. Consider the edges from cheapest to most expensive. Take each edge unless its two endpoints are already connected by edges taken earlier, in which case it would close a cycle, so skip it.

```{.cpp include="ch09/graph_test.cpp" region="kruskal"}
```

"Are these two vertices already connected?" is exactly the question union-find answers, and "now they are" is `unite`. With it, the algorithm is a sort followed by a single pass: $O(E \log E)$ for the sort, and effectively linear for everything else. It is not obvious that so greedy a strategy gives the best possible answer. It does, and the proof belongs to an algorithms course.

::: llm
With graphs, the algorithms are the easy part. The hard part, and the part that makes someone good at this, is *modeling*: seeing that a problem which never mentions graphs is one. Use a model to practice.

> Give me a short word problem, with no mention of graphs or of any algorithm by name, that is best solved by exactly one of: breadth-first search, topological sort, Dijkstra's algorithm, or union-find. I will tell you what the vertices are, what the edges are, whether they are directed or weighted, and which algorithm applies. Then critique my modeling, especially if there is a better choice of vertices.

Do ten of these. Pay particular attention to problems where a vertex is not a physical thing but a *state*: a configuration of a puzzle, a position plus the amount of fuel remaining, a set of tasks already completed.
:::

Here are the chapter's algorithms side by side.

| Algorithm | Structure at its heart | Cost | Answers |
|:--|:--|:--|:--|
| Breadth-first search | queue | $\Theta(V + E)$ | fewest-edge paths; what is reachable |
| Depth-first search | stack (recursion) | $\Theta(V + E)$ | reachability; components; cycles |
| Topological sort | queue, in-degree counts | $\Theta(V + E)$ | a valid order for dependencies |
| Dijkstra's algorithm | priority queue | $O((V + E) \log V)$ | least-weight paths, weights $\ge 0$ |
| Union-find | forest in an array | nearly $O(1)$ per operation | are two things connected yet? |
| Kruskal's algorithm | sort, then union-find | $O(E \log E)$ | minimum spanning tree |

::: {.fourq title="Graph (adjacency lists)"}
Where does it live?
:   A vector with one entry per vertex, each entry a vector of that vertex's outgoing edges. Vertices are integers, so they index directly. Space is proportional to vertices plus edges.

What does it promise?
:   `neighbors(u)` lists exactly the edges leaving `u`. An undirected edge is stored as two directed ones, and the two must be kept consistent.

What does it cost?
:   Visiting a vertex's neighbors takes time proportional to their number. Any algorithm that looks at the whole graph, such as BFS or DFS, takes $\Theta(V + E)$.

How do you know?
:   Test each algorithm against a slower, simpler, independent one on many small random graphs: Dijkstra against Bellman–Ford, BFS against Dijkstra with unit weights, Kruskal against a different spanning-tree algorithm. Include disconnected graphs, self-loops, and repeated edges.
:::

::: {.fourq title="Union-find"}
Where does it live?
:   Two arrays of integers. `parent[x]` is the element above `x` in its tree; a root is its own parent. A forest with no nodes and no pointers.

What does it promise?
:   Two elements are in the same set exactly when climbing from each reaches the same root. With union by size, a tree of height $h$ has at least $2^{h-1}$ elements.

What does it cost?
:   Amortized $O(\alpha(n))$ per operation, where $\alpha(n) \le 4$ in practice. It cannot split a set or list a set's members efficiently.

How do you know?
:   Compare its answers with connectivity computed by a graph search over the same edges; check that the number of sets equals the number of connected components.
:::

## Exercises {#sec:graphs-ex .unnumbered}

1. Draw the directed graph with vertices 0 through 5 and edges 0→1, 0→3, 1→2, 1→3, 3→4, 4→1, 5→4. Give its adjacency lists and its adjacency matrix. Does it contain a cycle? Which vertices are reachable from 5? From 2?

2. For each scenario, say what the vertices and edges are, whether the graph is directed and whether it is weighted, and whether you would expect it to be sparse or dense: (a) flights between airports; (b) which pairs of students have a class together; (c) the "follows" relation on a social network; (d) the squares of a chessboard and a knight's moves; (e) the tiles of a sliding puzzle.

3. Run breadth-first search by hand on the graph of [[fig:graph-rep]] starting from vertex 5. Give the order in which vertices are removed from the queue, and the final `dist` and `parent` arrays. Then give the order in which `dfs` visits vertices starting from 5.

4. A graph has $V$ vertices and is stored as an adjacency matrix. What is the running time of BFS? Explain why it differs from the $\Theta(V + E)$ of the adjacency-list version.

5. Rewrite `dfs` without recursion, using an explicit `std::stack<int>`. Does your version visit vertices in the same order as the recursive one? If not, what would you change to make it do so? Why might an iterative version be necessary for a very large graph?

6. An undirected graph is **bipartite** if its vertices can be colored with two colors so that every edge joins vertices of different colors. Write a function that determines whether a graph is bipartite, using BFS. (What must be true of the distances of two adjacent vertices?)

7. Give all the valid topological orders of the course graph in [[sec:dfs]]. Then add the smallest set of edges you can that leaves exactly one valid order.

8. Run Dijkstra's algorithm by hand on the `roads` graph from the source vertex 4, with a table like the one in [[sec:dijkstra]]. Which entries in the priority queue turn out to be stale?

9. Modify `dijkstra` to return a `parent` vector as `bfs` does, so that the shortest paths themselves can be recovered. At exactly which line should a vertex's parent be recorded?

10. Suppose every edge of a weighted graph has its weight increased by the same constant $c$. Does the shortest path between two given vertices stay the same? Give a proof or a small counterexample. (This is why "just add a constant to make all the weights positive" does not rescue Dijkstra's algorithm from negative edges.)

11. Start with elements 0 through 7 in separate sets, and perform `unite(0,1)`, `unite(2,3)`, `unite(4,5)`, `unite(6,7)`, `unite(0,2)`, `unite(4,6)`, `unite(0,4)`, using union by size, with ties broken as in the code. Draw the forest after each step. Then perform `find(7)` with path compression and draw the result.

12. Prove that with union by size (and no path compression), a tree of height $h$ contains at least $2^{h-1}$ elements. Conclude that `find` takes $O(\log n)$ time.

13. An image is a grid of pixels, each black or white. Two black pixels belong to the same *blob* if they are adjacent horizontally or vertically. Write a function that counts the blobs, once using a graph search and once using union-find. Which would you use if pixels were being turned black one at a time and you needed the count after each change?

14. **With a model, as critic.** A classic puzzle: you have a 3-liter jug and a 5-liter jug, and you must measure exactly 4 liters. Model it as a graph yourself, on paper: what is a vertex, what is an edge, and what does a shortest path mean? Then ask a model to critique your modeling without solving the puzzle. Finally, write a BFS over your implicit graph that prints the shortest sequence of moves.

15. **Lab.** *Word ladders.* Given a dictionary and two words of the same length, find a shortest sequence of dictionary words leading from one to the other in which each word differs from the previous one in exactly one letter (`cold`, `cord`, `card`, `ward`, `warm`). The graph is implicit: a word's neighbors are the dictionary words one letter away. Use a hash set for the dictionary and BFS for the search. Consider two ways to generate a word's neighbors, by comparing it with every dictionary word, or by trying every single-letter change and looking each up in the set, and measure both. Report the ladder from `stone` to `money`, or the fact that none exists in your dictionary, and the number of words your search discovered along the way.

## Chapter notes {#sec:graphs-notes .unnumbered}

Graph theory is usually dated to Euler's 1736 solution of the Königsberg bridges problem. The algorithms in this chapter are from the first two decades of computer science: Dijkstra's shortest-path algorithm [@dijkstra1959], Kahn's topological sort [@kahn1962], Kruskal's spanning-tree algorithm [@kruskal1956], and the systematic use of depth-first search by Hopcroft and Tarjan [@hopcroft1973], which earned them a Turing Award. The forest representation of disjoint sets is from Galler and Fischer [@galler1964]; the two refinements followed within a few years, and the remarkable analysis of the combination is Tarjan's [@tarjan1975].

Sedgewick and Wayne [@sedgewick2011] devote a quarter of their book to graphs, with many applications and an outstanding treatment of union-find as an opening case study. Cormen et al. [@clrs2022] give the correctness proofs omitted here, including those for Dijkstra's and Kruskal's algorithms, and the algorithms for graphs with negative weights.
