#python SMT.py sl1.txt

import sys, random, time
import ast
mostRecentConflict = [];

trueConstraints = []

def parse(file):
    global numVars
    clauses = []
    for line in open(file):
        if (line.startswith('c')):
            continue
        if (line.startswith('p')):
            problemLine = line.split()
            numVars = int(problemLine[1])
            numClauses = [3]
            continue
        clause = []
        negative = False
        for x in line.split():
            clause.append(ast.literal_eval(x))
        clauses.append(clause)
    for y in clauses:
        y.pop(-1)
    return clauses, int(numVars)

def bcp2(formula, unit):
    updated = []
    for clause in formula:
        if unit in clause:
            continue
        if -unit in clause:
            c = [x for x in clause if x != -unit]
            if len(c) == 0:
                return -1
            updated.append(c)
        else:
            updated.append(clause)
    return updated

def bcp(formula, unit):
    global trueConstraints
    if(checkConstraints(trueConstraints + [unit]) == False):
        trueConstraints.append(unit)
        return formula
    updated = []
    for clause in formula:
        conflicts = []
        containsUnit = False
        containsConflict = False
        for constraint in clause:
            if(constraint == unit):
                containsUnit = True
                continue
            if(checkConflict(unit, constraint)):
                conflicts.append(constraint)
                containsConflict = True
        if(containsUnit):
            continue
        if(containsConflict):
            newClause = [x for x in clause if x not in conflicts]
            if(len(newClause) == 0):
                return -1
            updated.append(newClause)
        else:
            updated.append(clause)
    trueConstraints.append(unit)
    return updated


def checkConflict(constraint1, constraint2):
    global mostRecentConflict;
    if(constraint1[0] == constraint2[0] and constraint1[1] == constraint2[1] and constraint1[2] != constraint2[2]):
        return True
    else:
        mostRecentConflict = [constraint1, constraint2]
        return False

def get_counter(formula):
    counter = []
    for clause in formula:
        for constraint in clause:
            if (constraint not in counter):
                counter.append(constraint)
    return counter

def nextVariable(formula):
##    counter = get_counter(formula)
##    return random.choice(counter)
    return formula[0][0]



def unit_propagation(formula):
    assignment = []
    unit_clauses = []
    for clause in formula:
        if len(clause) == 1:
            unit_clauses.append(clause)
    while len(unit_clauses) > 0:
        unit = unit_clauses[0]
        formula = bcp(formula, unit[0])
        assignment += [unit[0]]
        if formula == -1:
            return -1, []
        if (checkConstraints(trueConstraints) == False):
            trueConstraints.pop()
            return -1, []
        if not formula:
            return formula, assignment
        unit_clauses = [clause for clause in formula if len(clause) == 1]
    return formula, assignment


def backtracking(formula, assignment):
    global trueConstraints
    formula, unit_assignment = unit_propagation(formula)
    assignment = assignment + unit_assignment
    if (formula == -1):
        return []
    if (checkConstraints(trueConstraints) == False):
        trueConstraints.pop()
        return []
    if not formula:
        return assignment
    variable = nextVariable(formula)
    solution = backtracking(bcp(formula, variable), assignment + [variable])
    if not solution:
        for i in range(1,len(formula[0])):
            variable2 = formula[0][i]
            solution = backtracking(bcp(formula, variable2), assignment + [variable2])
            if (solution):
                break
    return solution

def checkConstraints(constraints):
    global mostRecentConflict;
    newConstraint = []
    for i in constraints:
        for j in constraints:
            if(i!=j):
                if(i[0] == j[0]):
                    if(i[1] == j[1]):
                        if(i[2] != j[2]):
                            mostRecentConflict = [i,j]
                            return False
                        else:
                            continue
                    else:
                        newConstraint = [i[1],j[1],j[2]-i[2]]
                elif(i[0] == j[1]):
                    if(i[1] == j[0]):
                        if(j[2] != -i[2]):
                            mostRecentConflict = [i,j]
                            return False
                        else:
                            continue
                    else:
                        newConstraint = [j[0],i[1],i[2]+j[2]]
                elif(i[1] == j[0]):
                    if(i[0] == j[1]):
                        if(j[2] != -i[2]):
                            mostRecentConflict.append(i)
                            mostRecentConflict.append(j)
                            return False
                        else:
                            continue
                    else:
                        newConstraint = [i[0],j[1],i[2]+j[2]]
                elif(i[1] == j[1]):
                    if(i[0] == j[0]):
                        if(i[2] != j[2]):
                            mostRecentConflict = [i,j]
                            return False
                        else:
                            continue
                    else:
                        newConstraint = [j[0],i[0],j[2]-i[2]]
                        
                if(newConstraint != []):
                    for k in constraints:
                        if(newConstraint[0] == k[0] and newConstraint[1] == k[1] and newConstraint[2] != k[2]):
                            mostRecentConflict = [newConstraint[0], newConstraint[1], k]
                            return False
                    if (newConstraint not in constraints):
                        constraints.append(newConstraint)
                    newConstraint = []
    return True

def main():
    global mostRecentConflict;
    start = time.time()
    clauses, numVars = parse(sys.argv[1])
    solution = backtracking(clauses, [])
    if solution:
##        solution += [x for x in clauses if x not in solution and -x not in solution] #adds numbers that aren't in the solution (set positively)
#         solution.sort(key=lambda x: abs(x)) #ensures numbers are in order     
        print ('SATISFIABLE')
        print ('' + ' '.join([str(x) for x in solution]) + ' 0') #just formatting
    else:
        print ('UNSATISFIABLE')
        print("Cause of conflict: ")
        print(mostRecentConflict)
    end = time.time()
    totalTime = end-start
    print("Time Taken: ", totalTime)


if __name__ == '__main__':
    main()
