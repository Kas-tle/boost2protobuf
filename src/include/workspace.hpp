/*
 * workspace.hpp
 *
 *  Created on: Mar 22, 2012
 *      Author: wjiang2
 */

#ifndef WORKSPACE_HPP_
#define WORKSPACE_HPP_
#include <vector>
#include <string>
#include <libxml/xpath.h>
#include <libxml/parser.h>
#include "wsNode.hpp"
#include "transformation.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>


using namespace std;


using namespace std;
/*TODO: so far I have seen the major difference between win and mac workspace is the xpath(like xpath of sample node)
 * if this is the case eventually we can try to use one template class (eliminate two derived classes )
 * with T structure that stores different versions of xpaths for win/mac,for example:
 *
 * struct winWorkspace{
 * xpath_sample=xxx
 * ....
 * }
 *
 * struct macWorkspace{
 * xpath_sample=xxx
 * ....
 * }
 *
 * this may potentially reduce the amount of code
 *
 */
class compensation{
//	friend std::ostream & operator<<(std::ostream &os, const compensation &gh);
	friend class boost::serialization::access;

public:
	string cid;
	string prefix;
	string suffix;
	string name;
	string comment;// store "Acquisition-defined" when the spillOver matrix is not supplied and cid==-1
	vector<string> marker;
	vector<double> spillOver;
	void convertToPb(pb::COMP & comp_pb);
	compensation(){};
private:
template<class Archive>
				void serialize(Archive &ar, const unsigned int version)
				{
					ar & BOOST_SERIALIZATION_NVP(cid);
					ar & BOOST_SERIALIZATION_NVP(prefix);
					ar & BOOST_SERIALIZATION_NVP(suffix);
					ar & BOOST_SERIALIZATION_NVP(name);
					ar & BOOST_SERIALIZATION_NVP(comment);
					ar & BOOST_SERIALIZATION_NVP(marker);
					ar & BOOST_SERIALIZATION_NVP(spillOver);
				}

};


struct xpath{
	string group;
	string sampleRef;
	string sample;
	string sampleNode;
	string popNode;
	string gateDim;
	string gateParam;

	string attrName;
	string compMatName;
	string compMatChName;
	string compMatVal;
	unsigned short sampNloc;//get FCS filename(or sampleName) from either $FIL keyword or name attribute of sampleNode
	template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{


			ar & BOOST_SERIALIZATION_NVP(group);
			ar & BOOST_SERIALIZATION_NVP(sampleRef);
			ar & BOOST_SERIALIZATION_NVP(sample);
			ar & BOOST_SERIALIZATION_NVP(sampleNode);
			ar & BOOST_SERIALIZATION_NVP(popNode);
			ar & BOOST_SERIALIZATION_NVP(sampNloc);

			ar & BOOST_SERIALIZATION_NVP(attrName);
			ar & BOOST_SERIALIZATION_NVP(compMatName);
			ar & BOOST_SERIALIZATION_NVP(compMatChName);
			ar & BOOST_SERIALIZATION_NVP(compMatVal);
		}
};



class workspace{
//	friend std::ostream & operator<<(std::ostream &os, const workspace &gh);
	friend class boost::serialization::access;
public:
	 xpath nodePath;
//protected:

	 xmlDoc * doc;

private:
	 template<class Archive>
	 		    void save(Archive &ar, const unsigned int version) const
	 		    {
	 				ar & BOOST_SERIALIZATION_NVP(nodePath);
	 				ar & BOOST_SERIALIZATION_NVP(doc);

	 		    }
	 template<class Archive>
	 	 		    void load(Archive &ar, const unsigned int version)
	 	 		    {
	 	 				ar & BOOST_SERIALIZATION_NVP(nodePath);
	 	 				ar & BOOST_SERIALIZATION_NVP(doc);
	 	 				if(version<2){
	 	 					unsigned short dMode;
	 	 					ar & BOOST_SERIALIZATION_NVP(dMode);
	 	 				}

	 	 		    }
	BOOST_SERIALIZATION_SPLIT_MEMBER()

public:
	 workspace(){doc=NULL;};
	 virtual ~workspace();

};
BOOST_CLASS_VERSION(workspace,2)

#endif /* WORKSPACE_HPP_ */

