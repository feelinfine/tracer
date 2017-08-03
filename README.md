# tracer
A C++ implementation of "A Fast Voxel Traversal Algorithm for Ray Tracing" (John Amanatides, Andrew Woo). 

Works with any ray direction. Modified for selection by frame or convex polygon on a grid.

This is a header-only implementation. Requires a C++14 compiler. 

Usage example: 

```C++
{
	using namespace trace;

	std::vector<V2d> points = { {102, 363}, {304, 503}, {515,387}, {423,147}, {49,71} };

	size_t cell_size = 30;

	auto cells = pick_cells(points, cell_size);

	for (auto& it : cells)
		...		//do something with cells
}
```
Result example (any GUI and graphical representation is not included)

![alt tag](https://github.com/feelinfine/tracer/blob/master/image.jpg "Example picture")
