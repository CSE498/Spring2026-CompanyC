**Write-up for Initial C++ Class Development and Module Ideas**  
**Group 8:** Interaction-Heavy Simulation World

**Random**

* Class description:  
  * Random will provide fast and consistent random number generation to be used throughout the world.  
  * The primary goal is to let users efficiently generate uniform random values across number ranges and do any probability based checks.  
  * This class supports generating random floating point values, random integers, and any probability based booleans.  
* Similar classes in the standard library:  
  * std::mt19937  
  * std::default\_random\_engine  
  * std::uniform\_real\_distribution  
  * std::uniform\_int\_distribution  
* Key functions:  
  * double GetDouble(double min, double max);  
    * Returns a random double in a given range  
  * int GetInt(int min, int max);  
    * Returns a random int in a given range  
  * bool P(double probability);  
    * Returns true or false based on provided probability  
  * void Seed(uint64\_t seed);  
    * Sets internal seed value for random generation  
* Error conditions:  
  * Using GetDouble or GetInt with an invalid range (min \> max)  
    * Programmer error, can use assert statement  
  * Using P() with a probability outside of \[0.0, 1.0\]  
    * Programmer error, can use assert statement  
  * Failure to properly initialize internal generator state  
    * Programmer error, can use assert statement  
* Expected challenges:  
  * Ensuring uniform distribution across large numeric ranges while avoiding bias and maintaining performance.  
* Coordination with other groups classes:  
  * Scheduler, ExpressionParser \- Group 7  
  * BehaviorTree, ActionMap \- Group 6  
  * AnnotationSet, FeatureVector \- Group 5  
  * ReplayDriver, ActionLog, Timer \- Group 23

**WeightedSet**

* Class description:  
  * WeightedSet is a templated container in charge of maintaining unique elements with different associated weights (weights must be numerical and positive). The main goals of this class are to have unique elements where each element appears at most one, support efficient indexing by cumulative weight, and allow insertion, deletion, and weight modification all while still maintaining performance.  
* Similar classes in the standard library:  
  * std::set\<T\>, std::map\<K, V\>, std::priority\_queue  
* Key functions:  
  * bool insert(const T& value, double weight);  
  * bool erase(const T& value);  
  * iterator find(const T& value);  
  * size\_t size() const;  
  * bool empty() const;  
  * void clear();  
* Error conditions:  
  * Negative or zero weight on insert/update: programmer error handled with assert  
  * Invalid cumulative weight: programmer error handled with assert  
  * Calling operations on empty set: programmer error handled with assert  
  * Dereferencing invalid iterators: programmer error handled with assert  
  * Memory allocation failure: recoverable error handled with exception  
  * Element not found: recoverable error handled with exception  
  * Inserting duplicate element: user error  
  * Erasing non-existing element: user error  
  * Finding non existing element: user error  
* Expected challenges:  
  * I think the tree structure will be tricky to tackle as well as memory management (I will want this class to perform as best as possible). Figuring out the weight propagation on insert/delete/update in order to keep the accurate cumulative sums at each node will take some time as well.   
* Coordination with other groups classes:  
  * Random number generator class (from our group)  
  * Any data analysis classes (from data analysis group)  
  * Data structure testing classes (from database group)  
  * Other world integration (from dynamic world group)

**DataFileManager**

* Class description:  
  * DataFileManager manages structured data files that are periodically updated over time. Its primary responsibility is to collect a set of user-provided functions, execute them on demand, and write their results as a new row in a data file, with each function contributing a single column value. The class is especially useful for analytics, debugging, simulation output, and long-running experiments where data must be captured incrementally.  
* Similar classes in the standard library:  
  * std::fstream/std::ofstream, std::vector/std::tuple, std::function  
* Key functions:  
  * void addColumn(std::string, std::function);  
    * Registers a function that will be called on update to produce a column value  
  * void update();  
    * Triggers all stored functions in order, collects their results, and appends a new row to the data file  
  * void flush();  
    * Forces any buffered output to be written to disk  
  * void open(std::string filename);  
    * Opens (or creates) the data file and prepares it for writing  
  * void close();  
    * Closes the data file and releases associated resources  
* Error conditions:  
  * Calling update() with no file open; programmer error, can use assert  
  * Registering a null or invalid function; programmer error, can use assert  
  * Mismatch between existing file schema and registered columns; programmer error, behavior must be clearly defined (assert, ignore, overwrite)  
  * File I/O failure (permission, storage, etc.); Runtime error, should be reported with return values or logging  
* Expected challenges:  
  * I think the main challenge with this class will be ensuring consistency across updates. There are a number of issues that could arise, most prominently handling dynamic column registration vs fixed file schemas, as well as coordinating function execution timings to avoid unintended side effects.  
* Coordination with other groups classes:  
  * AnnotationSet, FunctionSet \- Group 5  
  * BehaviorTree, ActionMap, WorldPath \- Group 6  
  * DataMap, Scheduler \- Group 7  
  * Datum, DataGrid, Serializer \- Group 9  
  * DataLog, ActionLog, Timer \- Group 23

**EventQueue**

* Class description:  
  * The EventQueue class will use a heap-based data structure to track and manage a series of scheduled events ordered by its priority value.  
  * The primary goal is to allow users to efficiently determine and control which event happens next in the simulation.   
  * This class supports adding new events, peeking at the next event without removing it, removing the next event, viewing the number of processed events, and tie-breaking two events with the same priority values.  
* Similar classes in the standard library:  
  * std::priority\_queue, std::queue, std::vector  
* Key functions:  
  * void push(const type& value);   
    * Inserts a new event into the EventQueue.  
  * void pop();   
    * Removes the next element from the EventQueue.   
  * const\_reference top() const;   
    * Returns a reference to the topmost event without removing it.  
  * size\_t size() const;   
    *  Returns the number of elements within the EventQueue.  
  * bool empty() const;   
    * Check whether the EventQueue is empty.  
* Error conditions:  
  * Using top() on an empty EventQueue  
    * (1) Programmer error, handled with an assert statement.  
  * Using pop() on an empty EventQueue  
    * (1) Programmer error, handled with an assert statement.  
  * Invalid event data or comparison logic   
    * (1) Programmer error, handled with an assert statement.  
  * Failure to allocate memory when inserting an event  
    * (2) Potentially recoverable error, handled by triggering an exception.  
* Expected challenges:  
  * Implementing a consistent method of breaking ties when two events have the same priority value.

* Coordination with other groups classes:  
  * AnnotationSet, FunctionSet \- Group 5  
  * BehaviorTree, ActionMap  \- Group 6  
  * Scheduler \- Group 7  
  * DataFileManager \- Group 9  
  * ActionLog, ReplayDriver, Timer \- Group 23

**MemoFunction**

* Class description:  
  * This class will work similarly to a cache and its role in memory management. The first time you call the function with some input, it does the real work and saves (“caches”) the result. If you call it again later with the same input, it doesn’t recompute. It just returns the saved answer. It trades a little memory for a faster runtime.  
      
* Similar classes in the standard library:  
  * std::unordered\_map (cache storage), std::map (ordered storage), std::function (store the wrapped callable)  
* Key functions:  
  * output\_type operator()(const input\_type& input)  
    * Call the wrapped function; return cached result if input was seen before, otherwise compute \+ store \+ return.  
  * bool contains(const input\_type& input) const (optional)  
    * Check if input is already in the cache.  
  * void clear()  
    * Delete everything stored in the cache.  
  * size\_t size() const  
    * How many cached inputs are stored.  
  * bool empty() const  
    * True if nothing is cached.  
  * void set\_capacity(size\_t max)   
    * Limit cache size; if full, remove an old entry when adding a new one  
* Error conditions:  
  * Calling MemoFunction when no function/callable was provided  
    * Programmer error assert (or throw std::invalid\_argument, depending on your style).  
  * Using an input type that can’t be used as a key in std::unordered\_map  
    * Programmer/design error, won’t compile unless the type has a hash \+ equality (or you provide them).  
  * Memory allocation fails when caching a new entry  
    * Potentially recoverable  std::bad\_alloc exception.  
  * Eviction enabled, but capacity is invalid (ex: capacity \= 0\) or eviction tracking breaks (only if you implement eviction)  
    * Programmer error: assert (or handle by refusing to store).  
* Expected challenges:  
  * Choosing the right cache size: too small \= lots of recomputes, too big \= memory grows fast.  
  * Eviction policy correctness (if implemented): making sure the “oldest” (or least recently used) entry is removed every time the cache is full.  
  * Keeping structures in sync (if eviction): if you use an unordered\_map \+ a list/queue for order, they must always match.  
  * Building keys for multiple parameters (hard mode): combining inputs into one key and writing a correct hash.  
  * Avoiding accidental inserts: not using cache\[key\] for lookups since it creates empty entries.  
  * Performance tradeoffs: caching helps only when inputs repeat; if inputs are always new, you pay extra overhead for little benefit.

* Coordination with other group classes:  
  * Whole Group 5  
    * AI agents do a lot of repeated queries, and MemoFunction helps it run faster  
  * Group 9 — Database (Serializer, StringCompressor, StringDiff, DataGrid)  
    * serialization/compression/diff operations can be expensive and often repeated with the same input.  
  * Group 7 — Dynamic World (ExpressionParser, DataMap, Scheduler)  
    * parsing/evaluating expressions or looking up dynamic values can be expensive and repeated.

**Vision for the main module:**  
For the interaction-heavy simulation world, our group is thinking about creating a world that is based around a large building structure. Something similar to a large sporting arena, music venue, or even a sky scraper. This will allow us to develop a world where the two agent groups can put different types of agents inside this world and have them interacting with each other or objects, and simulate the outcomes of those interactions. In terms of the different interactable objects/items around the world, realistically they can be anything from a pc desktop, to books, a sound stage, sports field, office desks, etc. We are still waiting on the other groups in the company in order to decide exactly what to go for. The types of agents we are thinking about can also be just about anything, a small person, a blob (like “amongus” type), or really whatever the two agent groups decide on.  
	Another idea we played with during internal group discussions was a tile-based world where the user interacts with different tile types to allow the agents to traverse the environment effectively. An example of this idea would be the puzzle game Mini Motorways, in which the player uses the tiles they are given to manipulate the grid and allow the cars (agents) a route to and from their destinations.