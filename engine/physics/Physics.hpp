#pragma once
#include <engine/math/mat.hpp>
#include <engine/math/Quaternion.hpp>
#include <engine/math/vec.hpp>
#include <engine/math/RandomFactory.hpp>
#include <engine/math/Math.hpp>
namespace Physics
{
	using namespace Math;
	enum class PhysycsShapeType
	{
		AABB , SPHERE , CILINDER , CONE , PYRAMID , CONVEX_POLYMESH , PLANE , OBB
	};
	struct AABB
	{
		f3 center;
		f3 size;
	};
	struct Sphere
	{
		f3 pos;
		float radius;
	};
	struct Cilinder
	{
		f3 pos;
		f3 dir;
		float radius;
	};
	struct Cone
	{
		f3 pos;
		f3 dir;
		float radius;
	};
	struct Pyramid
	{
		f3 pos;
		f3 dir;
		f2 size;
	};
	struct Plane
	{
		f3 pos;
		f3 dir;
	};
	struct OBB
	{
		//where it just 1x1x1 box in the center at ( 0 , 0 , 0 )
		f4x4 to_local_space;
	};
	struct Triangle
	{
		f3 p0 , p1 , p2;
	};
	struct ConvexMesh
	{
		Array< Triangle > triangles;
	};
	struct PysicsBodyProperties
	{
		float invmass;
		float invmomentum;
	};
	struct PysicsBodyState
	{
		f3 pos;
		qf orientation;
		qf angle_vel;
		f3 vel;
	};
	struct PhysicsShape
	{
		PhysycsShapeType type;
		union
		{
			AABB aabb;
			Sphere sphere;
			Cilinder cilinder;
			Cone cone;
			Pyramid pyramid;
			Plane plane;
			OBB obb;
			ConvexMesh convex_mesh;
		};
		~PhysicsShape()
		{

		}
	};
	struct Collision
	{
		f3 pos;
		f3 norm;
		f3 tang;
		f3 binorm;
		float depth;
	};
}