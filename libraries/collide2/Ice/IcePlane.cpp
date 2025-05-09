///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for planes.
 *  \file		IcePlane.cpp
 *  \author		Pierre Terdiman
 *  \date		April, 4, 2000
 *
 *  Copyright (C) 1998-2025 Pierre Terdiman, Stephen G. Tuggy, Benjamen R. Meyer
 *  Public Domain
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Plane class.
 *  \class		Plane
 *  \author		Pierre Terdiman
 *  \version	1.0
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "collide2/Opcode.h"

using namespace Opcode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the plane equation from 3 points.
 *  \param		p0	[in] first point
 *  \param		p1	[in] second point
 *  \param		p2	[in] third point
 *  \return		Self-reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Plane &Plane::Set(const Point &p0, const Point &p1, const Point &p2) {
    Point Edge0 = p1 - p0;
    Point Edge1 = p2 - p0;

    n = Edge0 ^ Edge1;
    n.Normalize();

    d = -(p0 | n);

    return *this;
}

