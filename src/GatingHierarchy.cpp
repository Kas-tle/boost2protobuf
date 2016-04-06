/*
 * GatingHierarchy.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: wjiang2
 */

#include "include/GatingHierarchy.hpp"




void GatingHierarchy::convertToPb(pb::GatingHierarchy & gh_pb){
	pb::populationTree * ptree = gh_pb.mutable_tree();
	/*
	 * cp tree
	 */
	VertexID_vec verIDs = getVertices();
	for(VertexID_vec::iterator it = verIDs.begin(); it != verIDs.end(); it++){
		VertexID thisVert = *it;
		nodeProperties & np = getNodeProperty(thisVert);

		pb::treeNodes * node = ptree->add_node();
		pb::nodeProperties * pb_node = node->mutable_node();
		bool isRoot = thisVert == 0;
		np.convertToPb(*pb_node, isRoot);
		if(!isRoot){
			node->set_parent(getParent(thisVert));
		}


	}
	//cp comp
	pb::COMP * comp_pb = gh_pb.mutable_comp();
	comp.convertToPb(*comp_pb);
	//cp trans
	pb::trans_local * trans_pb = gh_pb.mutable_trans();
	trans.convertToPb(*trans_pb);
	//cp trans flag
	BOOST_FOREACH(PARAM_VEC::value_type & it, transFlag){
		pb::PARAM * tflag_pb = gh_pb.add_transflag();
		it.convertToPb(*tflag_pb);
	}


}


class custom_bfs_visitor : public boost::default_bfs_visitor
{

public:
	custom_bfs_visitor(VertexID_vec& v) : vlist(v) { }
	VertexID_vec & vlist;
  template < typename Vertex, typename Graph >
  void discover_vertex(Vertex u, const Graph & g) const
  {
	  vlist.push_back(u);
//	  v=u;
  }

};

/*
 * retrieve the vertexIDs in topological order,BFS or in regular order
 */

VertexID_vec GatingHierarchy::getVertices(unsigned short order){

	VertexID_vec res, vertices;
	switch (order)
	{

		case REGULAR:
		{
			VertexIt it_begin,it_end;
			boost::tie(it_begin,it_end)=boost::vertices(tree);
			for(VertexIt it=it_begin;it!=it_end;it++)
				res.push_back((unsigned long)*it);
		}
		break;

		case TSORT:
		{
			boost::topological_sort(tree,back_inserter(vertices));
			for(VertexID_vec::reverse_iterator it=vertices.rbegin();it!=vertices.rend();it++)
				res.push_back(*it);
		}
		break;

		case BFS:
		{
			custom_bfs_visitor vis(res);
//			vector<VertexID> p(num_vertices(tree));
//			populationTree tree_copy(num_vertices(tree));
			boost::breadth_first_search(tree, vertex(0, tree)
										, boost::visitor(
												vis
//												boost::make_bfs_visitor(boost::record_predecessors(&p[0]
//																									 ,boost::on_tree_edge()
//																									)
//																					)
														)
										);
//			res=vis.vlist;

		}
		break;

		default:
			throw(domain_error("not valid sort type for tree traversal!"));
	}

	return(res);

}

/*
 * using boost in_edges out_edges to retrieve adjacent vertices
 * assuming only one parent for each node
 */
EdgeID GatingHierarchy::getInEdges(VertexID target){
	vector<EdgeID> res;
	string err;
	err.append(boost::lexical_cast<string>(target));

	if(target<=boost::num_vertices(tree)-1)
	{

		boost::graph_traits<populationTree>::in_edge_iterator in_i, in_end;

		for (boost::tie(in_i, in_end) = in_edges(target,tree);
			         in_i != in_end; ++in_i)
		{
			EdgeID e = *in_i;
			res.push_back(e);
		}

	}
	else
		throw(domain_error(err+" :invalid vertexID!"));


	if(res.size()==0)
		throw(domain_error(err+" :parent not found!"));
	if(res.size()>1) //we only allow one parent per node
		throw(domain_error(err+" :multiple parent nodes found!"));

	return(res.at(0));
}

VertexID GatingHierarchy::getParent(VertexID target){
	EdgeID e=getInEdges(target);
	return  boost::source(e, tree);
}
/*
 *
 * make sure to use this API always since since it is safe way to access tree nodes due to the validity check
 *
 *since the vertex bundle should always exist as long as the  tree and node exist, thus it is safe
 * to return the reference of it
 */
nodeProperties & GatingHierarchy::getNodeProperty(VertexID u){


	if(u<=boost::num_vertices(tree)-1)
		return(tree[u]);
	else
	{
		throw(out_of_range("returning empty node due to the invalid vertexID:" + boost::lexical_cast<std::string>(u)));

	}
}

