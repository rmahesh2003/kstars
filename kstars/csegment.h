/***************************************************************************
                          csegment.h  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Feb 1 2004
    copyright            : (C) 2004 by Jason Harris
    email                : kstars@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CSEGMENT_H
#define CSEGMENT_H

class QString;
class SkyPoint;

/**@class CSegment
	*A segment of a constellation boundary.  The segment consists
	*of two or more SkyPoint "nodes" which are vertices of the 
	*boundary polygon.  A single segment is define as the set of nodes
	*that separates a single pair of constellations.  An entire 
	*constellation boundary must consist of many segments, because 
	*each constellation is surrounded by multiple neighbors.  
	*
	*For example, imagine constellation A is surrounded by constellations
	*B, C, and D.  One CSegment (AB) will describe the boundary between
	*A and B; another (AC) will describe the boundary between A and C;
	*and a third (AD) will describe the boundary between A and D. 
	*/
class CSegment {
public:
	/**
	  *Constructor
		*/
	CSegment();
	/**
	  *Destructor (empty)
		*/
	~CSegment();
	
	/**
	  *Add a SkyPoint node to the boundary segment.
		*@param ra the RA of the node
		*@param dec the Dec of the node
		*/
	void addPoint( double ra, double dec );
	
	/**
	  *@return the name of one of the constellations
		*that borders this boundary segment.
		*/
	QString name1() const { return Name1; }
	
	/**
	  *@return the name of one of the constellations
		*that borders this boundary segment.
		*/
	QString name2() const { return Name2; }
	
	/**
	  *Set the names of the bounding constellations.  Use the IAU
		*three-letter abbreviations.
		*@param n1 IAU name of one bounding constellation
		*@param n2 IAU name of the other bounding constellation
		*/
	bool setNames( const QString &n1, const QString &n2 );
	
	/**
	  *Determine if a given constellation borders this boundary segment
		*@param cname the IAU code of the constellation to be tested.
		*/
	bool borders( const QString &cname );

	/**
	  *@return reference to the list of nodes
		*/
	QList<SkyPoint*>& nodes() { return Nodes; }

private:
	QList<SkyPoint*> Nodes;
	QString Name1, Name2;
};

#endif
