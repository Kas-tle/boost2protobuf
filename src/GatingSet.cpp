/*
 * GatingSet.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: wjiang2
 */


#include "include/GatingSet.hpp"
#include <string>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <iostream>
#include <exception>
#include "include/delimitedMessage.hpp"
using namespace std;
/**
 * separate filename from dir to avoid to deal with path parsing in c++
 * @param path the dir of filename
 * @param filename
 */
void GatingSet::serialize_pb(string filename){
		// Verify that the version of the library that we linked against is
		// compatible with the version of the headers we compiled against.
		GOOGLE_PROTOBUF_VERIFY_VERSION;
		//init the output stream
		ofstream output(filename.c_str(), ios::out | ios::trunc | ios::binary);
		google::protobuf::io::OstreamOutputStream raw_output(&output);

		//empty message for gs
		pb::GatingSet gs_pb;

		/*
		 * add messages for trans
		 */

		//save the address of global biexp (as 1st entry)
		pb::TRANS_TBL * trans_tbl_pb = gs_pb.add_trans_tbl();
		intptr_t address = (intptr_t)&globalBiExpTrans;
		trans_tbl_pb->set_trans_address(address);


		//save the address of global lintrans(as 2nd entry)
		trans_tbl_pb = gs_pb.add_trans_tbl();
		address = (intptr_t)&globalLinTrans;
		trans_tbl_pb->set_trans_address(address);


		// cp trans group
		BOOST_FOREACH(trans_global_vec::value_type & it, gTrans){
			pb::trans_local * tg = gs_pb.add_gtrans();
			it.convertToPb(*tg, gs_pb);
		}

		//add sample name
		BOOST_FOREACH(gh_map::value_type & it,ghs){
				string sn = it.first;
				gs_pb.add_samplename(sn);
		}

		//write gs message to stream
		bool success = writeDelimitedTo(gs_pb, raw_output);

		if (!success){
			google::protobuf::ShutdownProtobufLibrary();
			throw(domain_error("Failed to write GatingSet."));
		}else
		{
			/*
			 * write pb message for each sample
			 */

			BOOST_FOREACH(gh_map::value_type & it,ghs){
					string sn = it.first;
					GatingHierarchy * gh =  it.second;

					pb::GatingHierarchy pb_gh;
					gh->convertToPb(pb_gh);
					//write the message
					bool success = writeDelimitedTo(pb_gh, raw_output);
					if (!success)
						throw(domain_error("Failed to write GatingHierarchy."));
			}

		}

		// Optional:  Delete all global objects allocated by libprotobuf.
		google::protobuf::ShutdownProtobufLibrary();
}
/**
 * constructor from the archives (de-serialization)
 * @param filename
 * @param format
 * @param isPB
 */
GatingSet::GatingSet(string filename, unsigned short format, bool isPB):wsPtr(NULL)
{


		// open the archive
		std::ios::openmode mode = std::ios::in;
		if(format == ARCHIVE_TYPE_BINARY)
			mode = mode | std::ios::binary;
		std::ifstream ifs(filename.c_str(), mode);

		switch(format)
		{
		case ARCHIVE_TYPE_BINARY:
			{
				boost::archive::binary_iarchive ia(ifs);
				ia >> BOOST_SERIALIZATION_NVP(*this);
			}

			break;
		case ARCHIVE_TYPE_TEXT:
			{
				boost::archive::text_iarchive ia1(ifs);
				ia1 >> BOOST_SERIALIZATION_NVP(*this);
			}

			break;
		case ARCHIVE_TYPE_XML:
			{
				boost::archive::xml_iarchive ia2(ifs);
				ia2 >> BOOST_SERIALIZATION_NVP(*this);
			}

			break;
		default:
			throw(invalid_argument("invalid archive format!only 0,1 or 2 are valid type."));
			break;

		}



}


/*
 * this should be called only in GatingSet destructor
 * because it is shared by every GatingHiearchy class
 * thus should not be deleted separately (otherwise it causes some segfault particulary on mac)
 */
void GatingSet::freeWorkspace(){
	if(wsPtr!=NULL)
	{
		delete wsPtr;
		wsPtr = NULL;
	}

}
GatingSet::~GatingSet()
{
	if(g_loglevel>=GATING_SET_LEVEL)
		COUT<<endl<<"start to free GatingSet..."<<endl;

	freeWorkspace();

	BOOST_FOREACH(gh_map::value_type & it,ghs){
		GatingHierarchy * ghPtr=it.second;
		string sampleName=it.first;
		if(g_loglevel>=GATING_HIERARCHY_LEVEL)
			COUT<<endl<<"start to free GatingHierarchy:"<<sampleName<<endl;

		delete ghPtr;

	}

	for(trans_global_vec::iterator it=gTrans.begin();it!=gTrans.end();it++)
	{
		trans_map curTrans=it->getTransMap();
		if(g_loglevel>=GATING_SET_LEVEL)
			COUT<<endl<<"start to free transformatioin group:"<<it->getGroupName()<<endl;
		for(trans_map::iterator it1=curTrans.begin();it1!=curTrans.end();it1++)
		{
			transformation * curTran=it1->second;
			if(curTran!=NULL)
			{
				if(g_loglevel>=GATING_SET_LEVEL)
						COUT<<"free transformatioin:"<<curTran->getChannel()<<endl;

				delete curTran;
				curTran = NULL;
			}

		}

	}

}
