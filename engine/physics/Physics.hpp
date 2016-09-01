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
		float3 center;
		float3 size;
	};
	struct Sphere
	{
		float3 pos;
		float radius;
	};
	struct Cilinder
	{
		float3 pos;
		float3 dir;
		float radius;
	};
	struct Cone
	{
		float3 pos;
		float3 dir;
		float radius;
	};
	struct Pyramid
	{
		float3 pos;
		float3 dir;
		float2 size;
	};
	struct Plane
	{
		float3 pos;
		float3 dir;
	};
	struct OBB
	{
		//where it just 1x1x1 box in the center at ( 0 , 0 , 0 )
		f4x4 to_local_space;
	};
	struct Triangle
	{
		float3 p0 , p1 , p2;
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
		float3 pos;
		qf orientation;
		qf angle_vel;
		float3 vel;
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
		float3 pos;
		float3 norm;
		float3 tang;
		float3 binorm;
		float depth;
	};
}