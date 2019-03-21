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

#ifndef MODEL_GEOMETRY_HPP
#define MODEL_GEOMETRY_HPP

#include <vector>
#include "../config/traits.hpp"

namespace model
{
	
/*================================================================================*/
/*                                   Vector                                       */
/*================================================================================*/

	class Vector
	{
	public:
		static const int x = Traits<Vector>::x;
		static const int y = Traits<Vector>::y;
		static const int z = Traits<Vector>::z;
		static const int w = Traits<Vector>::w;
	
		const static int dimension = Traits<Vector>::dimension;

		Vector() :
			_coordinates{x, y, z, w}
		{
		}

		Vector(const Vector& v) :
			_coordinates{v._coordinates}
		{
		}

		Vector(Vector&& v) :
			_coordinates{v._coordinates}
		{
		}
		
		Vector(double x0, double y0, double z0 = z, double w0 = w) :
			_coordinates{x0, y0, z0, w0}
		{
		}

	    Vector &operator=(const Vector &) = default;
	    Vector &operator=(Vector &&) = default;
		
		~Vector() = default;

		double& operator[](const int position);
		const double& operator[](int position) const;

		double operator*(const Vector& v) const;
		Vector operator*(const double scalar) const;
		Vector operator*(const Matrix& M) const;
	
	private:
		std::vector<double> _coordinates;
	};

/*================================================================================*/
/*                                   Matrix                                       */
/*================================================================================*/

	class Matrix
	{
	public:
        using MatrixLine = Vector;
		const static int dimension = Traits<Vector>::dimension;

	public:

		Matrix(const MatrixLine& l0 = {1, 0, 0, 0},
               const MatrixLine& l1 = {0, 1, 0, 0},
               const MatrixLine& l2 = {0, 0, 1, 0},
               const MatrixLine& l3 = {0, 0, 0, 1}) :
			_vectors{l0, l1, l2, l3}
		{
		}

		Matrix(MatrixLine&& l0,
               MatrixLine&& l1,
               MatrixLine&& l2,
               MatrixLine&& l3) :
			_vectors{l0, l1, l2, l3}
		{
		}
		
		~Matrix() = default;

		Matrix column_oriented() const;

		MatrixLine& operator[](const int position);
		const MatrixLine& operator[](const int position) const;

		Matrix operator*(const Matrix& M) const;
	
	private:
		std::vector<MatrixLine> _vectors;
	};

/*================================================================================*/
/*                                   Vector                                       */
/*================================================================================*/

	double& Vector::operator[](const int position)
	{
		return _coordinates[position];
	}

	const double& Vector::operator[](const int position) const
	{
		return _coordinates.at(position);
	}

	double Vector::operator*(const Vector& v) const
	{
		return _coordinates[0] * v[0]
		     + _coordinates[1] * v[1]
			 + _coordinates[2] * v[2]
			 + _coordinates[3] * v[3];
	}

	Vector Vector::operator*(const double scalar) const
	{
		Vector v(0, 0, 0, 0);

		for (int i = 0; i < dimension; ++i)
			v[i] = scalar * _coordinates.at(i);
		
		return v;
	}

	Vector Vector::operator*(const Matrix& M) const
	{
		Vector v(0, 0, 0, 0);

		for (int j = 0; j < dimension; ++j)
			for (int i = 0; i < dimension; ++i)
				v[j] += _coordinates.at(i) * M[i][j];
		
		return v;
	}

/*================================================================================*/
/*                                   Matrix                                       */
/*================================================================================*/

	Matrix Matrix::column_oriented() const
	{
		return Matrix(
			{_vectors[0][0], _vectors[1][0], _vectors[2][0], _vectors[3][0]},
			{_vectors[0][1], _vectors[1][1], _vectors[2][1], _vectors[3][1]},
			{_vectors[0][2], _vectors[1][2], _vectors[2][2], _vectors[3][2]},
			{_vectors[0][3], _vectors[1][3], _vectors[2][3], _vectors[3][3]}
		);
	}
	
	Matrix::MatrixLine& Matrix::operator[](const int position)
	{
		return _vectors[position];
	}

	const Matrix::MatrixLine& Matrix::operator[](const int position) const
	{
		return _vectors.at(position);
	}

    Matrix Matrix::operator*(const Matrix& M) const
    {
		// Matrix C = M.column_oriented();
		Matrix R( //! Result
			{0, 0, 0, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		);

        for (int h = 0; h < dimension; ++h)
            for (int j = 0; j < dimension; ++j)
                for (int i = 0; i < dimension; ++i)
                    R[h][j] += _vectors[h][i] * M[i][j];

		// for (int i = 0; i < dimension; ++i)
		// 	for (int j = 0; j < dimension; ++j)
		// 		R[i][j] += _vectors[i] * C[j];

        return R;
    }


/*================================================================================*/
/*                                Operations                                      */
/*================================================================================*/

	namespace transformations
	{

		Matrix translation(const Vector& factor)
		{
			if (Traits<Vector>::dimension == 3)
			{
				return Matrix(
					{1, 0, 0, 0},
					{0, 1, 0, 0},
					factor,
					{0, 0, 0, 1}
				);
			}
			else
			{
				return Matrix(
					{1, 0, 0, 0},
					{0, 1, 0, 0},
					{0, 0, 1, 0},
					factor
				);
			}
		}

		Matrix scheduling(const double factor, const Vector& mass_center)
		{
			auto to_origin = translation(mass_center * -1);
			auto go_back   = translation(mass_center);

			return to_origin
			* Matrix(
				{factor, 0, 0, 0},
				{0, factor, 0, 0},
				{0, 0, factor, 0},
				{0, 0, 0, factor}
			)
			* go_back;
		}
		
	} //! namespace operations

} //! namespace model

#endif  // MODEL_GEOMETRY_HPP
