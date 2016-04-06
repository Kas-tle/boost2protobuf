/*
 * workspace.cpp
 *
 *  Created on: Mar 22, 2012
 *      Author: wjiang2
 */




#include "include/workspace.hpp"
#include <iostream>
#include <sstream>
workspace::~workspace(){

//	COUT<<"entring the destructor of workspace"<<endl;

	 /*free the document */
	if(doc!=NULL)
	{
		xmlFreeDoc(doc);
		doc = NULL;

		/*
		 *Free the global variables that may
		 *have been allocated by the parser.
		 */
		xmlCleanupParser();
		if(g_loglevel>=GATING_SET_LEVEL)
			COUT<<"xml freed!"<<endl;
	}
}

void compensation::convertToPb(pb::COMP & comp_pb){
	comp_pb.set_cid(cid);
	comp_pb.set_name(name);
	comp_pb.set_prefix(prefix);
	comp_pb.set_suffix(suffix);
	comp_pb.set_comment(comment);
	BOOST_FOREACH(vector<double>::value_type & it, spillOver){
		comp_pb.add_spillover(it);
	}
	BOOST_FOREACH(vector<string>::value_type & it, marker){
			comp_pb.add_marker(it);
		}
}
