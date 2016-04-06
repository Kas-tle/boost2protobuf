/*
 * R_GatingSet.cpp
 *
 *these are R APIs
 *
 *  Created on: Mar 30, 2012
 *      Author: wjiang2
 */
#include "include/GatingSet.hpp"
/*
 * save/load GatingSet
 */
//[[Rcpp::export(name=".cpp_saveGatingSet")]]
void saveGatingSet(XPtr<GatingSet> gs
                , string fileName
                , unsigned short format, bool isPB) {
			gs->serialize_pb(fileName);

}


//[[Rcpp::export(name=".cpp_loadGatingSet")]]
XPtr<GatingSet> loadGatingSet(string fileName, unsigned short format, bool isPB) {

	
		GatingSet * gs=new GatingSet(fileName, format,isPB);
		return XPtr<GatingSet>(gs);

}

