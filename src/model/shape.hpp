/* The MIT License
 * 
 * Copyright (c) 2019 João Vicente Souto and Bruno Izaias Bonotto
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MODEL_SHAPE_HPP
#define MODEL_SHAPE_HPP

#include <gtkmm/drawingarea.h>
#include "vector.hpp"

namespace model
{
	
	class Shape
	{	
	public:
		Shape()  = default;

		Shape(const Vector& v) :
			_vectors{{v}}
		{}
		
		Shape(double x, double y, double z = Vector::_z, double w = Vector::_w) :
			_vectors{{x, y, z, w}}
		{}
		
		Shape(std::initializer_list<Vector>& vs) :
			_vectors{vs}
		{}

		Shape(std::initializer_list<Vector>&& vs) :
			_vectors{vs}
		{}

		~Shape() = default;

        virtual void draw(const Cairo::RefPtr<Cairo::Context>& cr);
	
	protected:
		std::vector<Vector> _vectors{{0, 0}};
	};

    void Shape::draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
		/* First point */
		cr->move_to(_vectors[0][0], _vectors[0][1]);

		// Draw all other points
		for (Vector& v : _vectors)
			cr->line_to(v[0], v[1]);
    }

} //! namespace model

#endif  // MODEL_SHAPE_HPP
