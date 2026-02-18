My name is George Almeida and I created the WeightedSet class for Group 8 in Company C.

At a high level, it's a templated container that stores unique elements each with a positive weight attached, and the main thing it enables beyond a regular set is weighted sampling where you can ask it to pick an element proportional to its weight. The class lives in the cse498 namespace and takes a type parameter plus an optional comparator, just like std::set does.

Before getting into the methods, I want to call out the type aliases up top. Naming double as weight_type and the underlying std::map as map_type was a deliberate choice as it keeps the rest of the code readable and makes it easy to swap the weight precision later if needed. The underlying storage is just a std::map, which gives us sorted unique keys.

Insert is straightforward. The assert at the top enforces that weights are strictly positive.  If the element already exists the function just returns false.

Erase and find follow the same pattern. They return false or end() on a miss rather than throwing an exception, since not finding something is a normal outcome the caller should handle. Erase also decrements m_total before removing the node, keeping that running sum consistent.

Sample_by_weight takes a cumulative weight value and walks the map linearly, accumulating weights until it crosses the threshold. The comment about floating point rounding at the bottom is important: when the input equals m_total exactly, you can fall out of the loop due to precision, so the fallback just returns the last element rather than asserting or returning end().

Finnaly, sample_random takes any uniform random bit generator (so it will work with our group's Random class directly) and generates a value from 0 to total_weight using nextafter to keep the distribution closed on the right, and delegates to sample_by_weight. 

The private section is just the map and the running total.