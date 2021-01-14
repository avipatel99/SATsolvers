//g++ --std=c++11 DLIS.cpp -o DLIS
//./DLIS < sat1.txt

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>


using namespace std;

enum State 
{
  satisfied,  
  unsatisfied, 
  normal,             
  completed 
};

class Formula 
{
  public:
    vector<int> literals;
    vector<int> literalFrequency; 
    vector<int> literalPolarity;
    vector<vector<int>> clauses;
    Formula() {}

    Formula(const Formula &formula) 
    {
      literals = formula.literals;
      clauses = formula.clauses;
      literalFrequency = formula.literalFrequency;
      literalPolarity = formula.literalPolarity;
    }
};


class solver 
{
  private:
    Formula formula;      
    int numOfLiterals;      
    int numOfClauses;              
    int unitPropagation(Formula &); 
    int DPLL(Formula);            
    int updateLiteral(Formula &, int); 
    void outputAnswer(Formula &, int); 
    int branchCount = 0;
  public:
    solver() {}
    void parse(); 
    void callSolver();      
};


void solver::parse() 
{
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
  cin >> numOfLiterals;
  cin >> numOfClauses;

  formula.literals.clear();
  formula.literals.resize(numOfLiterals, -1);
  formula.clauses.clear();
  formula.clauses.resize(numOfClauses);
  formula.literalFrequency.clear();
  formula.literalFrequency.resize(numOfLiterals, 0);
  formula.literalPolarity.clear();
  formula.literalPolarity.resize(numOfLiterals, 0);

  int literal; 

  for (int i = 0; i < numOfClauses; i++) {
    while (true) 
    {
      cin >> literal;
      if (literal > 0) 
      {
        formula.clauses[i].push_back(2 * (literal - 1)); 

        formula.literalFrequency[literal - 1]++;
        formula.literalPolarity[literal - 1]++;
      } else if (literal < 0) 
      {
        formula.clauses[i].push_back(2 * ((-1) * literal - 1) + 1); 
        formula.literalFrequency[-1 - literal]++;
        formula.literalPolarity[-1 - literal]--;
      } else {
        break; 
      }
    }
  }
}

int solver::unitPropagation(Formula &formula) 
{
  bool isUnitClause = false; 
  if (formula.clauses.size() == 0) 
  {
    return State::satisfied; 
  }
  do 
  {
    isUnitClause = false;

    for (int i = 0; i < formula.clauses.size(); i++) 
    {
      if (formula.clauses[i].size() == 1) 
      {
        isUnitClause = true;
        formula.literals[formula.clauses[i][0] / 2] = formula.clauses[i][0] % 2; 
        formula.literalFrequency[formula.clauses[i][0] / 2] = -1; 
        int result = updateLiteral(formula, formula.clauses[i][0] / 2); 
        if (result == State::satisfied || result == State::unsatisfied) 
        {
          return result;
        }
        break; 
      } 
      else if (formula.clauses[i].size() == 0) 
      {
        return State::unsatisfied; 
      }
    }
  } while (isUnitClause);

  return State::normal;
}


int solver::updateLiteral(Formula &formula, int updatedLiteral) 
{
  int newValue = formula.literals[updatedLiteral]; 

  for (int i = 0; i < formula.clauses.size(); i++) 
  {
    for (int j = 0; j < formula.clauses[i].size(); j++) 
    {
      if ((2 * updatedLiteral + newValue) == formula.clauses[i][j]) {
        formula.clauses.erase(formula.clauses.begin() + i); 
        i--;            
        if (formula.clauses.size() == 0) 
        {
          return State::satisfied;
        }
        break; 
      } else if (formula.clauses[i][j] / 2 == updatedLiteral) 
      {
        formula.clauses[i].erase(
            formula.clauses[i].begin() + j);
        j--;    
        if (formula.clauses[i].size() == 0) 
        {
          return State::unsatisfied;
        }
        break; 
      }
    }
  }

  return State::normal;
}

int solver::DPLL(Formula formula) 
{
  int result = unitPropagation(formula); 
  if (result == State::satisfied) 
  {
    outputAnswer(formula, result);
    return State::completed;
  } 
  else if (result == State::unsatisfied)                                     
  {
    return State::normal;
  }

  int i = distance(formula.literalFrequency.begin(), max_element(formula.literalFrequency.begin(), formula.literalFrequency.end()));

  for (int j = 0; j < 2; j++) 
  {
    Formula newFormula = formula; 
    if (newFormula.literalPolarity[i] > 0) 
    {
      newFormula.literals[i] = j; 
    }
    else                  
    {
      newFormula.literals[i] = (j + 1) % 2; 
    }
    branchCount = branchCount + 1;
    newFormula.literalFrequency[i] = -1; 
    int transform_result = updateLiteral(newFormula, i); 
    if (transform_result == State::satisfied) 
    {
      outputAnswer(newFormula, transform_result);
      return State::completed;
    } 
    else if (transform_result == State::unsatisfied)
    {
      continue;
    }
    int dpll_result = DPLL(newFormula); 
    if (dpll_result == State::completed) 
    {
      return dpll_result;
    }
  }
  return State::normal;
}

void solver::outputAnswer(Formula &formula, int result) 
{
  cout << "Branch count:  " << branchCount << endl;
  if (result == State::satisfied)
  {
    cout << "Satisfiable " << endl;
    for (int i = 0; i < formula.literals.size(); i++) 
    {
      if (i != 0) 
      {
        cout << " ";
      }
      if (formula.literals[i] != -1) 
      {
        cout << pow(-1, formula.literals[i]) * (i + 1);
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
    cout << "Unsatisfiable" << endl;
  }
}


void solver::callSolver() 
{
  int result = DPLL(formula); 
  if (result == State::normal) 
  {
    outputAnswer(formula, State::unsatisfied); 
  }
}

int main() 
{
  std::clock_t start;
  double duration;
  start = std::clock();

  solver solver; 
  solver.parse();  
  solver.callSolver();      
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  std::cout<<"Time taken: "<< duration <<'\n';
  return 0;

}


