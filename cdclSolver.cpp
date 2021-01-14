//g++ --std=c++11 cdclSolver.cpp -o cdclSolver
// cdclSolver < file.txt


#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#include <ctime> 

using namespace std;

enum RetVal 
{
  satisfied, 
  unsatisfied, 
  normal       
};

class SATSolverCDCL {
private:

  vector<int> literals;
  vector<vector<int>> literalsPerClause;
  vector<int> literalFrequency;
  vector<int> literalPolarity;
  vector<int> ogLiteralFrequency;
  int literalCount;   
  int clauseCount;   
  int ogClauseCount;
  int conflictAntecedent; 
  int branchCount = 0;
  vector<int> literalDecisionLevel;

  vector<int> literalAntecedentClause;
  int assignedLiteralCount; 
  bool previouslyUnsatisfied;   
  int pickCounter; 

  random_device randomGenerator;
  mt19937 generator;

  int unitPropagation(int); 
  void literalAssignment(int, int, int);
  void literalUnassignment(int); 
  int findLiteralIndex(int);
  int conflictAnalysis(int); 
  vector<int> &resolve(vector<int> &, int);     
  int chooseNextVariable(); 
  bool allAssigned();
  void outputAnswer(int);        

public:
  SATSolverCDCL() : generator(randomGenerator()) {} 
  void initialize();                                 
  int CDCL(); 
  void solve(); 
};

void SATSolverCDCL::initialize() {
  char c;   
  string s; 

  while (true) {
    cin >> c;
    if (c == 'c') 
    {
      getline(cin, s); 
    } else             
    {
      cin >> s; 
      break;
    }
  }
  cin >> literalCount;
  cin >> clauseCount;
  ogClauseCount = clauseCount;
  assignedLiteralCount = 0; 
  conflictAntecedent = -1;
  pickCounter = 0;
  previouslyUnsatisfied = false;

  literals.clear();
  literals.resize(literalCount, -1);
  literalFrequency.clear();
  literalFrequency.resize(literalCount, 0);
  literalPolarity.clear();
  literalPolarity.resize(literalCount, 0);
  literalsPerClause.clear();
  literalsPerClause.resize(clauseCount);
  literalAntecedentClause.clear();
  literalAntecedentClause.resize(literalCount, -1);
  literalDecisionLevel.clear();
  literalDecisionLevel.resize(literalCount, -1);

  int literal;                  
  int clauseLiteralCount = 0; 
  for (int i = 0; i < clauseCount; i++) {
    clauseLiteralCount = 0;
    while (true) 
    {
      cin >> literal;
      if (literal > 0) 
      {
        literalsPerClause[i].push_back(literal); 
        literalFrequency[literal - 1]++;
        literalPolarity[literal - 1]++;
      } 
      else if (literal < 0) 
      {
        literalsPerClause[i].push_back(literal); 
        literalFrequency[-1 - literal]++;
        literalPolarity[-1 - literal]--;
      } 
      else 
      {
        if (clauseLiteralCount == 0) 
        {
          previouslyUnsatisfied = true;
        }
        break; 
      }
      clauseLiteralCount++;
    }
  }
  ogLiteralFrequency = literalFrequency; 
}


int SATSolverCDCL::CDCL() 
{
  int decisionLevel = 0;  
  if (previouslyUnsatisfied)                        
  {
    return RetVal::unsatisfied;
  }
  int unitPropagationResult = unitPropagation(decisionLevel);
  if (unitPropagationResult == RetVal::unsatisfied) 
  {
    return unitPropagationResult;
  }
  while (!allAssigned()) 
  {
    int pickedVariable = chooseNextVariable(); 
    decisionLevel++; 
    literalAssignment(pickedVariable, decisionLevel, -1);

    while (true) 
    {
      unitPropagationResult = unitPropagation(decisionLevel);
      if (unitPropagationResult == RetVal::unsatisfied) 
      {
        if (decisionLevel == 0) 
        {
          return unitPropagationResult;
        }

        decisionLevel = conflictAnalysis(decisionLevel);
      } 
      else 
      {
        break;
      }
    }
  }

  return RetVal::satisfied;
}


int SATSolverCDCL::unitPropagation(int decisionLevel) 
{
  bool isUnitClause = false; 
  int falseCount = 0;           
  int unassignedCount = 0;           
  int literalIndex;
  bool isSatisfied = false; 
  int lastUnassignedLiteral = -1; 
  do 
  {
    isUnitClause = false;

    for (int i = 0; i < literalsPerClause.size() && !isUnitClause; i++) 
    {
      falseCount = 0;
      unassignedCount = 0;
      isSatisfied = false;

      for (int j = 0; j < literalsPerClause[i].size(); j++) 
      {
        literalIndex = findLiteralIndex(literalsPerClause[i][j]);
        if (literals[literalIndex] == -1) 
        {
          unassignedCount++;
          lastUnassignedLiteral = j; 
        } 
        else if ((literals[literalIndex] == 0 && literalsPerClause[i][j] > 0) || (literals[literalIndex] == 1 && literalsPerClause[i][j] < 0)) 
        {
          falseCount++;
        } 
        else
        {
          isSatisfied = true;
          break;
        }
      }
      if (isSatisfied) 
      {
        continue;
      }
      if (unassignedCount == 1) 
      {
        literalAssignment(literalsPerClause[i][lastUnassignedLiteral], decisionLevel, i);
        isUnitClause = true; 
        break;
      }
      else if (falseCount == literalsPerClause[i].size()) 
      {
        conflictAntecedent = i; 
        return RetVal::unsatisfied; 
      }
    }
  } while (isUnitClause); 
  conflictAntecedent = -1;
  return RetVal::normal; 
}


void SATSolverCDCL::literalAssignment(int variable, int decisionLevel, int antecedent) 
{
  int literal = findLiteralIndex(variable); 
  int value;
  if (variable > 0)
  {
    value = 1;
  }
  else
  {
    value = 0;
  }
  literals[literal] = value;        
  literalDecisionLevel[literal] = decisionLevel; 
  literalAntecedentClause[literal] = antecedent;        
  literalFrequency[literal] = -1; 
  assignedLiteralCount++; 
}


void SATSolverCDCL::literalUnassignment(int literalIndex) 
{
  literals[literalIndex] = -1;              
  literalDecisionLevel[literalIndex] = -1;
  literalAntecedentClause[literalIndex] = -1;   
  literalFrequency[literalIndex] =
  ogLiteralFrequency[literalIndex]; 
  assignedLiteralCount--; 
}


int SATSolverCDCL::findLiteralIndex(int variable) 
{
  if(variable > 0)
  {
    return (variable - 1);
  }
  else
  {
    return (-variable - 1);
  }

}


int SATSolverCDCL::conflictAnalysis(int decisionLevel) 
{
  vector<int> learntClause = literalsPerClause[conflictAntecedent];
  int conflictDecisionLevel = decisionLevel;
  int currentLevelCount = 0;          
  int resolutionLiteral; 
  int literal;         
  do 
  {
    currentLevelCount = 0;
    for (int i = 0; i < learntClause.size(); i++) {
      literal = findLiteralIndex(learntClause[i]); 
      if (literalDecisionLevel[literal] == conflictDecisionLevel) 
      {
        currentLevelCount++;
      }
      if (literalDecisionLevel[literal] == conflictDecisionLevel && literalAntecedentClause[literal] != -1) 
      {
        resolutionLiteral = literal;
      }
    }
    if (currentLevelCount == 1) 
    {
      break;
    }
    learntClause = resolve(learntClause, resolutionLiteral);
  } while (true);
  literalsPerClause.push_back(learntClause); 
  for (int i = 0; i < learntClause.size(); i++) 
  {
    int literalIndex = findLiteralIndex(learntClause[i]);
    int update;
    if(learntClause[i] > 0)
    {
      update = 1;
    }
    else
    {
      update = -1;
    }
    literalPolarity[literalIndex] += update;
    if (literalFrequency[literalIndex] != -1) 
    {
      literalFrequency[literalIndex]++;
    }
    ogLiteralFrequency[literalIndex]++;
  }
  clauseCount++;                     
  int backtrackedDecisionLevel = 0; 
  for (int i = 0; i < learntClause.size(); i++) 
  {
    int literalIndex = findLiteralIndex(learntClause[i]);
    int decisionLevel_here = literalDecisionLevel[literalIndex];
    if (decisionLevel_here != conflictDecisionLevel &&
        decisionLevel_here > backtrackedDecisionLevel) 
    {
      backtrackedDecisionLevel = decisionLevel_here;
    }
  }
  for (int i = 0; i < literals.size(); i++) 
  {
    if (literalDecisionLevel[i] > backtrackedDecisionLevel) 
    {
      literalUnassignment(i); 
    }
  }
  return backtrackedDecisionLevel; 
}


vector<int> &SATSolverCDCL::resolve(vector<int> &inputClause, int literal) 
{

  vector<int> resolvingClause = literalsPerClause[literalAntecedentClause[literal]];
  inputClause.insert(inputClause.end(), resolvingClause.begin(), resolvingClause.end());
  for (int i = 0; i < inputClause.size(); i++) 
  {
    if (inputClause[i] == literal + 1 || inputClause[i] == -literal - 1) 
    {
      inputClause.erase(inputClause.begin() + i);
      i--;
    }
  }
  sort(inputClause.begin(), inputClause.end());
  inputClause.erase(unique(inputClause.begin(), inputClause.end()), inputClause.end());
  return inputClause; 
}


int SATSolverCDCL::chooseNextVariable() 
{
  branchCount = branchCount + 1;
  uniform_int_distribution<int> choose_branch(1, 10);
  uniform_int_distribution<int> choose_literal(0, literalCount - 1);

  int randomValue = choose_branch(generator); 
  bool reachedMaxAttempts = false;
  int attemptCounter = 0;
  do 
  {
    if (randomValue > 4 || assignedLiteralCount < literalCount / 2 || reachedMaxAttempts) 
    {
      pickCounter++; 
      if (pickCounter == 255) 
      {
        for (int i = 0; i < literals.size(); i++) 
        {
          ogLiteralFrequency[i] /= 2;
          if (literalFrequency[i] != -1) 
          {
            literalFrequency[i] /= 2;
          }
        }
        pickCounter = 0; 
      }
      
      int variable = distance(literalFrequency.begin(), max_element(literalFrequency.begin(), literalFrequency.end()));
      if (literalPolarity[variable] >= 0) 
      {
        return variable + 1;
      }
      return -variable - 1;

    } 
    else 
    {
      while (attemptCounter < 10 * literalCount) 
      {
        int variable = choose_literal(generator); 
        if (literalFrequency[variable] != -1)   
        {
          if (literalPolarity[variable] >= 0) 
          {
            return variable + 1;
          }
          return -variable - 1;
        }
        attemptCounter++; 
      }
      reachedMaxAttempts = true; 
    }
  } while (reachedMaxAttempts); 
} 

    

bool SATSolverCDCL::allAssigned() 
{
  return literalCount == assignedLiteralCount;
}


void SATSolverCDCL::outputAnswer(int resultStatus) 
{
  if (resultStatus == RetVal::satisfied) 
  {
    cout << "Satisfiable" << endl;
    for (int i = 0; i < literals.size(); i++) 
    {
      if (i != 0) {
        cout << " ";
      }
      if (literals[i] != -1) {
        cout << pow(-1, (literals[i] + 1)) * (i + 1);
      } 
      else 
      {
        cout << (i + 1);
      }
    }
    cout << " 0";
    cout << " " << endl;
  } 
  else 
  {
    cout << "Unsatisfiable" << endl;;
  }
  cout << "Branch count:  " << branchCount << endl;
  cout << "Learnt clauses: " << clauseCount - ogClauseCount << endl;
}


void SATSolverCDCL::solve() 
{
  int resultStatus = CDCL();
  outputAnswer(resultStatus);
}

int main() 
{
  std::clock_t start;
  double duration;
  start = std::clock();

  SATSolverCDCL solver;
  solver.initialize();
  solver.solve();
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  std::cout<<"Time taken: "<< duration <<'\n';
  return 0;
}









