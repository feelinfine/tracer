#pragma once

#include <set>
#include <algorithm>
#include <future>

/*
Require C++14 

Usage
{
	using namespace trace;

	std::vector<V2d> points = { {200, 200}, {400, 400}, {500,100} };

	size_t cell_size = 30;

	auto cells = pick_cells(points, cell_size);

	for (auto& it : cells)
		...					//do something with cells
}


Coors (cells and points)
 ^
n|
.|
.|
1|
0|------------>
  0 1 . . .  n
*/


namespace trace
{

template <typename _T>
struct V2
{
	_T x, y;

	V2(_T _x = 0, _T _y = 0) : x(_x), y(_y)
	{
	};

	V2 operator-(const V2& _rhs) const
	{
		return V2(x - _rhs.x, y - _rhs.y);
	}

	bool operator==(const V2& _rhs) const
	{
		return (x == _rhs.x) && (y == _rhs.y);
	}

	//for std::set sorting
	bool operator<(const V2& _rhs) const
	{
		return (x == _rhs.x) ? (y < _rhs.y) : (x < _rhs.x);
	}
};

using V2d = V2<double>;
using V2i = V2<int>;

template <typename Container>
std::set<V2i> pick_cells(Container&& _points, size_t _cell_size)
{
	if (_points.size() < 2 || _cell_size <= 0)
		return std::set<V2i>();

	auto add_to_set = [](auto& _set, const auto& _to_append)
	{
		_set.insert(std::cbegin(_to_append), std::cend(_to_append));
	};

	//Outline
	std::set<V2i> cells;

	/*
	for (auto it = std::begin(_points); it != std::prev(std::end(_points)); ++it)
		add_to_set(cells, trace_line(*it, *std::next(it), _cell_size));

	add_to_set(cells, trace_line(_points.back(), _points.front(), _cell_size));

	*/

	//Maybe this code works faster
	std::vector<std::future<std::set<V2i> > > results;

	using PointType = decltype(_points.cbegin())::value_type;

	for (auto it = _points.cbegin(); it != std::prev(_points.cend()); ++it)			//до предпоследнего
		results.push_back(std::async(trace_line<PointType>, *it, *std::next(it), _cell_size));

	results.push_back(std::async(trace_line<PointType>, _points.back(), _points.front(), _cell_size));	//линия от последней точки к первой

	for (auto& it : results)
		add_to_set(cells, it.get());

	//Inner
	std::set<V2i> to_add;

	int last_x = cells.begin()->x;
	int counter = cells.begin()->y;

	for (auto& it : cells)
	{
		if (last_x != it.x)
		{
			counter = it.y;
			last_x = it.x;
		}

		if (it.y > counter)	
		{
			for (int i = counter; i < it.y; ++i)
				to_add.insert(V2i(it.x, i));
		}

		++counter;
	}

	add_to_set(cells, to_add);

	return cells;
}

template <typename PointType>
std::set<V2i> trace_line(const PointType& _start_point, const PointType& _end_point, size_t _cell_size)
{
	/*
	Algorithm

	http://www.cse.chalmers.se/edu/year/2011/course/TDA361/Advanced%20Computer%20Graphics/grid.pdf

	*/

	auto point_to_grid_fnc = [_cell_size](const auto& _point)
	{
		return V2i(std::floor((double)_point.x / _cell_size), std::floor((double)_point.y / _cell_size));
	};

	V2i start_cell = point_to_grid_fnc(_start_point);
	V2i last_cell = point_to_grid_fnc(_end_point);

	PointType direction = _end_point - _start_point;

	//Moving direction (cells)
	int step_x = (direction.x >= 0) ? 1 : -1;
	int step_y = (direction.y >= 0) ? 1 : -1;

	//Normalize vector
	double hypot = std::hypot(direction.x, direction.y);
	V2d norm_direction(direction.x / hypot, direction.y / hypot);

	//Distance to the nearest square side
	double near_x = (step_x >= 0) ? (start_cell.x + 1)*_cell_size - _start_point.x : _start_point.x - (start_cell.x*_cell_size);	
	double near_y = (step_y >= 0) ? (start_cell.y + 1)*_cell_size - _start_point.y : _start_point.y - (start_cell.y*_cell_size);

	//How far along the ray we must move to cross the first vertical (ray_step_to_vside) / or horizontal (ray_step_to_hside) grid line
	double ray_step_to_vside = (norm_direction.x != 0) ? near_x / norm_direction.x : std::numeric_limits<double>::max();
	double ray_step_to_hside = (norm_direction.y != 0) ? near_y / norm_direction.y : std::numeric_limits<double>::max();

	//How far along the ray we must move for horizontal (dx)/ or vertical (dy) component of such movement to equal the cell size
	double dx = (norm_direction.x != 0) ? _cell_size / norm_direction.x : std::numeric_limits<double>::max();
	double dy = (norm_direction.y != 0) ? _cell_size / norm_direction.y : std::numeric_limits<double>::max();

	//Tracing loop
	std::set<V2i> cells;
	cells.insert(start_cell);

	V2i current_cell = start_cell;

	size_t grid_bound_x = std::abs(last_cell.x - start_cell.x);
	size_t grid_bound_y = std::abs(last_cell.y - start_cell.y);

	size_t counter = 0;

	while (counter != (grid_bound_x + grid_bound_y))
	{
		if (std::abs(ray_step_to_vside) < std::abs(ray_step_to_hside))
		{
			ray_step_to_vside = ray_step_to_vside + dx;	//to the next vertical grid line
			current_cell.x = current_cell.x + step_x;
		}
		else
		{
			ray_step_to_hside = ray_step_to_hside + dy;//to the next horizontal grid line
			current_cell.y = current_cell.y + step_y;
		}

		++counter;

		cells.insert(current_cell);
	};

	return cells;
}

} // end namespace trace