#include"BTSolver.hpp"


using namespace std;

// =====================================================================
// Constructors
// =====================================================================

BTSolver::BTSolver(SudokuBoard input, Trail* _trail, string val_sh, string var_sh, string cc)
	: sudokuGrid(input.get_p(), input.get_q(), input.get_board()), network(input)
{
	valHeuristics = val_sh;
	varHeuristics = var_sh;
	cChecks = cc;

	trail = _trail;
}

// =====================================================================
// Consistency Checks
// =====================================================================

// Basic consistency check, no propagation done
bool BTSolver::assignmentsCheck(void)
{
	//cout << endl << "assignment check called" << endl;
	for (Constraint c : network.getConstraints())
		if (!c.isConsistent())
			return false;

	return true;
}

/**
* Part 1 TODO: Implement the Forward Checking Heuristic
*
* This function will do both Constraint Propagation and check
* the consistency of the network
*
* (1) If a variable is assigned then eliminate that value from
*     the square's neighbors.
*
* Note: remember to trail.push variables before you assign them
* Return: true is assignment is consistent, false otherwise
*/
bool BTSolver::forwardChecking(void)
{
	if (!network.isConsistent())//stops ealry(before push) if board was already wrong
	{
		//cout << endl<<"board started wrong" << endl;
		return false;
	}
	int varsAssigned = 0;
	for (Variable *v : network.getVariables())//this loop looks at each variable
	{
		if (v->isAssigned())//checks if the variable is assigned
		{
			varsAssigned++;
			//loop to eliminate value from neighbors
			for (Variable *neighbor : network.getNeighborsOfVariable(v))
			{
				if (!neighbor->isAssigned())
				{
					neighbor->removeValueFromDomain(v->getAssignment());


					if (!network.isConsistent())//stops ealry if problem is not obvious error(error found through propagation of 1 value)
					{
						//cout << endl << "nonobvious error" << endl;
						neighbor->getDomain().add(v->getAssignment());
						return false;
					}
					neighbor->getDomain().add(v->getAssignment());
					trail->push(neighbor);
					neighbor->removeValueFromDomain(v->getAssignment());//removes v's value from neighbor's domain
				}
			}
		}
	}

	//cout << endl << "Vars Assigned So Far: " << varsAssigned << endl;
	return assignmentsCheck();//returns true if consistent
}

/**
* Part 2 TODO: Implement both of Norvig's Heuristics
*
* This function will do both Constraint Propagation and check
* the consistency of the network
*
* (1) If a variable is assigned then eliminate that value from
*     the square's neighbors.
*
* (2) If a constraint has only one possible place for a value
*     then put the value there.
*
* Note: remember to trail.push variables before you assign them
* Return: true is assignment is consistent, false otherwise
*/
bool BTSolver::norvigCheck(void)
{
	


	if(!forwardChecking())//forward checking is the first part of norvig// return false for early errors
	{
		return false;
	}
	//do (2) checking the constraints
	//loop goes through constraints

	int unassignedCount = 0; //want to end if unassigned is above 1
	Variable* unassigned;
	int N = network.getConstraints()[0].size();//N holds the size of a constraint//constraints are all the same size
	int sumVals;//holds the sum of 1 to N(all possible values)
	
	for (Constraint c : network.getConstraints())
	{
		sumVals = N*(N + 1) / 2;
		
		//loop through constraint 
		for (Variable*v : c.vars)
		{
			if (!(v->isAssigned()))
			{
				unassignedCount++;
				unassigned = v;
			}
			else
			{
				sumVals -= v->getAssignment();//subtracts assigned value from sumVals//sumVals will be left with the missing value when 1 var is left
			}
			if(unassignedCount== 2)
			{
				break;
			}
			
		}
		
		
		if (unassignedCount ==1)
		{
			trail->push(unassigned);
			unassigned->assignValue(sumVals);
		
		}
		unassignedCount = 0;
	}

	
	return assignmentsCheck();
}

/**
* Optional TODO: Implement your own advanced Constraint Propagation
*
* Completing the three tourn heuristic will automatically enter
* your program into a tournament.
*/
bool BTSolver::getTournCC(void)
{

	if (!norvigCheck())
	{
		return false;
	}

	/*Single Candidate Technique*/

	vector<int> checkList;//keeps track of how many times a value is seen in a constraint
	int N = network.getConstraints()[0].size();// the size of a constraint
	for (Constraint c : network.getConstraints())
	{
		checkList.assign(N, 0);
		for (Variable*v : c.vars)
		{
			if (!v->isAssigned())
			{
				for (int i : v->getValues())
				{
					checkList[i - 1]++;//increments counter for value
				}
			}
		}//end of loop setting up checklist

		for (int i = 0; i < checkList.size(); ++i)//finds the value that appears once in the constraint
		{
			if (checkList[i] == 1)
			{
				for (Variable*v : c.vars)//finds the variable with the value(i+1) in domain
				{
					if (v->getDomain().contains(i + 1))
					{

						Domain temp(v->getDomain());//temp domain to look ahead once
						v->assignValue(i + 1);//this is for looking ahead
						if (!network.isConsistent())//stops ealry if problem is not obvious error(error found through propagation of 1 value)
						{
							//cout << endl << "nonobvious error in Single Candidate" << endl;
							v->setDomain(temp);//
						}
						else
						{
							v->setDomain(temp);//sets up v for backtracking
							trail->push(v);
							v->assignValue(i + 1);//commits to assignment
						}
					}
				}
			}
		}
	}


	
	return assignmentsCheck();
}

// =====================================================================
// Variable Selectors
// =====================================================================

// Basic variable selector, returns first unassigned variable
Variable* BTSolver::getfirstUnassignedVariable(void)
{
	for (Variable* v : network.getVariables())
		if (!(v->isAssigned()))
			return v;

	// Everything is assigned
	return nullptr;
}

/**
* Part 1 TODO: Implement the Minimum Remaining Value Heuristic
*
* Return: The unassigned variable with the smallest domain
*/
Variable* BTSolver::getMRV(void)
{//Want to go through each value to get variable with min domian
	float minimumDomain = INFINITY;
	Variable * smallV = nullptr;//variable with smallest Domain// initialized to null for when everything is assigned
	for (Variable* v : network.getVariables())
	{
		if (!(v->isAssigned()))//checks if the variable is not assigned
		{
			if (v->size() < minimumDomain)
			{
				minimumDomain = v->size();
				smallV = v;
				//used for testing//cout << "found a small one "<< v->toString() << endl;
			}
		}
	}
	return smallV;
}

/**
* Part 2 TODO: Implement the Minimum Remaining Value Heuristic
*                with Degree Heuristic as a Tie Breaker
*
* Return: The unassigned variable with, first, the smallest domain
*         and, second, the most unassigned neighbors
*/
Variable* BTSolver::getDegree(void)
{
	//picks the variable with the most unassigned neighbors
	int maxDegree = 0;//this is the the lowest possible number of unassigned neighbors
	int DegreeCount = 0;
	Variable * maxV = nullptr;//must be null if everything is assigned

	//cout << endl << "start of DEG" << endl;
	for (Variable* v : network.getVariables())
	{
		if (!(v->isAssigned()))//checks if the variable is not assigned
		{
			//if var is not assigned, find out how many neighbors, compare to max
			//goes through list of neighbors of v
			for (Variable * neighbor : network.getNeighborsOfVariable(v))
			{
				//when neighbor is not assigned it is added to degree count
				if (!(neighbor->isAssigned()))
				{
					DegreeCount++;
				}
			}
			if (DegreeCount >= maxDegree)
			{
				maxDegree = DegreeCount;
				maxV = v;
			}
		}
		//cout << endl << "MaxV:" << maxV->getName() << endl;
		DegreeCount = 0;
	}
	//cout << endl << "End of DEG" << endl;
	
	return maxV;
}

/**
* Part 2 TODO: Implement the Minimum Remaining Value Heuristic
*                with Degree Heuristic as a Tie Breaker
*
* Return: The unassigned variable with the smallest domain and involved
*             in the most constraints
*/
Variable* BTSolver::MRVwithTieBreaker(void)
{
	float minimumDomain = INFINITY;
	Variable * smallV = nullptr;//variable with smallest Domain// initialized to null for when everything is assigned

	int vDegree;
	int smallDegree = 0;//counters for degree tie breaker//counts the degree of smallV
	//smallDegree is set to 0 here so it's value can be saved
	for (Variable* v : network.getVariables())
	{
		if (!(v->isAssigned()))//checks if the variable is not assigned
		{
			if (v->size() < minimumDomain)
			{
				minimumDomain = v->size();
				smallV = v;
				//need to find the degree of minimum here to avoid constantly calculating it in ties(cases with many ties)
				for (Variable*neighbor : network.getNeighborsOfVariable(v))
				{
					if (!(neighbor->isAssigned()))
					{
						smallDegree++;
					}
				}
			}
			else if (v->size() == minimumDomain)//if the domain size of v and min are equal then use degree heuristic to pick one
			{
				vDegree = 0;
				//counts v's neighbors
				for (Variable*neighbor : network.getNeighborsOfVariable(v))
				{
					if (!(neighbor->isAssigned()))
					{
						vDegree++;
					}
				}
				if (vDegree > smallDegree)
				{
					smallV = v;
					smallDegree = vDegree;
				}
				
			}
		}
	}
	return smallV;

}

/**
* Optional TODO: Implement your own advanced Variable Heuristic
*
* Completing the three tourn heuristic will automatically enter
* your program into a tournament.
*/
Variable* BTSolver::getTournVar(void)
{

	/*Single Candidate Technique; once finished doing that as much as possible, uses MRV with TieBreaker*/

	vector<int> checkList;//keeps track of how many times a value is seen in a constraint
	int N = network.getConstraints()[0].size();// the size of a constraint
	for (Constraint c : network.getConstraints())
	{
		checkList.assign(N, 0);
		for (Variable*v : c.vars)
		{
			if (!v->isAssigned())
			{
				for (int i : v->getValues())
				{
					checkList[i - 1]++;//increments counter for value
				}
			}
		}//end of loop setting up checklist

		for (int i = 0; i < checkList.size(); ++i)//finds the value that appears once in the constraint
		{
			if (checkList[i] == 1)
			{
				for (Variable*v : c.vars)//finds the variable with the value(i+1) in domain
				{
					if (v->getDomain().contains(i + 1))
					{
				
						Domain temp(v->getDomain());//temp domain to look ahead once
						v->assignValue(i + 1);//this is for looking ahead
						if (!network.isConsistent())//stops ealry if problem is not obvious error(error found through propagation of 1 value)
						{
							//cout << endl << "nonobvious error in variable selector" << endl;
							v->setDomain(temp);//
						}
						else
						{
							v->setDomain(temp);//sets up v for backtracking
							trail->push(v);
							v->assignValue(i + 1);//commits to assignment
						}
					}
				}
			}
		}
	}

	return MRVwithTieBreaker();
}

// =====================================================================
// Value Selectors
// =====================================================================

// Default Value Ordering
vector<int> BTSolver::getValuesInOrder(Variable* v)
{
	vector<int> values = v->getDomain().getValues();
	sort(values.begin(), values.end());
	return values;
}

/**
* Part 1 TODO: Implement the Least Constraining Value Heuristic
*
* The Least constraining value is the one that will knock the least
* values out of it's neighbors domain.
*
* Return: A list of v's domain sorted by the LCV heuristic
*         The LCV is first and the MCV is last
*/

vector<int> BTSolver::getValuesLCVOrder(Variable* v)
{//need to look through the domain of v
	//goal is to sort the domain of v by LCV
	//iterate through neighbors and then main domain to save computations
	vector<int> sortedDomain;
	Domain vDomain = v->getDomain();
	map<int, int> DomainAndConstraint;//maps domain(key) to number of constraints
	//cout << endl <<"Runs LCV function" << endl;
	for (Variable* neighbor : network.getNeighborsOfVariable(v))
	{
		if (!neighbor->isAssigned())//only need to check constraints if neighbor is not assigned
		{
			int AvailablVals = 0;//counts values
			for (int domainVal : vDomain.getValues())//goes through the domain of v for each neighbor
			{
				//need to go through neighbor's domain to count the constraints for domainVal
				//need to initialize map and keys particularly

				AvailablVals = neighbor->size()  - neighbor->getDomain().contains(domainVal);
				/*
				will subtract 1 from the neighbor.size() if the domainVal is in neighbor's domain because they can't have the same domain values
				kind of like a pseudo forward check
				*/

				if (!DomainAndConstraint.count(domainVal))//checks if domainVal is in map//if not then creates it in map//runs for 1st neighbor
				{
					DomainAndConstraint[domainVal] = AvailablVals;
				}
				else
				{
					DomainAndConstraint[domainVal] += AvailablVals;
				}
				
				
				//neighbor->getDomain()->getValues()
			}
		}
	}
	//need to make a vector of pairs to then sort by value and then put into sortedDomain
	vector<pair<int, int>> Pairs;
	for (pair<int, int> P : DomainAndConstraint)
	{
		Pairs.push_back(P);
	}
	//now to sort the pairs
	//now the sort begins, sorts by largest number of available vals via sortByConstraintVal
	sort(Pairs.begin(), Pairs.end(), [](const pair<int, int> &a, const pair<int, int> &b) {return(a.second > b.second); });
	//sort over now put everything in sorted Domain, the first value is the domain value
	for (pair<int, int> P : DomainAndConstraint)
	{
		sortedDomain.push_back(P.first);
	}
	
	return sortedDomain;
}


/**
* Optional TODO: Implement your own advanced Value Heuristic
*
* Completing the three tourn heuristic will automatically enter
* your program into a tournament.
*/
vector<int> BTSolver::getTournVal(Variable* v)
{

	return getValuesLCVOrder(v);
}

// =====================================================================
// Engine Functions
// =====================================================================

void BTSolver::solve(void)
{
	if (hasSolution)
		return;

	// Variable Selection
	Variable* v = selectNextVariable();

	if (v == nullptr)
	{
		for (Variable* var : network.getVariables())
		{
			// If all variables haven't been assigned
			if (!(var->isAssigned()))
			{
				cout << "Error" << endl;
				return;
			}
		}

		// Success
		hasSolution = true;
		return;
	}

	// Attempt to assign a value
	for (int i : getNextValues(v))
	{
		// Store place in trail and push variable's state on trail
		trail->placeTrailMarker();
		trail->push(v);

		// Assign the value

		//cout << endl << v->toString() << endl;
		Domain temp(v->getDomain());
		v->assignValue(i);
		//cout << endl << v->toString() << endl;
		// Propagate constraints, check consistency, recurse
		if (checkConsistency())
			solve();

		// If this assignment succeeded, return
		if (hasSolution)
			return;

		// Otherwise backtrack
		//cout << endl << "Domain before assignment: " <<temp.toString() << endl;
		//cout << endl << v->toString() << endl;
		//cout << endl<< network.toSudokuBoard(3, 3).toString() << endl;
		trail->undo();
	}
	
}

bool BTSolver::checkConsistency(void)
{
	if (cChecks == "forwardChecking")
		return forwardChecking();

	if (cChecks == "norvigCheck")
		return norvigCheck();

	if (cChecks == "tournCC")
		return getTournCC();

	return assignmentsCheck();
}

Variable* BTSolver::selectNextVariable(void)
{
	if (varHeuristics == "MinimumRemainingValue")
		return getMRV();

	if (varHeuristics == "Degree")
		return getDegree();

	if (varHeuristics == "MRVwithTieBreaker")
		return MRVwithTieBreaker();

	if (varHeuristics == "tournVar")
		return getTournVar();

	return getfirstUnassignedVariable();
}

vector<int> BTSolver::getNextValues(Variable* v)
{
	if (valHeuristics == "LeastConstrainingValue")
		return getValuesLCVOrder(v);

	if (valHeuristics == "tournVal")
		return getTournVal(v);

	return getValuesInOrder(v);
}

bool BTSolver::haveSolution(void)
{
	return hasSolution;
}

SudokuBoard BTSolver::getSolution(void)
{
	return network.toSudokuBoard(sudokuGrid.get_p(), sudokuGrid.get_q());
}

ConstraintNetwork BTSolver::getNetwork(void)
{
	return network;
}


