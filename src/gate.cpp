/*
 * gate.cpp
 *
 *  Created on: Apr 14, 2012
 *      Author: mike
 */



#include "include/gate.hpp"
#include <algorithm>
#include <valarray>


/*
* init the bool values in constructors
*/
gate::gate():neg(false),isTransformed(false),isGained(false){};
void gate::convertToPb(pb::gate & gate_pb){
	//cp basic members
		gate_pb.set_istransformed(isTransformed);
		gate_pb.set_neg(neg);
		gate_pb.set_isgained(isGained);
}
rangeGate::rangeGate():gate(){}
void rangeGate::convertToPb(pb::gate & gate_pb){
	gate::convertToPb(gate_pb);
	gate_pb.set_type(pb::RANGE_GATE);
	//cp nested gate
	pb::rangeGate * g_pb = gate_pb.mutable_rg();
	//cp its unique member
	pb::paramRange * pr_pb = g_pb->mutable_param();
	param.convertToPb(*pr_pb);
}

void polygonGate::convertToPb(pb::gate & gate_pb){
	gate::convertToPb(gate_pb);

	gate_pb.set_type(pb::POLYGON_GATE);
	//cp nested gate
	pb::polygonGate * g_pb = gate_pb.mutable_pg();
	//cp its unique member
	pb::paramPoly * pr_pb = g_pb->mutable_param();
	param.convertToPb(*pr_pb);
}
void boolGate::convertToPb(pb::gate & gate_pb){
	gate::convertToPb(gate_pb);

	gate_pb.set_type(pb::BOOL_GATE);
	//cp nested gate
	pb::boolGate * g_pb = gate_pb.mutable_bg();
	//cp its unique member
	for(unsigned i = 0; i < boolOpSpec.size(); i++){
		pb::BOOL_GATE_OP * gop_pb = g_pb->add_boolopspec();
		boolOpSpec.at(i).convertToPb(*gop_pb);
	}

}

void ellipseGate::convertToPb(pb::gate & gate_pb){
	polygonGate::convertToPb(gate_pb);

	gate_pb.set_type(pb::ELLIPSE_GATE);
	//cp nested gate
	pb::ellipseGate * g_pb = gate_pb.mutable_eg();
	//cp its unique member
	g_pb->set_dist(dist);
	pb::coordinate * coor_pb = g_pb->mutable_mu();
	mu.convertToPb(*coor_pb);
	for(unsigned i = 0; i < cov.size(); i++){
		pb::coordinate * coor_pb = g_pb->add_cov();
		cov.at(i).convertToPb(*coor_pb);
	}
	for(unsigned i = 0; i < antipodal_vertices.size(); i++){
		pb::coordinate * coor_pb = g_pb->add_antipodal_vertices();
		antipodal_vertices.at(i).convertToPb(*coor_pb);
	}
}

void ellipsoidGate::convertToPb(pb::gate & gate_pb){
	ellipseGate::convertToPb(gate_pb);
	gate_pb.set_type(pb::ELLIPSOID_GATE);
}
void rectGate::convertToPb(pb::gate & gate_pb){
	polygonGate::convertToPb(gate_pb);
	gate_pb.set_type(pb::RECT_GATE);
}

void logicalGate::convertToPb(pb::gate & gate_pb){
	boolGate::convertToPb(gate_pb);
	gate_pb.set_type(pb::LOGICAL_GATE);
}
polygonGate::polygonGate():gate(){};
boolGate::boolGate():gate(){};

ellipseGate::ellipseGate(coordinate _mu, vector<coordinate> _cov, double _dist):mu(_mu),cov(_cov), dist(_dist){
	isTransformed = true;
	isGained = true;
	neg = false;
}

ellipseGate::ellipseGate(vector<coordinate> _antipodal, vector<string> _params):antipodal_vertices(_antipodal),dist(1){
	isTransformed = false;
	isGained = false;
	neg = false;

	/*
	 * init the dummy vertices for base class
	 * (this deprecated inheritance exists for the sake of legacy archive)
	 */
	param.setName(_params);

}

/*
 * covert antipodal points to covariance matrix and mean
 * antipodal points must be transformed first.
 */
void ellipseGate::computeCov(){
	if(!Transformed())
		throw(domain_error("antipodal points of ellipseGate must be transformed before computing covariance matrix!"));

	vector<coordinate> v=antipodal_vertices;
	unsigned short nSize = v.size();
	if (nSize != 4)
		throw(domain_error("invalid number of antipodal points"));

	/*
	 * get center and set mu
	 */
	mu.x=0;
	mu.y=0;
	for(vector<coordinate>::iterator it=v.begin();it!=v.end();it++)
	{
		mu.x+=it->x;
		mu.y+=it->y;
	}
	mu.x=mu.x/nSize;
	mu.y=mu.y/nSize;

	//center the antipods
	for(vector<coordinate>::iterator it=v.begin();it!=v.end();it++)
	{
		it->x = it->x - mu.x;
		it->y = it->y - mu.y;
	}

	/*
	 * find the four positions of four antipodals
	 */

	//far right point
	vector<coordinate>::iterator R_it=max_element(v.begin(),v.end(),compare_x);
	coordinate R = *R_it;

	//far left point
	vector<coordinate>::iterator L_it=min_element(v.begin(),v.end(),compare_x);
	coordinate L = *L_it;

	// calculate the a length
	double a = hypot(L.x-R.x,L.y-R.y)/2;

	//use the rest of two points for computing b
	vector<coordinate> Q;
	for(vector<coordinate>::iterator it = v.begin();it!= v.end();it++){
		if(it != R_it && it != L_it)
			Q.push_back(*it);
	}
	coordinate V1 = Q.at(0);
	coordinate V2 = Q.at(1);
	double b = hypot(V1.x-V2.x,V1.y-V2.y)/2;

	double a2 = a * a ;
	double b2 = b * b ;


	//normailize R and V1 first
	double L_norm = hypot(L.x, L.y);
	double x1 = L.x/L_norm;
	double y1 = L.y/L_norm;

	double V1_norm = hypot(V1.x, V1.y);
	double x2 = V1.x/V1_norm;
	double y2 = V1.y/V1_norm;

	coordinate p1;
	p1.x = x1 * x1 * a2 + x2 * x2 * b2;
	p1.y = x1 * y1 * a2 + x2 * y2 * b2;

	coordinate p2;
	p2.x = p1.y;
	p2.y = y1 * y1 * a2 + y2 * y2 * b2;


	//set cov
	cov.push_back(p1);
	cov.push_back(p2);

	//set distance (in this calculation should always be 1)
	dist = 1;
}

/*
 * interpolation has to be done on the transformed original 4 coordinates
 * otherwise, the interpolation results will be wrong
 */
void ellipseGate::toPolygon(unsigned nVertices){




	/*
	 * using 4 vertices to fit polygon points
	 */
	vector<coordinate> v=antipodal_vertices;
	vector<coordinate> vertices=param.getVertices();
	vertices.clear();//reset the vertices

	unsigned nSize=v.size();
	/*
	 * scaling and centering the points
	 */
	coordinate mu;
	mu.x=0;
	mu.y=0;
	for(vector<coordinate>::iterator it=v.begin();it!=v.end();it++)
	{
		mu.x+=it->x;
		mu.y+=it->y;
	}
	mu.x=mu.x/nSize;
	mu.y=mu.y/nSize;

	coordinate sd;
	sd.x=0;
	sd.y=0;
	for(vector<coordinate>::iterator it=v.begin();it!=v.end();it++)
	{
		sd.x+=pow((it->x-mu.x),2);
		sd.y+=pow((it->y-mu.y),2);
	}
	sd.x=sqrt(sd.x/nSize);
	sd.y=sqrt(sd.y/nSize);

	for(vector<coordinate>::iterator it=v.begin();it!=v.end();it++)
	{
		it->x=(it->x-mu.x)/sd.x;
		it->y=(it->y-mu.y)/sd.y;
	}

	/*
	 * find the right positions of four antipodals
	 */
	coordinate R=*max_element(v.begin(),v.end(),compare_x);
	coordinate L=*min_element(v.begin(),v.end(),compare_x);

	coordinate T=*max_element(v.begin(),v.end(),compare_y);
	coordinate B=*min_element(v.begin(),v.end(),compare_y);

	/*
	 * calculate the a,b length
	 */
	coordinate E;
	E.x=hypot(L.x-R.x,L.y-R.y)/2;
	E.y=hypot(T.x-B.x,T.y-B.y)/2;

	/*
	 * calculate the rotation angle
	 */
	double phi=tan((R.y-L.y)/(R.x-L.x));
	double CY=(B.y+T.y)/2;
	double CX=(R.x+L.x)/2;

	double delta=2*PI/nVertices;
	/*
	 * fit the polygon points
	 */
	for(unsigned short i=0;i<nVertices;i++)
	{
		double S=i*delta;
		coordinate p;
		p.x=CX+E.x*cos(S)*cos(phi)-E.y*sin(S)*sin(phi);
		p.y=CY+E.x*cos(S)*sin(phi)+E.y*sin(S)*cos(phi);


		/*
		 * scale back
		 */
		p.x=p.x*sd.x+mu.x;
		p.y=p.y*sd.y+mu.y;

		vertices.push_back(p);
	}

	param.setVertices(vertices);

}

ellipsoidGate::ellipsoidGate(vector<coordinate> _antipodal, vector<string> _params):ellipseGate(_antipodal,_params)
{
	/*
	 * interpolate to polygon gate
	 */

	toPolygon(100);
}
/*
 *
 * we moved the interpolation to polygonGate form gating method to here because
 * gating may not be called when only gates to be extracted
 *
 *
 * ellipsoidGate does not follow the regular transforming process
 * for historical reason, it is defined in 256 * 256 scale.
 * For linear channel, we simply linear scale it back to raw scale
 * For non-linear channel, We need to first inverse transform it back to raw scale
 * before transforming to the ultimate appropriate data scale.
 */
vertices_valarray paramPoly::toValarray(){

	vertices_valarray res;
	unsigned nSize=vertices.size();
	res.resize(nSize);
	for(unsigned i=0;i<nSize;i++)
	{
		res.x[i]=vertices.at(i).x;
		res.y[i]=vertices.at(i).y;
	}
	return res;
}


vertices_valarray paramRange::toValarray(){

	vertices_valarray res;
	res.resize(2);
	res.x[0]=min;
	res.x[1]=max;

	return res;
}


