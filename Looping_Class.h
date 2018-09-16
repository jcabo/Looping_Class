#ifndef _LOOPING_CLASS_H_
#define _LOOPING_CLASS_H_

#include <CXXFramework/BBVASmartPtr/h/TENP_BBVA_shared_ptr.h>

#include <boost/mem_fn.hpp>
#include <boost/algorithm/string/join.hpp>

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <functional>
#include <cassert>
#include <log4cxx/logger.h>
#include <log4cxx/log4cxx.h>

namespace xva {

	//////////////////////////////////////////////////////////////////////////////
	// LoopingClass class
	// Class for represent a tree structure
	//////////////////////////////////////////////////////////////////////////////

	template<class T>
	bbva::shared_ptr<T> defaultCreateNode( const std::string &id) {
		 return bbva::shared_ptr<T>(new T(id));
	}

	template<class T, class U>
	class LoopingClass
	{
	public:
		// Typedef
		typedef T DataType	;
		typedef U NodeType	;
		typedef std::function<bbva::shared_ptr<NodeType> (const std::string&)>	CreateNodeType			;
		typedef std::function<void (NodeType&)>									LoopFunctionTypeVoid	;
		typedef std::function<bool (NodeType&)>									LoopFunctionTypeBool	;

	protected:
		// Static attributes
		//static const std::string _ID_ROOT;

		// Structure
		bbva::shared_ptr<DataType> _data;

		// Tree attributes
		size_t												_deep			;
		std::string											_id				;
		std::vector<std::string> 							_path 			;
		bool												_is_root		;
		bool												_is_terminal	;		
		NodeType*											_parent			;
		std::map<std::string, bbva::shared_ptr<NodeType> >	_childs			;

		// Methods

		// Setup child added
		void setupChildAdded(bbva::shared_ptr<NodeType> &child)
		{
			child->_parent = dynamic_cast<NodeType*>(this)		;
			child->setIsRoot(false)								;
			child->setDeep(_deep+1)								; // Recursive
			child->setPath(_path)								; // Recursive
			child->getPath().push_back(child->getId())			;
		}

		// Set parent
		void setParent(NodeType* parent)
		{
			_parent = parent;
		}

	public:
		// Constructor
		LoopingClass(	const std::string &id = "ROOT"/*_ID_ROOT*/	)
			:	_deep(0)											,
				_id(id)												,
				_is_root(true)										,
				_is_terminal(true)									,
				_data(bbva::shared_ptr<DataType>(new DataType()))	,
				_parent(nullptr)
		{
			_path.push_back(_id);
		}

		LoopingClass(	bbva::shared_ptr<DataType>	data						,
						const std::string			&id	 = "ROOT"/*_ID_ROOT*/	)
			:	_deep(0)							,
				_id(id)								,
				_is_root(true)						,
				_is_terminal(true)					,
				_data(data)							,
				_parent(nullptr)
		{
			_path.push_back(_id);
		}

		virtual ~LoopingClass()
		{}

		//	setup
		void setup(	const std::vector<std::vector<std::string> >	&flat_vector					,
					const CreateNodeType							&createNode = defaultCreateNode )
		{
			for(auto it: flat_vector) {
				helper_par(it.begin(), it.end(), createNode);
			}
		}
		
		// Clear
		void clear()
		{
			_data 			= bbva::shared_ptr<DataType>(new DataType())	;
			_deep			= 0												;
			_id				= "ROOT"/*_ID_ROOT*/							;
			_path.clear() 													;
			_path.push_back(_id)											;
			_is_root		= true											;
			_is_terminal	= true											;		
			_parent			= 0 											;
			_childs.clear()													;		
		}
		
		// Getters and setters
		void setData(const bbva::shared_ptr<DataType> &data)
		{
			_data = data;
		}

		void setDeep(size_t deep)
		{
			_deep = deep;
		}

		void setId(const std::string &id)
		{
			_id = id;
		}
		
		void setPath(const std::vector<std::string> &path)
		{
			_path = path;
		}
		
		void setIsRoot(bool is_root)
		{
		    _is_root = is_root;
		}
		
		void setIsTerminal(bool is_terminal)
		{
		    _is_terminal = is_terminal;
		}

		bbva::shared_ptr<DataType> getData() const
		{
			return _data;
		}

		size_t getDeep() const 
		{
			return _deep;
		}

		const std::string& getId() const 
		{
			return _id;
		}
		
		std::vector<std::string>& getPath()   
		{
			return _path;
		}
		
		bool getIsRoot() const 
		{
			return _is_root;
		}
		
		bool getIsTerminal() const 
		{
			return _is_terminal;
		}
		
		NodeType* getParent() 
		{
			return _parent;
		}
		
		const std::map<std::string, bbva::shared_ptr<NodeType> >& getChilds() const
		{
			return _childs;
		}

		std::string getExtendedPath() const
		{
			return boost::algorithm::join(_path, "/");
		}

		size_t getMaxDeep()
		{
			size_t max_deep = 0											;
			std::vector< bbva::shared_ptr<NodeType> > terminal_nodes	;
			getVectorTerminalNodes(terminal_nodes)						;

			for(size_t i=0; i<terminal_nodes.size(); i++) {
				max_deep = (terminal_nodes[i]->getDeep()>max_deep) ? terminal_nodes[i]->getDeep() : max_deep;
			}
			return max_deep;
		}
		
		//	Add child
		void addChild(bbva::shared_ptr<NodeType> child)
		{
			if(_childs.insert(std::make_pair(child->getId(), child)).second) {
				setupChildAdded(_childs[child->getId()]);
				setIsTerminal(false);
			}
			else {
				// Log
				log4cxx_qbs::LoggerPtr logger = log4cxx_qbs::Logger::getLogger("bbva.xva")														;
				LOG4CXX_INFO( logger, "LoopingClass::addChild - Node not inserted. It already exist a child with this id " +  child->getId())	;
			}
		}

		// Delete node
		void deleteChild(const std::string &id)
		{
			if(_childs.size()>1) {
				auto it = _childs.find(id)	;
				_childs.erase(it)			;
			}
			else {
				_parent->deleteChild(_id);
			}		
		}

		// exist child
		bool exist(const std::string &id) const
		{
			return (_childs.find(id)==_childs.end())? false : true;
		}

		// exist path
		bool exist(	std::vector<std::string>::const_iterator id_begin	, 
					std::vector<std::string>::const_iterator id_end		) const
		{
			if(id_begin<id_end) {
				if(!exist(*id_begin)) {
					return false;
				}
				else {
					const std::string & id= *id_begin;
					return at(id).exist(++id_begin, id_end);
				}
			}
			return true;
		}

		// Operator =
		void operator = (const bbva::shared_ptr<DataType> &data)
		{
			setData(data);
		}
		
		//
		// Operator []
		NodeType& helper_cor (	const std::string		&id			,
								const CreateNodeType	&createNode )
		{
			if(_childs.insert(std::make_pair(id, createNode(id))).second) {
				setupChildAdded(_childs[id]);
				setIsTerminal(false);		
			}
			return *_childs.at(id);
		}

		NodeType& operator [] (	const std::string &id )
		{
			return helper_cor(id, defaultCreateNode<NodeType>);
		}

		// Operator []
		NodeType& helper_cor(	std::vector<std::string>	&ids		,
								const CreateNodeType		&createNode )
		{
			if(!ids.empty()) {
				std::string label = ids[0];
				ids.erase(ids.begin() + 0);
				return helper_cor(label, createNode).helper_cor(ids, createNode);
			}
			// Return himself for the las element of ids 
			return *dynamic_cast<NodeType*>(this);
		}

		NodeType& operator [] (	std::vector<std::string> &ids )
		{
			return helper_cor(ids, defaultCreateNode<NodeType>);	
		}

		// Operator ()
		NodeType& helper_par (	std::vector<std::string>::const_iterator	id_begin	, 
								std::vector<std::string>::const_iterator	id_end		,
								const CreateNodeType						&createNode )
		{
			if(id_begin<id_end) {
				const std::string & id= *id_begin;
				return helper_cor(id, createNode).helper_par(++id_begin, id_end, createNode);
			}
			// Return himself for the las element of ids 
			return *dynamic_cast<NodeType*>(this);
		}

		NodeType& operator () (	std::vector<std::string>::const_iterator	id_begin	, 
								std::vector<std::string>::const_iterator	id_end		)
		{
			return helper_par(id_begin, id_end, defaultCreateNode<NodeType>);
		}

		// at
		NodeType& at(const std::string &id) const  
		{
			//std::cout << "ID exit: " << _id << std::endl;
			if(_childs.find(id)==_childs.end()) {
				throw std::runtime_error( "LoopingClass::at - Not found a child with id " + id + " in node " + _id);	
			}
			return *_childs.at(id);
		}

		NodeType& at(std::vector<std::string>& ids)  
		{
			if(!ids.empty()) {
				std::string label = ids[0]		;
				ids.erase(ids.begin() + 0)		;
				return at(label).at(ids);
			}
			// Return himself for the las element of ids 
			return *dynamic_cast<NodeType*>(this);
		}

		NodeType& at(	std::vector<std::string>::const_iterator id_begin	, 
						std::vector<std::string>::const_iterator id_end		) 
		{
			if(id_begin<id_end) {
				const std::string & id= *id_begin		;
				return at(id).at(++id_begin, id_end)	;
			}
			// Return himself for the las element of ids 
			return *dynamic_cast<NodeType*>(this);
		}

		// Generic loop
		void loop(	const LoopFunctionTypeVoid &pre_function								, 
					const LoopFunctionTypeVoid &post_function								, 
					const LoopFunctionTypeBool &exp = [](NodeType& /*node*/){return true;}	)
		{
			// Pre function
			if(exp( *dynamic_cast<NodeType*>(this) )) {
				pre_function( *dynamic_cast<NodeType*>(this) )	;
			}
			// Generic loop in children
			for(auto it: getChilds()) {
				it.second->loop(pre_function, post_function, exp)	;
			}
			// Post function
			if(exp( *dynamic_cast<NodeType*>(this) )) {
				post_function( *dynamic_cast<NodeType*>(this) )	;
			}
		}

		void loopChilds(	const LoopFunctionTypeVoid &pre_function								, 
							const LoopFunctionTypeVoid &post_function								, 
							const LoopFunctionTypeBool &exp = [](NodeType& /*node*/){return true;}	)
		{
			for(auto it: _childs) {
				it.second->loop( pre_function, post_function, exp );
			}
		}

		// Gen flat map data
		void getFlatMapData( std::map<std::vector<std::string>, bbva::shared_ptr<DataType> > &flat_map_data	)
		{
			flat_map_data.clear();
			loop(	[&flat_map_data](NodeType& root){ 
						flat_map_data.insert(std::pair<std::vector<std::string>, bbva::shared_ptr<T> >(root.getPath(), root.getData()));}	,
					[](NodeType& /*root*/){} 																								);			
		}
		
		// Gen flat vector 
		void getFlatVector(	std::vector<std::vector<std::string> >	&flat_vector)
		{
			flat_vector.clear();
			loop(	[&flat_vector](NodeType& root){
						flat_vector.push_back(root.getPath());} ,
					[](NodeType& /*root*/){} 					);			
		}
		
		// Get vector child deep
		void getVectorNodesDeepN(	const size_t								deep			,
									std::vector<bbva::shared_ptr<NodeType> >	&vector_childs	)
		{
			vector_childs.clear();
			loop(	[&vector_childs, &deep](NodeType& node){
							for (auto it: node.getChilds()) {
								if (it.second->getDeep()==deep) {
									vector_childs.push_back(it.second);
								}
							}
											},
					[](NodeType& /*root*/){} 	);							
		}

		// Vector of ids level without repeat
		void getVectorIdsLevel(	const size_t				deep	,
								std::vector<std::string>	&ids	)
		{
			ids.clear();

			std::vector<bbva::shared_ptr<NodeType> > vector_childs;
			getVectorNodesDeepN( deep, vector_childs );

			for(size_t i=0; i<vector_childs.size(); i++) {
				if(std::find(ids.begin(), ids.end(), vector_childs[i]->getId()) == ids.end()) {
					ids.push_back(vector_childs[i]->getId());
				}
			}
		}

		// Get vector terminal children
		void getVectorTerminalNodes( std::vector<bbva::shared_ptr<NodeType> > &terminal_nodes )
		{
			terminal_nodes.clear();
			loop(	[&terminal_nodes](NodeType& node){
							for (auto it: node.getChilds()) {
								if (it.second->getIsTerminal()) {
									terminal_nodes.push_back(it.second);
								}
							}
											},
					[](NodeType& /*node*/){} 	);							
		}
		
		// Print 
		void printPath(	std::ostream		&os_output	,
						size_t				level		,
						const std::string	&delimiter	) const
		{
			if(_path.size()>0) {
				os_output << _path[0];			
				for(size_t i=1; i<_path.size(); i++) {
					os_output << delimiter	;
					os_output << _path[i]	;
				}
				for(size_t i=_path.size(); i<=level; i++) {
					os_output << delimiter	;
				}
			}
		}
	};

	// Static attributes
	/*template<class T, class U>
	const std::string LoopingClass<T,U>::_ID_ROOT = "ROOT";*/

}
#endif