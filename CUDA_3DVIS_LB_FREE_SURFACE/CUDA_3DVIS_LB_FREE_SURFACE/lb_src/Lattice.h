#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>

#pragma once

# ifndef LATTICE
# define LATTICE

using namespace std;

//Macro to linearly go over the arrays
#define I3D(width, height,i,j,k)				width*(j+height*k)+i

//Macro to go over arrays that have a stride other than 1
#define I3D_S(width, height, stride, i,j,k,l)	(width*(j+height*k)+i)*stride+l

#define FILL_OFFSET			((float)0.003)
#define LONELY_THRESH		((float)0.1)

struct float3
{
	float x, y, z;

	float3& operator +=(const float3& rh)
	{
		x += rh.x;
		y += rh.y;
		z += rh.z;
		return *this;
	}
	
	inline float3 operator+(const float3& rhs)
	{
		*this += rhs;
		return *this;
	}

	bool operator ==(const float3& rh)
	{
		return (x == rh.x && y == rh.y && z == rh.z);
	}
};

enum cell_types{
	gas = 1 << 0,
	fluid = 1 << 1,
	interphase = 1 << 2,
	solid = 1 << 3,
	CT_NO_FLUID_NEIGH = 1 << 4,
	CT_NO_EMPTY_NEIGH = 1 << 5,
	CT_NO_IFACE_NEIGH = 1 << 6,
	CT_IF_TO_FLUID = 1 << 7,
	CT_IF_TO_EMPTY = 1 << 8
};

static inline float dot(float3 a, float3 b)
{
	return a.x*b.x + a.y*b.y + a.z * b.z;
}

static inline float3 float3_ScalarMultiply(const float s, const float3 v)
{
	return float3{ s*v.x, s*v.y, s*v.z, };
}

static inline float float3_Norm(const float3 v)
{
	// have to change to 'fabs' for 'typedef double real'
	float a = fabsf(v.x), b = fabsf(v.y), c = fabsf(v.z);

	if (a < b)
	{
		if (b < c)
			return c*sqrtf(1 + sqrtf(a / c) + sqrtf(b / c));
		else	// a < b, c <= b
			return b*sqrtf(1 + sqrtf(a / b) + sqrtf(c / b));
	}
	else	// b <= a
	{
		if (a < c)
			return c*sqrtf(1 + sqrtf(a / c) + sqrtf(b / c));
		else	// b <= a, c <= a
		{
			if (a != 0)
				return a*sqrtf(1 + sqrtf(b / a) + sqrtf(c / a));
			else
				return 0;
		}
	}
}

static float latticeWeights[19] =
{
	1.f / 9.f,
	1.f / 18.f, 1.f / 18.f, 1.f / 18.f, 1.f / 18.f, 1.f / 18.f, 1.f / 18.f,
	1.f / 36.f, 1.f / 36.f, 1.f / 36.f, 1.f / 36.f, 1.f / 36.f, 1.f / 36.f,
	1.f / 36.f, 1.f / 36.f, 1.f / 36.f, 1.f / 36.f, 1.f / 36.f, 1.f / 36.f
};

static float3 speedDirection[19] =
{
	{0, 0, 0},
	{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0},
	{0, 0, 1}, {0, 0, -1}, {1, 1, 0}, {1, -1, 0},
	{1, 0, 1}, {1, 0, -1}, {-1, 1, 0}, {-1, -1, 0},
	{-1, 0, 1}, {-1, 0, -1}, {0, 1, 1}, {0, 1, -1},
	{0, -1, 1}, {0, -1, -1}
};

static int inverseSpeedDirectionIndex[19] =
{
	0, 
	2, 1, 4, 3, 
	6, 5, 12, 11, 
	14, 13, 8, 7,
	10, 9, 18, 17,
	16, 15
};

static const float v_max = 0.816496580927726f;		//!< set maximum velocity to sqrt(2/3), such that f_eq[0] >= 0

class latticed3q19
{
private:
	int				_width, _height, _depth, _stride, _numberAllElements, _numberLatticeElements;

	float			*ro, rovx, rovy, rovz, v_sq_term;

	float			_cellsPerSide, cellSize, viscosity, timeStep, _domainSize, gravity, latticeAcceleration;

	// Epsilon is the fluid fraction of a given cell
	float			*f, *ftemp, *feq, *epsilon;
	
	// W -> relaxation time; Values (0..2]; tending to 0 = more viscous
	float			_tau, c, _w;

	// Mass of the entire fluid, and mass of single cells of the fluid.
	float			_mass, *cellMass, *cellMassTemp;

	vector<float3>	_filledCells, _emptiedCells;
	
	// Dirichlet and Neumann Boundary Conditions
	void boundary_BC(float3 inVector);

	// Solid Boundary: This is the boundary condition for a solid node. All the f's are reversed - this is known as "bounce-back"
	void solid_BC(int i0);

	void in_BC(float3 inVector);

	// Move the f values one grid spacing in the directions that they are pointing
	// i.e. f1 is copied one location to the right, etc.
	void stream(void);

	//Boundary Conditions
	void applyBoundaryConditions(void);

	// Collision using Single Relaxation time BGK
	void collide(void);

	void calculateSpeedVector(int index);
	
	void calculateEquilibriumFunction(float3 inVector, float inRo);

	// Calculate derived quantities density and velocity from distribution functions
	void deriveQuantities(int index);

	float calculateEpsilon(int cellIndex);

	float3 calculateNormal(int i, int j, int k);

	/// Table 4.1
	float calculateMassExchange(int currentIndex, int neighborIndex, float currentDf, float inverse_NbFi);

	// Exchange of mass to adjecent cells
	void adjustCellMass(int index);

	void setFilledOrEmpty(int i, int j, int k);

	// Change the cell type in order to ensure the layer of interface cells has to be closed again, once the filled and emptied interface
	//cells have been converted into their respective types
	void cellTypeAdjustment();

	void setNeighborhoodFlags();

	void averageSurroundings(int i, int j, int k);

public:

	unsigned int	*solid;
	float3			*velocityVector;
	int				*cellType;

	// This method has to be called after all the cells have a defined type
	void calculateInitialMass();
	
	void calculateInEquilibriumFunction(int index, float3 inVector, float inRo);

	latticed3q19(int width, int height, int depth, float worldViscosity, float mass, float cellsPerSide, float domainSize);
	~latticed3q19();

	void step(void);

	void printLattice(void);
	void printLatticeElement(int i, int j, int k);
	int getNumElements(void) 
	{ 
		return _numberLatticeElements; 
	}
};
#endif