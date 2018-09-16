#ifndef _K_LOOPING_CLASS_H_
#define _K_LOOPING_CLASS_H_

#include <CXXFramework/BBVASmartPtr/h/TENP_BBVA_shared_ptr.h>
#include <XVA_Libraries/XVACommons/H/K_LoopingCoreBridgeClass.h>
#include <XVA_Libraries/XVACommons/H/K_Loop_Core_Class.h>
#include <XVA_Libraries/XVACommons/H/XVACommons_API_Exports.h>

#include <iostream>
#include <vector>

namespace xva {

	//////////////////////////////////////////////////////////////////////////////////
	//
	//	K_LoopNesting_Looping_Class
	//
	//	Run an iterative loop to execute 'K_LoopCoreArgumentClass' methods
	//
	//////////////////////////////////////////////////////////////////////////////////
	class XVACOMMONS_API K_LoopNesting_Looping_Class
	{
		protected:

			K_LoopNesting_Looping_Class														*_parent_node				;

			//	ID for the current node (this pointer)
			std::vector			< std::string >												_structure_nodes_ids		;
			
			//	Children nodes
			std::vector			< bbva::shared_ptr < K_LoopNesting_Looping_Class > >		_lower_level_looping_class	;

			//	For instance, these will be the Collateral Posting class, contained within the EAD at Collateral Set Level,
			//	in the development of the original case (we will assume that the calculation of the composition of the
			//	aggregated value of the collateral accounts has been performed prior to capital calculation (EAD calculation)
			//	The name 'bridge' comes form the fact that they will be the communication link between structure and functionality,
			//	provided by certain groups of input data (such as deals data or collateral posting data)
			bbva::shared_ptr < K_LoopingCoreBridgeClass >									_current_level_bridge_class		;
			
		public:

			K_LoopNesting_Looping_Class():
				_parent_node(NULL)
			{}

			void setCurrentLevelBridgeClass(bbva::shared_ptr < K_LoopingCoreBridgeClass >	inp_current_level_bridge_class);

			bbva::shared_ptr < K_LoopingCoreBridgeClass > getCurrentLevelBridgeClass() const
			{
				return _current_level_bridge_class;
			}

			void addLowerLevelLoopingClass(bbva::shared_ptr < K_LoopNesting_Looping_Class >	inp_lower_level_looping_class);

			virtual void pre_nested_loop_operations (bbva::shared_ptr < K_LoopCoreArgumentClass > argument, CapitalType k_type);

			virtual void post_nested_loop_operations (bbva::shared_ptr < K_LoopCoreArgumentClass > argument, CapitalType k_type);

			virtual void loop (bbva::shared_ptr < K_LoopCoreArgumentClass > argument, CapitalType k_type);

			virtual void createTree(bbva::shared_ptr < K_LoopCoreArgumentClass > argument, CapitalType k_type);
	

			virtual bool consider4Computation(CapitalType										k_type		) const;

			const std::string& id() const 
			{
				return _structure_nodes_ids[_structure_nodes_ids.size()-1];
			}
			
			void insertNestingLevelId(std::string new_level_id)
			{
				_structure_nodes_ids.push_back(new_level_id);
			}

			void insertNestingLevelId(const std::vector<std::string>& structure_nodes_ids)
			{
				_structure_nodes_ids.insert( _structure_nodes_ids.end()	 ,
											 structure_nodes_ids.begin() ,
											 structure_nodes_ids.end()	 );
			}

			const std::vector<std::string>& getNestingLevel() const
			{
				return _structure_nodes_ids;
			}

			void getLowerLevelLoopingClass(std::vector< bbva::shared_ptr < K_LoopNesting_Looping_Class > >	&lower_level_looping_class)
			{
				lower_level_looping_class = _lower_level_looping_class	;
			}
			
			bbva::shared_ptr<K_LoopNesting_Looping_Class> getLowerLevelLoopingClass(const std::string& name);

			bool checkLowerLevelLoopingClass(const std::string& name);

			void setParentNode(K_LoopNesting_Looping_Class *parent_node);

			virtual void setLoopZeroCollatUpdater();

			virtual void setZeroCollatUpdater();

			void dump2Logger() const;

			void getInvolvedInstruments( std::set<std::string>		&csa_involved_ccys			,
										 std::set<std::string>		&deal_involved_ccys			,
										 std::set<std::string>		&irates_instr_names			,
										 std::set<std::string>		&irates_options_instr_names	,
										 std::set<std::string>		&equity_instr_names			,
										 std::set<std::string>		&equity_options_instr_names	,
										 std::set<std::string>		&fx_instr_names				,
										 std::set<std::string>		&fx_options_instr_names		) const;


			//
			//	CLASS DESTRUCTOR
			//
			virtual ~K_LoopNesting_Looping_Class()
			{}
	};			
}

#endif