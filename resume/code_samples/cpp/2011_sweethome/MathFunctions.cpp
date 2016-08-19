#include <windows.h>

#include "Utils/MathFuncts.h"

const FLOAT g_eps = 0.0001f;

bool Intersection( FLOAT ax1 , FLOAT ay1, FLOAT ax2, FLOAT ay2, FLOAT bx1, FLOAT by1, FLOAT bx2, FLOAT by2 );


bool IsCrossing( FLOAT dblX11, FLOAT dblY11, FLOAT dblX12, FLOAT dblY12
					, FLOAT dblX21, FLOAT dblY21, FLOAT dblX22, FLOAT dblY22 )
{
	return Intersection( dblX11, dblY11, dblX12, dblY12, dblX21, dblY21, dblX22, dblY22 );
//	FLOAT a = 0.0f, b = 0.0f;
//	return FindCrossing(dblX11, dblY11, dblX12, dblY12, dblX21, dblY21, dblX22, dblY22, a, b);
}


bool Intersection( FLOAT ax1 , FLOAT ay1, FLOAT ax2, FLOAT ay2, FLOAT bx1, FLOAT by1, FLOAT bx2, FLOAT by2 )
{
	FLOAT v1 = (bx2-bx1)*(ay1-by1)-(by2-by1)*(ax1-bx1);
	FLOAT v2 = (bx2-bx1)*(ay2-by1)-(by2-by1)*(ax2-bx1);
	FLOAT v3 = (ax2-ax1)*(by1-ay1)-(ay2-ay1)*(bx1-ax1);
	FLOAT v4 = (ax2-ax1)*(by2-ay1)-(ay2-ay1)*(bx2-ax1);

	bool bRes = ( ( v1*v2 < 0.001f ) && ( v3*v4 < 0.001f ) );
	return bRes;
}


//The function uses the equation y = k*x + b to represent lines. Therefore
//at the beginning we check if any of the lines is vertical or not, because
//you cannot represent a vertical line with this equation, so it's a special case.
//If one of the lines is vertical and the other isn't they do intersect, and
//the only thing remaining is to check if the intersection point lies inside
//the given line segments.
//Otherwise they are either parallel or not, with both being not vertical.
//First we check the case of both being vertical. We accomplish that by checking
//whether the both segments belong to one line and then, if they do, get the
//intersection point using their Y coordinates. If they do intersect we return
//one of the top borders of segments, the one which lies inside the both segments.
//Then we check the case of both being parallel and not vertical using the same
//method as in previous case, but with X coordinates as Y will fail us in the
//case of horizontal lines.
//After that we have only one case remaining: lines are not parallel and not vertical.
//We get the intersection point and return true if it lies in the required segments.
bool FindCrossing( FLOAT dblX11, FLOAT dblY11, FLOAT dblX12, FLOAT dblY12
					, FLOAT dblX21, FLOAT dblY21, FLOAT dblX22, FLOAT dblY22 
					, FLOAT &dblCrossX, FLOAT &dblCrossY )
{
	dblCrossX = 0.0f;
	dblCrossY = 0.0f;
	//check vertical lines
	bool vert1 = abs(dblX11 - dblX12) < FLT_EPSILON;
	bool vert2 = abs(dblX21 - dblX22) < FLT_EPSILON;
	//coefficients for y = k*x + b equation
	FLOAT k1 = 0.0f, b1 = 0.0f, k2 = 0.0f, b2 = 0.0f;
	if(!vert1)
	{
		k1 = (dblY12 - dblY11) / (dblX12 - dblX11);
		b1 = dblY11 - k1 * dblX11;
	}
	if(!vert2)
	{
		k2 = (dblY22 - dblY21) / (dblX22 - dblX21);
		b2 = dblY21 - k2 * dblX21;
	}
	FLOAT X1min = dblX11, X1max = dblX12;
	if(dblX12 < dblX11)
	{
		X1min = dblX12;
		X1max = dblX11;
	}
	FLOAT X2min = dblX21, X2max = dblX22;
	if(dblX22 < dblX21)
	{
		X2min = dblX22;
		X2max = dblX21;
	}
	FLOAT Y1min = dblY11, Y1max = dblY12;
	if(dblY12 < dblY11 + g_eps)
	{
		Y1min = dblY12;
		Y1max = dblY11;
	}
	FLOAT Y2min = dblY21, Y2max = dblY22;
	if(dblY22 < dblY21 + g_eps)
	{
		Y2min = dblY22;
		Y2max = dblY21;
	}
	if(vert1 && !vert2)
	{
		dblCrossX = dblX11;
		dblCrossY = k2 * dblX11 + b2;
	}
	else if(!vert1 && vert2)
	{
		dblCrossX = dblX21;
		dblCrossY = k1 * dblX21 + b1;
	}
	else
	{
		//check if they are parallel
		if(abs(k2 - k1) < FLT_EPSILON)
		{
			//they are parallel, means they are either apart or have more than one intersection point
			//both vertical case
			if(vert1 && vert2)
			{
				//they are parallel and not on one line
				if(abs(dblX11 - dblX22) > FLT_EPSILON)
					return false;
				//if they don't intersect
				if(Y2max < Y1min - g_eps || Y2min > Y1max + g_eps)
					return false;
				else
				{
					dblCrossX = dblX11;
					dblCrossY = Y2max;
					if(Y1max < Y2max + g_eps && Y1max > Y2min - g_eps)
						dblCrossY = Y1max;
					return true;
				}
			}
			else if(abs(b2 - b1) < FLT_EPSILON)
			{
				//they are parallel and not vertical and their corresponding lines are the same as k1 == k2, b1 == b2
				//they do not intersect
				if(X2max < X1min - g_eps || X2min > X1max + g_eps)
					return false;
				dblCrossX = X2max;
				if(X1max < X2max + g_eps && X1max > X2min - g_eps)
					dblCrossX = X1max;
				dblCrossY = k1 * dblCrossX + b1;
				return true;
			}
			else return false;
		}
		//they are not parallel, let's find the intersection point
		dblCrossX = (b2 - b1) / (k1 - k2);
		dblCrossY = k1 * dblCrossX + b1;
	}
	return (((dblCrossX < X1max + g_eps && dblCrossX > X1min - g_eps) ||
		(abs(X1max - X1min) < FLT_EPSILON && abs(dblCrossX - X1max) < FLT_EPSILON))&&
		((dblCrossX < X2max + g_eps && dblCrossX > X2min - g_eps) ||
		(abs(X2max - X2min) < FLT_EPSILON && abs(dblCrossX - X2max) < FLT_EPSILON)) &&
		((dblCrossY < Y1max + g_eps && dblCrossY > Y1min - g_eps) ||
		(abs(Y1max - Y1min) < FLT_EPSILON && abs(dblCrossY - Y1max) < FLT_EPSILON))&&
		((dblCrossY < Y2max + g_eps && dblCrossY > Y2min - g_eps) ||
		(abs(Y2max - Y2min) < FLT_EPSILON && abs(dblCrossY - Y2max) < FLT_EPSILON)));
}

//Length of Ax^2 + Bx + C from the X1 to the X2
FLOAT GetParaLength(FLOAT dblA, FLOAT dblB, FLOAT dblC,
					FLOAT dblX1, FLOAT dblX2, int PrecisionParam)
{
	FLOAT res = 0.0f;
	if((dblA > 0.0f && dblA - FLT_EPSILON < 0.0f) || (dblA <= 0.0f && dblA + FLT_EPSILON > 0.0f)) //a = 0, wtf?
		return (dblX2 - dblX1) * sqrt(1.0f + dblB);
	FLOAT t = 2 * dblA * dblX2 + dblB;
	res += sqrt(t * t + 1) * t + log(t + sqrt(t * t + 1));
	t = 2 * dblA * dblX1 + dblB;
	res -= sqrt(t * t + 1) * t + log(t + sqrt(t * t + 1));
	res /= (4 * dblA);
	return res;
}

FLOAT CalculateAngle(FLOAT dblXFrom, FLOAT dblYFrom, FLOAT dblXTo, FLOAT dblYTo)
{
	FLOAT angle = 0.0f;
	//atan2f returns +- pi/2 in case of x = 0,
	//but it returns always 0 if y = 0, which is bad.
	if(abs(dblYFrom - dblYTo) < FLT_EPSILON) //same Y value
		angle = dblXTo > dblXFrom ? 0.0f : (FLOAT)M_PI;
	else
		angle = atan2f(dblYTo - dblYFrom, dblXTo - dblXFrom);
	return angle;
}

//points should be given in a (counter)clockwise order
//the quad is convex iff (BDxBA)*(BDxBC) < 0 &&
//(ACxAD)*(ACxAB) < 0
bool CheckQuadConvexity(POINT a, POINT b, POINT c, POINT d)
{
	LONG v0, v1, v2, u0, u1, u2, w0, w1, w2, r0, r1, r2;
	//BD
	v0 = d.x - b.x;
	v1 = d.y - b.y;
	v2 = 0;
	//BA
	u0 = a.x - b.x;
	u1 = a.y - b.y;
	u2 = 0;
	//BD x BA
	w0 = v1 * u2 - v2 * u1;
	w1 = v2 * u0 - v0 * u2;
	w2 = v0 * u1 - v1 * u0;
	//BC
	u0 = c.x - b.x;
	u1 = c.y - b.y;
	u2 = 0;
	//BD x BC
	r0 = v1 * u2 - v2 * u1;
	r1 = v2 * u0 - v0 * u2;
	r2 = v0 * u1 - v1 * u0;
	if(w0 * r0 + w1 * r1 + w2 * r2 >= 0)
		return false;
	//AC
	v0 = c.x - a.x;
	v1 = c.y - a.y;
	v2 = 0;
	//AD
	u0 = d.x - a.x;
	u1 = d.y - a.y;
	u2 = 0;
	//AC x AD
	w0 = v1 * u2 - v2 * u1;
	w1 = v2 * u0 - v0 * u2;
	w2 = v0 * u1 - v1 * u0;
	//AB
	u0 = b.x - a.x;
	u1 = b.y - a.y;
	u2 = 0;
	//AC x AB
	r0 = v1 * u2 - v2 * u1;
	r1 = v2 * u0 - v0 * u2;
	r2 = v0 * u1 - v1 * u0;
	if(w0 * r0 + w1 * r1 + w2 * r2 >= 0)
		return false;
	return true;
}

void GetMaxMinProjectionsQuad(FLOAT x, FLOAT y, FLOAT q0x, FLOAT q0y, FLOAT q1x, FLOAT q1y,
							  FLOAT q2x, FLOAT q2y, FLOAT q3x, FLOAT q3y, FLOAT &u0, FLOAT &u1)
{
	FLOAT len = sqrt(x * x + y * y);
	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	//but we don't need x, y coordinates therefore we don't normalize coordinate vectors
	//thus we divide by len instead of len^2 which allows us to add/substract plain radius
	//from the result

	u0 = (q0x * x + q0y * y);
	u1 = u0;
	FLOAT w = (q1x * x + q1y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (q2x * x + q2y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (q3x * x + q3y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
}

void GetMaxMinProjectionsSector(FLOAT x, FLOAT y, FLOAT cx, FLOAT cy, FLOAT r, FLOAT alpha, FLOAT beta,
								FLOAT &v0, FLOAT &v1)
{
	FLOAT len = sqrt(x * x + y * y);
	if(!len) //shouldn't happen as previous cases should cover this but just to be safe
		len = 1.0f;

	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	FLOAT w = (cx * x + cy * y); //center
	FLOAT angle, u0, u1;
	v0 = w;
	v1 = w;
	if(x)
		angle = atan2f(y, x);
	else angle = y > 0.0f ? (FLOAT)M_PI_2 : -(FLOAT)M_PI_2;
	while(angle > beta)
		angle -= (FLOAT)M_PI * 2;
	while(angle < alpha)
		angle += (FLOAT)M_PI * 2;
	if(angle < beta)
		v1 = w + r * len;
	angle += (FLOAT)M_PI;
	if(angle < beta && angle > alpha)
		v0 = w - r * len;
	else
	{
		angle -= 2 * (FLOAT)M_PI;
		if(angle < beta && angle > alpha)
			v0 = w - r * len;
	}
	u0 = (cx + r * cosf(alpha)) * x + (cy + r * sinf(alpha)) * y;
	u1 = (cx + r * cosf(beta)) * x + (cy + r * sinf(beta)) * y;
	if(v0 > u0)
		v0 = u0;
	if(v0 > u1)
		v0 = u1;
	if(v1 < u0)
		v1 = u0;
	if(v1 < u1)
		v1 = u1;
}

void GetClosestToCenter(FLOAT cx, FLOAT cy, FLOAT q0x, FLOAT q0y, FLOAT q1x, FLOAT q1y,
						FLOAT q2x, FLOAT q2y, FLOAT q3x, FLOAT q3y, FLOAT &x, FLOAT &y)
{
	//we won't take square root from len, as we don't need the value of the length, only
	//comparison of them. sqrt(len1) > sqrt(len2) <=> len1 > len2
	FLOAT len = (q0x - cx)*(q0x - cx) + (q0y - cy)*(q0y - cy);
	x = q0x;
	y = q0y;
	FLOAT t = (q1x - cx)*(q1x - cx) + (q1y - cy)*(q1y - cy);
	if(t < len)
	{
		len = t;
		x = q1x;
		y = q1y;
	}
	t = (q2x - cx)*(q2x - cx) + (q2y - cy)*(q2y - cy);
	if(t < len)
	{
		len = t;
		x = q2x;
		y = q2y;
	}
	t = (q3x - cx)*(q3x - cx) + (q3y - cy)*(q3y - cy);
	if(t < len)
	{
		len = t;
		x = q3x;
		y = q3y;
	}
}

//using the Separating Axis Theorem
//Quad vertices should be given in a (counter)clockwise order
//r0 is upper-left corner, r1 is bottom-right
bool CheckQuadRectIntersection(FLOAT ax, FLOAT ay, FLOAT bx, FLOAT by, FLOAT cx, FLOAT cy,
							   FLOAT dx, FLOAT dy, FLOAT r0x, FLOAT r0y, FLOAT r1x, FLOAT r1y)
{
	FLOAT v0, v1, u0, u1;
	//X
	v1 = ax;
	v0 = ax;
	if(bx > v1)
		v1 = bx;
	if(bx < v0)
		v0 = bx;
	if(cx > v1)
		v1 = cx;
	if(cx < v0)
		v0 = cx;
	if(dx > v1)
		v1 = dx;
	if(dx < v0)
		v0 = dx;
	u0 = r0x;
	u1 = r1x;
	if(u1 < v0 || u0 > v1)
		return false;
	//Y
	v1 = ay;
	v0 = ay;
	if(by > v1)
		v1 = by;
	if(by < v0)
		v0 = by;
	if(cy > v1)
		v1 = cy;
	if(cy < v0)
		v0 = cy;
	if(dy > v1)
		v1 = dy;
	if(dy < v0)
		v0 = dy;
	u0 = r0y;
	u1 = r1y;
	if(u1 < v0 || u0 > v1)
		return false;

	//now we need to project all vertices to normals to each of the quad's faces
	FLOAT ax0, ax1, n0, n1;
	FLOAT w0;
	for(int i = 0; i < 4; ++i)
	{
		switch(i)
		{
		case 0:
			ax0 = bx - ax;
			ax1 = by - ay;
			break;
		case 1:
			ax0 = cx - bx;
			ax1 = cy - by;
			break;
		case 2:
			ax0 = dx - cx;
			ax1 = dy - cy;
			break;
		case 3:
			ax0 = ax - dx;
			ax1 = ay - dy;
			break;
		}
		//it's normal
		n0 = -ax1;
		n1 = ax0;

		//projected vector should be n0/1 * (ax * n0 + ay * n1) / (n0 * n0 + n1 * n1)
		//but we need to _compare_ these values therefore we can omit the common denominator
		//also we don't need to multiply by n0/1 as the points will lie on the same line
		//and we don't need their coordinates. Only how far are they from zero on that line
		v0 = (ax * n0 + ay * n1);
		v1 = v0;
		w0 = (bx * n0 + by * n1);
		if(w0 < v0)
			v0 = w0;
		if(w0 > v1)
			v1 = w0;
		w0 = (cx * n0 + cy * n1);
		if(w0 < v0)
			v0 = w0;
		if(w0 > v1)
			v1 = w0;
		w0 = (dx * n0 + dy * n1);
		if(w0 < v0)
			v0 = w0;
		if(w0 > v1)
			v1 = w0;

		u0 = (r0x * n0 + r0y * n1);
		u1 = u0;
		w0 = (r0x * n0 + r1y * n1);
		if(w0 < u0)
			u0 = w0;
		if(w0 > u1)
			u1 = w0;
		w0 = (r1x * n0 + r1y * n1);
		if(w0 < u0)
			u0 = w0;
		if(w0 > u1)
			u1 = w0;
		w0 = (r1x * n0 + r0y * n1);
		if(w0 < u0)
			u0 = w0;
		if(w0 > u1)
			u1 = w0;

		if(u1 < v0 || u0 > v1)
			return false;
	}
	return true;
}
bool CheckQuadRectIntersection(POINT a, POINT b, POINT c, POINT d, POINT r0, POINT r1)
{
	return CheckQuadRectIntersection((FLOAT)a.x, (FLOAT)a.y, (FLOAT)b.x, (FLOAT)b.y,
		(FLOAT)c.x, (FLOAT)c.y, (FLOAT)d.x, (FLOAT)d.y, (FLOAT)r0.x, (FLOAT)r0.y,
		(FLOAT)r1.x, (FLOAT)r1.y);
}

//Using the Separating Axis Theorem again. We test axes perpendicular to rect's faces and
//the axis parallel to the vector from the center of the circle to the center of the rect
bool CheckCircleRectIntersection(FLOAT cx, FLOAT cy, FLOAT r, FLOAT r0x, FLOAT r0y,
								 FLOAT r1x, FLOAT r1y)
{
	FLOAT v0, v1, u0, u1;
	//X
	v1 = cx + r;
	v0 = cx - r;

	u0 = r0x;
	u1 = r1x;
	if(u1 < v0 || u0 > v1)
		return false;
	//Y
	v1 = cy + r;
	v0 = cy - r;

	u0 = r0y;
	u1 = r1y;
	if(u1 < v0 || u0 > v1)
		return false;

	//find the closest vertex
	FLOAT px, py;
	if(abs(cx - r0x) < abs(cx - r1x))
		px = r0x;
	else px = r1x;
	if(abs(cy - r0y) < abs(cy - r1y))
		py = r0y;
	else py = r1y;

	//get the vector c->p
	FLOAT x, y, len;
	x = px - cx;
	y = py - cy;

	//get perpendicular vector
	len = x;
	x = -y;
	y = len;
	len = sqrt(x * x + y * y);

	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	//but we don't need x, y coordinates therefore we don't normalize coordinate vectors
	//thus we divide by len instead of len^2 which allows us to add/substract plain radius
	//from the result
	v0 = (cx * x + cy * y) / len;
	v1 = v0 + r;
	v0 -= r;

	FLOAT w;
	u0 = (r0x * x + r0y * y) / len;
	u1 = u0;
	w = (r0x * x + r1y * y) / len;
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (r1x * x + r1y * y) / len;
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (r1x * x + r0y * y) / len;
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;

	if(u1 < v0 || u0 > v1)
		return false;
	return true;
}
bool CheckCircleRectIntersection(POINT c, LONG r, POINT r0, POINT r1)
{
	return CheckCircleRectIntersection((FLOAT)c.x, (FLOAT)c.y, (FLOAT)r,
		(FLOAT)r0.x, (FLOAT)r0.y, (FLOAT)r1.x, (FLOAT)r1.y);
}

bool CheckCircleQuadIntersection(FLOAT cx, FLOAT cy, FLOAT r, FLOAT q0x, FLOAT q0y, FLOAT q1x, FLOAT q1y,
								 FLOAT q2x, FLOAT q2y, FLOAT q3x, FLOAT q3y)
{
	FLOAT v0, v1, u0, u1;
	FLOAT x, y, len;
	//axes perpendicular to faces of the quad
	//q0 - q1
	x = q1x - q0x;
	y = q1y - q0y;
	len = x;
	x = -y;
	y = len;
	len = sqrt(x * x + y * y);
	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	//but we don't need x, y coordinates therefore we don't normalize coordinate vectors
	//thus we divide by len instead of len^2 which allows us to add/substract plain radius
	//from the result
	v0 = (cx * x + cy * y);
	v1 = v0 + r * len;
	v0 -= r * len;
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;
	//q1 - q2
	x = q2x - q1x;
	y = q2y - q1y;
	len = x;
	x = -y;
	y = len;
	len = sqrt(x * x + y * y);

	v0 = (cx * x + cy * y);
	v1 = v0 + r * len;
	v0 -= r * len;
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;
	//q2 - q3
	x = q3x - q2x;
	y = q3y - q2y;
	len = x;
	x = -y;
	y = len;
	len = sqrt(x * x + y * y);

	v0 = (cx * x + cy * y);
	v1 = v0 + r * len;
	v0 -= r * len;
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;
	//q3 - q0
	x = q0x - q3x;
	y = q0y - q3y;
	len = x;
	x = -y;
	y = len;
	len = sqrt(x * x + y * y);

	v0 = (cx * x + cy * y);
	v1 = v0 + r * len;
	v0 -= r * len;
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;

	//find the closest vertex
	FLOAT px, py;
	GetClosestToCenter(cx, cy, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, px, py);

	//get the vector c->p
	x = px - cx;
	y = py - cy;

	v0 = (cx * x + cy * y);
	v1 = v0 + r * len;
	v0 -= r * len;

	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);

	if(u1 < v0 || u0 > v1)
		return false;
	return true;
}

//c - center of the circle, rad - radius
//alpha and beta - arc angles, rn - rect vertices
//using SAT
bool CheckSectorRectIntersection(FLOAT cx, FLOAT cy, FLOAT r, FLOAT alpha, FLOAT beta,
								 FLOAT r0x, FLOAT r0y, FLOAT r1x, FLOAT r1y)
{
	FLOAT angle;
	if(beta - alpha > 2 * M_PI || abs(beta - alpha - 2 * M_PI) < FLT_EPSILON) //circle+
		return CheckCircleRectIntersection(cx, cy, r, r0x, r0y, r1x, r1y);
	else if(beta - alpha > M_PI + FLT_EPSILON) //half-circle+
	{
		angle = beta;
		while(angle > alpha || abs(angle - alpha) < FLT_EPSILON)
			angle -= (FLOAT)M_PI_2;
		return CheckCircleRectIntersection(cx, cy, r, r0x, r0y, r1x, r1y) &&
			!CheckSectorRectIntersection(cx, cy, r, angle, alpha, r0x, r0y, r1x, r1y);
	}
	FLOAT v0, v1, u0, u1;
	//to project the sector on X and Y we are going to get how many PI/2's are in [a, b]
	//so that we know whether we have cos/sin = 0/1 or not
	int minpi = 0, maxpi = 0, i = 0;
	v0 = alpha / (FLOAT)M_PI_2;
	minpi = (int)ceil(v0);
	v0 = beta / (FLOAT)M_PI_2;
	maxpi = (int)floor(v0);

	FLOAT cosa = cos(alpha);
	FLOAT cosb = cos(beta);
	FLOAT sina = sin(alpha);
	FLOAT sinb = sin(beta);
	//X
	v0 = 0.0f;
	v1 = 0.0f;
	u0 = cosa;
	if(u0 > v1)
		v1 = u0;
	if(u0 < v0)
		v0 = u0;
	u0 = cosb;
	if(u0 > v1)
		v1 = u0;
	if(u0 < v0)
		v0 = u0;
	for(i = minpi; i <= maxpi; ++i)
	{
		if(!(i % 4)) //2*pi*n, cos = 1
			v1 = 1;
		else if(!(i % 2)) //other cases of pi*n, cos = -1
			v0 = -1;
	}
	v1 = v1 * r + cx;
	v0 = v0 * r + cx;
	u0 = r0x;
	u1 = r1x;
	if(u1 < v0 || u0 > v1)
		return false;
	//Y
	v0 = 0.0f;
	v1 = 0.0f;
	u0 = sina;
	if(u0 > v1)
		v1 = u0;
	if(u0 < v0)
		v0 = u0;
	u0 = sinb;
	if(u0 > v1)
		v1 = u0;
	if(u0 < v0)
		v0 = u0;
	for(i = minpi; i <= maxpi; ++i)
	{
		if(!((i - 1) % 4)) //(4*n + 1) * pi/2, sin = 1
			v1 = 1;
		else if(i % 2) //other cases of odd i, sin = -1
			v0 = -1;
	}
	v1 = v1 * r + cy;
	v0 = v0 * r + cy;
	u0 = r0y;
	u1 = r1y;
	if(u1 < v0 || u0 > v1)
		return false;
	//axis perpendicular to (cos(alpha), sin(alpha))
	FLOAT x = 0.0f, y = 0.0f, len = 1.0f;
	x = -sina;
	y = cosa;
	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	//luckily, the vectors are already normalized yahay!
	v0 = cx * x + cy * y;  //center projection
	v1 = v0;
	if(beta - alpha >= M_PI_2)
		v1 += r;
	else 
		v1 += r * (cosb * x + sinb * y); //radius at angle beta

	FLOAT w = 0.0f;
	u0 = (r0x * x + r0y * y);
	u1 = u0;
	w = (r0x * x + r1y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (r1x * x + r1y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (r1x * x + r0y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;

	if(u1 < v0 || u0 > v1)
		return false;

	//axis perpendicular to (cos(beta), sin(beta))
	x = 0.0f, y = 0.0f, len = 1.0f;
	x = -sinb;
	y = cosb;
	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	//luckily, the vectors are already normalized yahay!
	v1 = cx * x + cy * y;
	v0 = v1;
	if(beta - alpha >= M_PI_2)
		v0 -= r;
	else 
		v0 += r * (cosa * x + sina * y); //radius at angle alpha

	u0 = (r0x * x + r0y * y);
	u1 = u0;
	w = (r0x * x + r1y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (r1x * x + r1y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (r1x * x + r0y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;

	if(u1 < v0 || u0 > v1)
		return false;

	//find the closest vertex
	FLOAT px, py;
	if(abs(cx - r0x) < abs(cx - r1x))
		px = r0x;
	else px = r1x;
	if(abs(cy - r0y) < abs(cy - r1y))
		py = r0y;
	else py = r1y;

	//get the vector c->p
	x = px - cx;
	y = py - cy;

	len = sqrt(x * x + y * y);
	if(!len) //shouldn't happen as previous cases should cover this but just to be safe
		len = 1.0f;

	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	w = cx * x + cy * y; //center
	v0 = w;
	v1 = w;
	if(x)
		angle = atan2f(y, x);
	else angle = y > 0.0f ? (FLOAT)M_PI_2 : -(FLOAT)M_PI_2;
	while(angle > beta)
		angle -= (FLOAT)M_PI * 2;
	while(angle < alpha)
		angle += (FLOAT)M_PI * 2;
	if(angle < beta)
		v1 = w + r * len;
	angle += (FLOAT)M_PI;
	if(angle < beta && angle > alpha)
		v0 = w - r * len;
	else
	{
		angle -= 2 * (FLOAT)M_PI;
		if(angle < beta && angle > alpha)
			v0 = w - r * len;
	}
	u0 = (cx + r * cosa) * x + (cy + r * sina) * y;
	u1 = (cx + r * cosb) * x + (cy + r * sinb) * y;
	if(v0 > u0)
		v0 = u0;
	if(v0 > u1)
		v0 = u1;
	if(v1 < u0)
		v1 = u0;
	if(v1 < u1)
		v1 = u1;

	u0 = (r0x * x + r0y * y);
	u1 = u0;
	w = (r0x * x + r1y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (r1x * x + r1y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (r1x * x + r0y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;

	if(u1 < v0 || u0 > v1)
		return false;
	return true;
}
bool CheckSectorRectIntersection(POINT c, LONG r, FLOAT alpha, FLOAT beta, POINT r0, POINT r1)
{
	return CheckSectorRectIntersection((FLOAT)c.x, (FLOAT)c.y, (FLOAT)r, alpha, beta,
		(FLOAT)r0.x, (FLOAT)r0.y, (FLOAT)r1.x, (FLOAT)r1.y);
}

bool CheckSectorQuadIntersection(FLOAT cx, FLOAT cy, FLOAT r, FLOAT alpha, FLOAT beta,
								 FLOAT q0x, FLOAT q0y, FLOAT q1x, FLOAT q1y, FLOAT q2x, FLOAT q2y, FLOAT q3x, FLOAT q3y)
{
	FLOAT angle;
	if(beta - alpha > 2 * M_PI || abs(beta - alpha - 2 * M_PI) < FLT_EPSILON) //circle+
		return CheckCircleQuadIntersection(cx, cy, r, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y);
	else if(beta - alpha > M_PI + FLT_EPSILON) //half-circle+
	{
		angle = beta;
		while(angle > alpha || abs(angle - alpha) < FLT_EPSILON)
			angle -= (FLOAT)M_PI_2;
		return CheckCircleQuadIntersection(cx, cy, r, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y) &&
			!CheckSectorQuadIntersection(cx, cy, r, angle, alpha, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y);
	}
	FLOAT v0, v1, u0, u1;
	FLOAT cosa = cos(alpha);
	FLOAT cosb = cos(beta);
	FLOAT sina = sin(alpha);
	FLOAT sinb = sin(beta);

	FLOAT x = 0.0f, y = 0.0f;

	//q0-q1
	x = q1x - q0x;
	y = q1y - q0y;
	v0 = x;
	x = -y;
	y = v0;
	GetMaxMinProjectionsSector(x, y, cx, cy, r, alpha, beta, v0, v1);
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;

	//q1-q2
	x = q2x - q1x;
	y = q2y - q1y;
	v0 = x;
	x = -y;
	y = v0;
	GetMaxMinProjectionsSector(x, y, cx, cy, r, alpha, beta, v0, v1);
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;

	//q2-q3
	x = q3x - q2x;
	y = q3y - q2y;
	v0 = x;
	x = -y;
	y = v0;
	GetMaxMinProjectionsSector(x, y, cx, cy, r, alpha, beta, v0, v1);
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;

	//q3-q0
	x = q0x - q3x;
	y = q0y - q3y;
	v0 = x;
	x = -y;
	y = v0;
	GetMaxMinProjectionsSector(x, y, cx, cy, r, alpha, beta, v0, v1);
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;

	//axis perpendicular to (cos(alpha), sin(alpha))
	x = -sina;
	y = cosa;
	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	//luckily, the vectors are already normalized yahay!
	v0 = cx * x + cy * y;  //center projection
	v1 = v0;
	if(beta - alpha >= M_PI_2)
		v1 += r;
	else 
		v1 += r * (cosb * x + sinb * y); //radius at angle beta
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;

	//axis perpendicular to (cos(beta), sin(beta))
	x = -sinb;
	y = cosb;
	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	//luckily, the vectors are already normalized yahay!
	v1 = cx * x + cy * y;
	v0 = v1;
	if(beta - alpha >= M_PI_2)
		v0 -= r;
	else 
		v0 += r * (cosa * x + sina * y); //radius at angle alpha
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;

	//find the closest vertex
	FLOAT px, py;
	GetClosestToCenter(cx, cy, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, px, py);

	//get the vector c->p
	x = px - cx;
	y = py - cy;

	GetMaxMinProjectionsSector(x, y, cx, cy, r, alpha, beta, v0, v1);
	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);
	if(u1 < v0 || u0 > v1)
		return false;
	return true;
}

//c - center of the circle, radn - inner/outer radius
//alpha, beta - arc angles, rn - rect vertices
//The Arc here is a part of a sector cut off with a straight line parallel to the line connecting arc's points
bool CheckArcRectIntersection(FLOAT cx, FLOAT cy, FLOAT innerR, FLOAT outerR, FLOAT alpha, FLOAT beta,
							  FLOAT r0x, FLOAT r0y, FLOAT r1x, FLOAT r1y)
{
	if(!CheckSectorRectIntersection(cx, cy, outerR, alpha, beta, r0x, r0y, r1x, r1y))
		return false;
	FLOAT x, y, bx, by;
	x = cos(alpha) * innerR;
	y = sin(alpha) * innerR;
	bx = cos(beta) * innerR;
	by = sin(beta) * innerR;
	bx -= x;
	by -= y;

	//required axis
	x = -by;
	y = bx;

	FLOAT len = sqrt(x * x + y * y);
	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	FLOAT v0, v1, u0, u1;
	FLOAT w = cx * x + cy * y; //center
	v1 = (cx + innerR * cos(alpha)) * x + (cy + innerR * sin(alpha)) * y; //one of inner arc points
	v0 = cx * x + cy * y - outerR * len; //the max projection is always at distance R from center

	u0 = (r0x * x + r0y * y);
	u1 = u0;
	w = (r0x * x + r1y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (r1x * x + r1y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;
	w = (r1x * x + r0y * y);
	if(w < u0)
		u0 = w;
	if(w > u1)
		u1 = w;

	if(u1 < v0 || u0 > v1)
		return false;
	return true;
}
bool CheckArcRectIntersection(POINT c, FLOAT innerR, FLOAT outerR, FLOAT alpha, FLOAT beta, POINT r0, POINT r1)
{
	return CheckArcRectIntersection((FLOAT)c.x, (FLOAT)c.y, innerR, outerR, alpha, beta,
		(FLOAT)r0.x, (FLOAT)r0.y, (FLOAT)r1.x, (FLOAT)r1.y);
}

bool CheckArcQuadIntersection(FLOAT cx, FLOAT cy, FLOAT innerR, FLOAT outerR, FLOAT alpha, FLOAT beta,
							  FLOAT q0x, FLOAT q0y, FLOAT q1x, FLOAT q1y, FLOAT q2x, FLOAT q2y, FLOAT q3x, FLOAT q3y)
{
	if(!CheckSectorQuadIntersection(cx, cy, outerR, alpha, beta, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y))
		return false;
	FLOAT x, y, bx, by;
	x = cos(alpha) * innerR;
	y = sin(alpha) * innerR;
	bx = cos(beta) * innerR;
	by = sin(beta) * innerR;
	bx -= x;
	by -= y;

	//required axis
	x = -by;
	y = bx;

	FLOAT len = sqrt(x * x + y * y);
	//projected vector for point p is x/y * ((p.x * x + p.y * y) / (x * x + y * y))
	FLOAT v0, v1, u0, u1;
	FLOAT w = (cx * x + cy * y); //center
	v1 = ((cx + innerR * cos(alpha)) * x + (cy + innerR * sin(alpha)) * y); //one of inner arc points
	v0 = w - outerR * len; //the max projection is always at distance R from center

	GetMaxMinProjectionsQuad(x, y, q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y, u0, u1);

	if(u1 < v0 || u0 > v1)
		return false;
	return true;
}