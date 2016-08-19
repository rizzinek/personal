#include <windows.h>
#include <cfloat>

#define _USE_MATH_DEFINES
#include <cmath>


// ****** lines crossings ****** 
// == IsCrossing == 
// gets the point of line's sections crossing 
// or tells that they are not crossing 
//  [in] line 1 : (dblX11,dblY11) to (dblX12,dblY12)
//  [in] line 2 : (dblX21,dblY21) to (dblX22,dblY22)
// return value : true if crosses , false if not 
// 
bool IsCrossing( FLOAT dblX11, FLOAT dblY11, FLOAT dblX12, FLOAT dblY12
					, FLOAT dblX21, FLOAT dblY21, FLOAT dblX22, FLOAT dblY22 );
// 
// == FindCrossing == 
//  same as IsCrossing plus
//  [out] point of crossing (dblCrossX,dblCrossY) if line's sections have crossing 
//				they must be set to the x,y values of crossing point 
//
bool FindCrossing( FLOAT dblX11, FLOAT dblY11, FLOAT dblX12, FLOAT dblY12
					, FLOAT dblX21, FLOAT dblY21, FLOAT dblX22, FLOAT dblY22 
					, FLOAT &dblCrossX, FLOAT &dblCrossY );



//Misc functions
FLOAT GetParaLength(FLOAT dblA, FLOAT dblB, FLOAT dblC,
                    FLOAT dblX1, FLOAT dblX2, int PrecisionParam);


FLOAT CalculateAngle(FLOAT dblXFrom, FLOAT dblYFrom, FLOAT dblXTo, FLOAT dblYTo);



bool CheckQuadConvexity(POINT a, POINT b, POINT c, POINT d);
//x, y - axis, q* - quad coordinates, u* - output
void GetMaxMinProjectionsQuad(FLOAT x, FLOAT y, FLOAT q0x, FLOAT q0y, FLOAT q1x, FLOAT q1y,
							  FLOAT q2x, FLOAT q2y, FLOAT q3x, FLOAT q3y, FLOAT &u0, FLOAT &u1);
//x, y - axis, then goes the sector info, v* - output
void GetMaxMinProjectionsSector(FLOAT x, FLOAT y, FLOAT cx, FLOAT cy, FLOAT r, FLOAT alpha, FLOAT beta,
								FLOAT &v0, FLOAT &v1);
//cx, cy - point, distance to which is measured, q* - quad, x & y - output
void GetClosestToCenter(FLOAT cx, FLOAT cy, FLOAT q0x, FLOAT q0y, FLOAT q1x, FLOAT q1y,
						FLOAT q2x, FLOAT q2y, FLOAT q3x, FLOAT q3y, FLOAT &x, FLOAT &y);


//Intersection checkers
bool CheckQuadRectIntersection(FLOAT ax, FLOAT ay, FLOAT bx, FLOAT by, FLOAT cx, FLOAT cy,
                               FLOAT dx, FLOAT dy, FLOAT r0x, FLOAT r0y, FLOAT r1x, FLOAT r1y);
bool CheckQuadRectIntersection(POINT a, POINT b, POINT c, POINT d, POINT r0, POINT r1);
bool CheckCircleRectIntersection(FLOAT cx, FLOAT cy, FLOAT r, FLOAT r0x, FLOAT r0y,
                                 FLOAT r1x, FLOAT r1y);
bool CheckCircleRectIntersection(POINT c, LONG r, POINT r0, POINT r1);
bool CheckCircleQuadIntersection(FLOAT cx, FLOAT cy, FLOAT r, FLOAT q0x, FLOAT q0y, FLOAT q1x, FLOAT q1y,
								 FLOAT q2x, FLOAT q2y, FLOAT q3x, FLOAT q3y);
bool CheckSectorRectIntersection(FLOAT cx, FLOAT cy, FLOAT r, FLOAT alpha, FLOAT beta,
                                 FLOAT r0x, FLOAT r0y, FLOAT r1x, FLOAT r1y);
bool CheckSectorRectIntersection(POINT c, LONG r, FLOAT alpha, FLOAT beta, POINT r0, POINT r1);
bool CheckSectorQuadIntersection(FLOAT cx, FLOAT cy, FLOAT r, FLOAT alpha, FLOAT beta,
								 FLOAT q0x, FLOAT q0y, FLOAT q1x, FLOAT q1y, FLOAT q2x, FLOAT q2y, FLOAT q3x, FLOAT q3y);
bool CheckArcRectIntersection(FLOAT cx, FLOAT cy, FLOAT innerR, FLOAT outerR, FLOAT alpha, FLOAT beta,
                              FLOAT r0x, FLOAT r0y, FLOAT r1x, FLOAT r1y);
bool CheckArcRectIntersection(POINT c, FLOAT innerR, FLOAT outerR, FLOAT alpha, FLOAT beta, POINT r0, POINT r1);
bool CheckArcQuadIntersection(FLOAT cx, FLOAT cy, FLOAT innerR, FLOAT outerR, FLOAT alpha, FLOAT beta,
                              FLOAT q0x, FLOAT q0y, FLOAT q1x, FLOAT q1y, FLOAT q2x, FLOAT q2y, FLOAT q3x, FLOAT q3y);
